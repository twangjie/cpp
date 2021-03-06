/*
 * This file is part of QuickCPP.
 * (c) Copyright 2011 Jie Wang(twj31470952@gmail.com)
 *
 * QuickCPP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QuickCPP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with QuickCPP.  If not, see <http://www.gnu.org/licenses/>.
 */
 
//==============================================================================
//
// $Revision$
// $Date$
//
//==============================================================================

#include "SocketOutputStream.h"
#include "NetUtils.h"
#include "Socket.h"
#include "SocketDescriptor.h"
#include "SocketException.h"

#include "QcCore/base/debug.h"
#include "QcCore/base/NullPointerException.h"
#include "QcCore/base/Tracer.h"
#include "QcCore/io/IOException.h"

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#endif //WIN32

QC_NET_NAMESPACE_BEGIN

using io::IOException;

//==============================================================================
// SocketOutputStream::SocketOutputStream
//
/**
   Creates a SocketOutputStream for a given SocketDescriptor.

   @throws NullPointerException if @c pDescriptor is null.
*/
//==============================================================================
SocketOutputStream::SocketOutputStream(SocketDescriptor* pDescriptor) :
	m_rpSocketDescriptor(pDescriptor)
{
	if(!pDescriptor) throw NullPointerException();
	pDescriptor->modifySocketFlags(SocketDescriptor::HasOutputStream, 0);
}

//==============================================================================
// SocketOutputStream::~SocketOutputStream
//
/**
   Marks the output stream as closed and releases system resources associated
   with it.

   If the socket's input stream has been shutdown (or end of file reached), then
   the socket's output channel will be closed with a shutdown operation.  However,
   if the input channel is still open, to avoid peer applications receiving
   a close signal, the output channel will remain open but be marked as 
   "shutdown-pending", to be shutdown automatically when the input channel 
   becomes closed.
   
   @sa close()
*/
//==============================================================================
SocketOutputStream::~SocketOutputStream()
{
	if(m_rpSocketDescriptor)
	{
		m_rpSocketDescriptor->modifySocketFlags(0, SocketDescriptor::HasOutputStream);
	}
}

//=============================================================================
// SocketOutputStream::write
// 
// Part of the OutputStream interface.  For a socket this involves calling the
// send() function.
//
// Note: For non-blocking sockets, the send() function may send less bytes
//       than have been requested.  For this reason we always loop
//       round until we have sent the requested number of bytes.  Note that
//       even non-blocking sockets will block until at least one byte has been
//       sent.
//=============================================================================
void SocketOutputStream::write(const Byte* pBuffer, size_t bufLen)
{
	if(!pBuffer) throw NullPointerException();
	if(!m_rpSocketDescriptor) throw IOException(QC_T("stream is closed"));

	//
	// Under Linux (and others?), reading from a broken socket can generate
	// a SIGPIPE signal.  Generally we would prefer to translate these
	// into exceptions, so we make use of the MSG_NOSIGNAL flag
	//
#if defined(MSG_NOSIGNAL)
	int iFlags = MSG_NOSIGNAL;
#else
	int iFlags = 0;
#endif

	if(Tracer::IsEnabled())
	{
		Tracer::TraceBytes(Tracer::Net, Tracer::Low, QC_T("Data send:"), pBuffer, bufLen);
	}

	size_t byteCount = 0;
	while(byteCount < bufLen)
	{
		int bytesSent = ::send(m_rpSocketDescriptor->getFD(), (char*)pBuffer+byteCount, bufLen-byteCount, iFlags);
		if(bytesSent < 1)
		{
			const int errNum = NetUtils::GetLastSocketError();

			// An error generated by a shutdown socket should be reported as such
			if(m_rpSocketDescriptor->getSocketFlags() & SocketDescriptor::ShutdownOutput)
				throw SocketException(QC_T("socket shutdown for output"));

			static const String err = QC_T("error writing to socket");
			String errMsg = err + NetUtils::GetSocketErrorString(errNum);
			throw SocketException(errMsg);
		}
		else
		{
			byteCount += bytesSent;
		}
	}
}

//==============================================================================
// SocketOutputStream::close
//
// Close the OutputStream.
//
// For a socket, closing the output stream is the equivalent of shutting
// down the outout stream - it does not imply that the socket itself should
// be closed.
//
//==============================================================================
void SocketOutputStream::close()
{
	if(m_rpSocketDescriptor)
	{
		NetUtils::ShutdownSocket(m_rpSocketDescriptor.get(),
		                         NetUtils::Write);
//TODO check compatability with JDK
		m_rpSocketDescriptor.release();
	}
}

QC_NET_NAMESPACE_END

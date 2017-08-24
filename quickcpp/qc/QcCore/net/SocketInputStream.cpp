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

#include "SocketInputStream.h"
#include "NetUtils.h"
#include "Socket.h"
#include "SocketDescriptor.h"
#include "SocketTimeoutException.h"

#include "QcCore/io/IOException.h"
#include "QcCore/base/SystemUtils.h"
#include "QcCore/base/Tracer.h"
#include "QcCore/base/NullPointerException.h"

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#endif //WIN32

QC_NET_NAMESPACE_BEGIN

using io::IOException;

//==============================================================================
// SocketInputStream::SocketInputStream
//
// Construct a SocketInputStream with the passed socket file descriptor.
// The sockfd should reference an open network socket.
//==============================================================================
SocketInputStream::SocketInputStream(SocketDescriptor* pDescriptor) :
	m_rpSocketDescriptor(pDescriptor),
	m_timeoutMS(0)
{
	if(!pDescriptor) throw NullPointerException();
}

//==============================================================================
// SocketInputStream::~SocketInputStream
//
//==============================================================================
SocketInputStream::~SocketInputStream()
{
}

//=============================================================================
// SocketInputStream::read
// 
// Part of the InputStream inteface.  For a socket this involves calling the
// recv() function.
//
// Returns:
//   the number of Bytes read
//   EndOfFile (-1) if at end-of-file
//
// Throws:
//   IOException
//   InterruptedIOException
//   NullPointerException
//=============================================================================
long SocketInputStream::read(Byte* pBuffer, size_t bufLen)
{
	SystemUtils::TestBufferIsValid(pBuffer, bufLen);

	if(!m_rpSocketDescriptor) throw IOException(QC_T("stream is closed"));

	// simulate SO_TIMEOUT
	if(m_timeoutMS && !NetUtils::SelectSocket(m_rpSocketDescriptor.get(), m_timeoutMS, true, false))
	{
		static const String err(QC_T("Recv timed out"));
		throw SocketTimeoutException(err);
	}

	//
	// Under Linux (and others?), reading from a broken socket can generate
	// a SIGPIPE signal.  Generally we would prefer to translate these
	// into exceptions, so we make use of the MSG_NOSIGNAL flag if it is available
	//
#if defined(MSG_NOSIGNAL)
	int iFlags = MSG_NOSIGNAL;
#else
	int iFlags = 0;
#endif

	int iBytes = ::recv(m_rpSocketDescriptor->getFD(), (char*)pBuffer, bufLen, iFlags);

	if(iBytes == 0)
	{
		//
		// A read of zero bytes indicates the socket stream has reached End Of File.
		//
		Tracer::Trace(Tracer::Net, Tracer::Low, String(QC_T("EOF rcvd from socket: ") + m_rpSocketDescriptor->toString()));

		//
		// We update the socket descriptor flags to indicate that the read half has been
		// shutdown.  While this is not strictly the case, we do want further read ops
		// to recv EOF rather than an exception and we also want to trigger
		// auto_close if it has been enabled, so this is a convenient way of achieving 
		// these aims.
		//
		m_rpSocketDescriptor->modifySocketFlags(SocketDescriptor::ShutdownInput, 0);

		return EndOfFile;
	}
	else if(iBytes < 0)
	{
		// An error generated from a shutdown socket should be translated to an
		// EndOfFile return.  We don't check this before the recv() call, as it is
		// a synchronized call.
		//
		const int sockFlags = m_rpSocketDescriptor->getSocketFlags();
		if(sockFlags & SocketDescriptor::ShutdownInput)
			return EndOfFile;
		else if(sockFlags & SocketDescriptor::DescriptorClosed)
			throw IOException(QC_T("stream is closed"));

		static const String err = QC_T("error reading from socket: ");
		String errMsg = err + NetUtils::GetSocketErrorString();
		throw IOException(errMsg);
	}

	//
	// Getting this far means that we have received some bytes.
	// If we are using particularly promiscuous logging, this data
	// may need to be logged.
	//
	if(Tracer::IsEnabled())
	{
		Tracer::TraceBytes(Tracer::Net, Tracer::Low, QC_T("Data rcvd:"), pBuffer, iBytes);
	}

	return iBytes;
}

//==============================================================================
// SocketInputStream::available
//
// Return the number of bytes available on the socket.  This is the number
// of bytes that can be read without blocking.
//
// There is no portable way to determine the number of bytes available.  We
// make use of the ioctl verb: FIONREAD which gives us the number we want - but
// it is not available in all systems.
//
// Even when it is correctly implemented, this function is of dubious utility
// because it returns 0 when the input stream is at EOF.  So an application
// cannot loop around indefinitely on a zero value.
//==============================================================================
size_t SocketInputStream::available()
{
	if(!m_rpSocketDescriptor) throw IOException(QC_T("stream is closed"));

	//
	// If the socket has been shutdown for input, return 0
	//
	if(m_rpSocketDescriptor->getSocketFlags() & SocketDescriptor::ShutdownInput)
		return 0;

#if defined(FIONREAD)

	return NetUtils::SocketIOControl(m_rpSocketDescriptor.get(), FIONREAD, 0);

#else // !FIONREAD

	//
	// If the FIONREAD option is not available, then we can check to see if there
	// is any data at all in the read buffer using select() with a timeout of 0
	//
	if(NetUtils::SelectSocket(m_rpSocketDescriptor.get(), 0, true, false))
	{
		return 1;
	}
	else
	{
		return 0;
	}
#endif // FIONREAD
}

//==============================================================================
// SocketInputStream::close
//
// Close the InputStream.
//
// For a socket, closing the input stream is the equivalent of shutting
// down the input stream - it does not imply that the socket itself should
// be closed.
//
// Note that Sockets which have their ouput stream in the "close-pending" state
// will have their output stream shutdown at this point also.
//==============================================================================
void SocketInputStream::close()
{
	if(m_rpSocketDescriptor)
	{
		NetUtils::ShutdownSocket(m_rpSocketDescriptor.get(),
		                         NetUtils::Read);
//TODO: check compatability with JDK
		m_rpSocketDescriptor.release();
	}
}

//==============================================================================
// SocketInputStream::getTimeout
//
// Return the timeout value (in milliseconds) for blocking receive operations
// on the socket underlying this stream.
//==============================================================================
size_t SocketInputStream::getTimeout() const
{
	return m_timeoutMS;
}

//==============================================================================
// SocketInputStream::setTimeout
//
// Set a timeout value (in milliseconds) which is the maximum time
// that a read call will block for.  If the timer expires, the read() will
// fail with an SocketTimeoutException.
//==============================================================================
void SocketInputStream::setTimeout(size_t timeoutMS)
{
	m_timeoutMS = timeoutMS;
}

QC_NET_NAMESPACE_END

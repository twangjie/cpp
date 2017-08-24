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
/**
	@class qc::net::SocketDescriptor
	
	@brief An abstract base class representing socket handles.

	By using reference-counted SocketDescriptors, @QuickCPP is able to share
	socket handles between multiple objects, preventing the socket from being
	closed until all references to the SocketDescriptor have been removed.

	This is a SynchronizedObject to allow QuickCPP to synchronize some socket
	operations so that they may be safely called by multiple threads.
*/
//==============================================================================

#include "SocketDescriptor.h"
#include "NetUtils.h"

#include "QcCore/base/NumUtils.h"
#include "QcCore/base/Tracer.h"
#include "QcCore/io/IOException.h"

QC_NET_NAMESPACE_BEGIN

using io::IOException;

//==============================================================================
// SocketDescriptor::SocketDescriptor
//
/**
   Constructs a SocketDescriptor from an operating system socket handle.
*/
//==============================================================================
SocketDescriptor::SocketDescriptor(OSSocketDescriptorType sockfd) :
	ResourceDescriptor(true /*auto close*/),
	m_fd(sockfd),
	m_flags(0)
{
}

//==============================================================================
// SocketDescriptor::~SocketDescriptor
//
/**
   Destructor.

   Calls getAutoClose() and closes the related resource if it returns @c true.
*/
//==============================================================================
SocketDescriptor::~SocketDescriptor()
{
	if(getAutoClose())
	{
		try
		{
			close();
		}
		catch(Exception& /*e*/)
		{
		}
	}
}

//==============================================================================
// SocketDescriptor::close
//
/**
   Closes the socket.
*/
//==============================================================================
void SocketDescriptor::close()
{
	Tracer::Trace(Tracer::Net, Tracer::Medium, String(QC_T("Closing socket: ")) + toString());

	int rc=
#if defined(WIN32)
	::closesocket(m_fd);
#else
	::close(m_fd);
#endif

	if(rc != 0)
	{
		static const String err(QC_T("error closing socket: "));
		String errMsg = err + NetUtils::GetSocketErrorString();
		throw IOException(errMsg);
	}

	QC_SYNCHRONIZED
	m_flags |= DescriptorClosed;
	setAutoClose(false);
}

//==============================================================================
// SocketDescriptor::getFD
//
/**
	Returns an operating system handle for the socket.
*/
//==============================================================================
SocketDescriptor::OSSocketDescriptorType SocketDescriptor::getFD() const
{
	return m_fd;
}

//==============================================================================
// SocketDescriptor::getSocketFlags
//
/**
   Returns the current socket flags.

   @sa setSocketFlags
   @mtsafe
*/
//==============================================================================
int SocketDescriptor::getSocketFlags() const
{
	QC_SYNCHRONIZED

	return m_flags;
}

//==============================================================================
// SocketDescriptor::setSocketFlags
//
/**
   Sets the socket flags.

   Each open socket has a set of flags associated with it for use by internal
   @QuickCPP routines.

   @param flags the new flag value
   
   @mtsafe
*/
//==============================================================================
void SocketDescriptor::setSocketFlags(int flags)
{
	QC_SYNCHRONIZED
	m_flags = flags;
}

//==============================================================================
// SocketDescriptor::toString
//
//==============================================================================
String SocketDescriptor::toString() const
{
	return NumUtils::ToString(m_fd);
}

//==============================================================================
// SocketDescriptor::setSocketFlags
//
/**
   Sets the socket flags.

   Each open socket has a set of flags associated with it for use by internal
   @QuickCPP routines.

   @param flagsToSet a value whose bits are ORed with the existing flag bits to
          arrive at a new value.
   @param flagsToUnset a value whose bits are NANDedd with the existing flag bits to
          arrive at a new value.
   @returns the new flag value
   @mtsafe
*/
//==============================================================================
int SocketDescriptor::modifySocketFlags(int flagsToSet, int flagsToUnset)
{
	QC_SYNCHRONIZED
	m_flags = ((m_flags | flagsToSet) & ~flagsToUnset);

	//
	// If the AutoCloseEnabled flag is set and the socket has not yet been closed
	// and input is at eof (or shutdown which is the same thing)
	// then the socket is automatically closed.
	//
	if((m_flags & (ShutdownInput | AutoCloseEnabled)) == (ShutdownInput | AutoCloseEnabled) &&
	   (m_flags & (HasOutputStream|DescriptorClosed)) == 0)
	{
		try
		{
			close();
		}
		catch(Exception& /*e*/)
		{
		}
	}

	return m_flags;
}

QC_NET_NAMESPACE_END

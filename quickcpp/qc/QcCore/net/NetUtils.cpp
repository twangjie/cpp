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

#include "NetUtils.h"
#include "Socket.h"
#include "SocketDescriptor.h"
#include "SocketException.h"

#include "QcCore/base/NumUtils.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/base/SystemUtils.h"
#include "QcCore/base/Tracer.h"
#include "QcCore/io/IOException.h"

#include <algorithm>

#include <fcntl.h>
#include <errno.h>

#ifndef WIN32
#include <sys/ioctl.h>
#include <sys/time.h>
#endif //WIN32

QC_NET_NAMESPACE_BEGIN

using io::IOException;

bool NetUtils::s_bInitialized = false;

// List of Winsock error constants mapped to an interpretation string.
// Note that this list must remain sorted by the error constants'
// values, because we do a binary search on the list when looking up
// items.
//
// Ported from public domain original code at
// http://www.cyberport.com/~tangent/programming/winsock/examples/basics/ws-util.cpp
//
#ifdef WIN32
struct ErrorEntry {
	int nID;
	const char* szMessage;

	ErrorEntry(int id, const char* pszMessage=0) :
		nID(id),
		szMessage(pszMessage)
	{}

	bool operator<(const ErrorEntry& rhs) const
	{
		return nID < rhs.nID;
	}
};

static ErrorEntry ErrorList[] = {
    ErrorEntry(0,                  "No error"),
    ErrorEntry(WSAEINTR,           "Interrupted system call"),
    ErrorEntry(WSAEBADF,           "Bad file number"),
    ErrorEntry(WSAEACCES,          "Permission denied"),
    ErrorEntry(WSAEFAULT,          "Bad address"),
    ErrorEntry(WSAEINVAL,          "Invalid argument"),
    ErrorEntry(WSAEMFILE,          "Too many open sockets"),
    ErrorEntry(WSAEWOULDBLOCK,     "Operation would block"),
    ErrorEntry(WSAEINPROGRESS,     "Operation now in progress"),
    ErrorEntry(WSAEALREADY,        "Operation already in progress"),
    ErrorEntry(WSAENOTSOCK,        "Socket operation on non-socket"),
    ErrorEntry(WSAEDESTADDRREQ,    "Destination address required"),
    ErrorEntry(WSAEMSGSIZE,        "Message too long"),
    ErrorEntry(WSAEPROTOTYPE,      "Protocol wrong type for socket"),
    ErrorEntry(WSAENOPROTOOPT,     "Bad protocol option"),
    ErrorEntry(WSAEPROTONOSUPPORT, "Protocol not supported"),
    ErrorEntry(WSAESOCKTNOSUPPORT, "Socket type not supported"),
    ErrorEntry(WSAEOPNOTSUPP,      "Operation not supported on socket"),
    ErrorEntry(WSAEPFNOSUPPORT,    "Protocol family not supported"),
    ErrorEntry(WSAEAFNOSUPPORT,    "Address family not supported"),
    ErrorEntry(WSAEADDRINUSE,      "Address in use"),
    ErrorEntry(WSAEADDRNOTAVAIL,   "Can't assign requested address"),
    ErrorEntry(WSAENETDOWN,        "Network is down"),
    ErrorEntry(WSAENETUNREACH,     "Network is unreachable"),
    ErrorEntry(WSAENETRESET,       "Net connection reset"),
    ErrorEntry(WSAECONNABORTED,    "Software caused connection abort"),
    ErrorEntry(WSAECONNRESET,      "Connection reset by peer"),
    ErrorEntry(WSAENOBUFS,         "No buffer space available"),
    ErrorEntry(WSAEISCONN,         "Socket is already connected"),
    ErrorEntry(WSAENOTCONN,        "Socket is not connected"),
    ErrorEntry(WSAESHUTDOWN,       "Can't send after socket shutdown"),
    ErrorEntry(WSAETOOMANYREFS,    "Too many references, can't splice"),
    ErrorEntry(WSAETIMEDOUT,       "Operation timed out"),
    ErrorEntry(WSAECONNREFUSED,    "Connection refused"),
    ErrorEntry(WSAELOOP,           "Too many levels of symbolic links"),
    ErrorEntry(WSAENAMETOOLONG,    "File name too long"),
    ErrorEntry(WSAEHOSTDOWN,       "Host is down"),
    ErrorEntry(WSAEHOSTUNREACH,    "Host unreachable"),
    ErrorEntry(WSAENOTEMPTY,       "Directory not empty"),
    ErrorEntry(WSAEPROCLIM,        "Too many processes"),
    ErrorEntry(WSAEUSERS,          "Too many users"),
    ErrorEntry(WSAEDQUOT,          "Disc quota exceeded"),
    ErrorEntry(WSAESTALE,          "Stale NFS file handle"),
    ErrorEntry(WSAEREMOTE,         "Too many levels of remote in path"),
    ErrorEntry(WSASYSNOTREADY,     "Network system is unavailable"),
    ErrorEntry(WSAVERNOTSUPPORTED, "Winsock version out of range"),
    ErrorEntry(WSANOTINITIALISED,  "WSAStartup not yet called"),
    ErrorEntry(WSAEDISCON,         "Graceful shutdown in progress"),
    ErrorEntry(WSAHOST_NOT_FOUND,  "Host not found"),
    ErrorEntry(WSANO_DATA,         "No host data of that type was found")
};

const int kNumMessages = sizeof(ErrorList) / sizeof(ErrorEntry);
#endif //WIN32

//==============================================================================
// NetUtils::GetLocalHostName
//
// Return the name of the local host
//==============================================================================
String NetUtils::GetLocalHostName()
{
	InitializeSocketLibrary();

	const int buflen = MAXHOSTNAMELEN + 1;
	char buffer[buflen];
	if(::gethostname(buffer, buflen) < 0)
	{
		static const String err(QC_T("unable to obtain local host name: "));
		String errMsg = err + SystemUtils::GetSystemErrorString();
		throw IOException(errMsg);
	}

	return StringUtils::FromLatin1(buffer);
}

//==============================================================================
// NetUtils::GetLastSocketError
//
// Static function to make socket errors more portable.
//==============================================================================
int NetUtils::GetLastSocketError()
{
#ifdef WIN32
	return ::WSAGetLastError();
#else
	return errno;
#endif
}

//==============================================================================
// NetUtils::Initialize
//
// Static function to Intiailise the (operating system's) socket library.
// Only WinSock demands this at present.
//==============================================================================
void NetUtils::InitializeSocketLibrary()
{
	if(!s_bInitialized)
	{
#ifdef WIN32
		WORD wVersionRequested;
		WSADATA wsaData;

		wVersionRequested = MAKEWORD(2, 2);
		int rc = ::WSAStartup(wVersionRequested, &wsaData);
		if(rc != 0)
		{
			throw IOException(GetSocketErrorString(rc));
		}
#endif
		s_bInitialized = true;
	}
}

//==============================================================================
// NetUtils::SocketIOControl
//
// Portable ioctl method for sockets.
//==============================================================================
long NetUtils::SocketIOControl(SocketDescriptor* pSocketDescriptor, long cmd, long arg)
{

#if defined(WIN32)
	unsigned long ioArg = arg;
	int rc = ::ioctlsocket(pSocketDescriptor->getFD(), cmd, &ioArg);
#else // !WIN32
	int ioArg = arg;
	int rc = ::ioctl(pSocketDescriptor->getFD(), cmd, &ioArg);
#endif // WIN32

	if(rc != 0)
	{
		static const String err(QC_T("error calling ioctl: "));
		String errMsg = err + NetUtils::GetSocketErrorString();
		throw SocketException(errMsg);
	}
	
	return ioArg;
}

//==============================================================================
// NetUtils::GetSocketErrorString
//
// Portable way to retrieve a message string associated with a socket error.
//==============================================================================
String NetUtils::GetSocketErrorString(int errorNum)
{

#if !defined(WIN32)

	return SystemUtils::GetSystemErrorString(errorNum);

#else

	// Locate appropriate message.
	//
	// Note: we use std::lower_bound to do a binary search of the list
	// of error messages - but this demands the sequence is sorted.
	//
	ErrorEntry* pEnd = ErrorList + kNumMessages;
	ErrorEntry target(errorNum ? errorNum : GetLastSocketError());
	ErrorEntry* iter = std::lower_bound(ErrorList, pEnd, target);

	if ((iter != pEnd) && (iter->nID == target.nID))
	{
		return StringUtils::FromLatin1(iter->szMessage);
	}
	else
	{
		// Didn't find error in list, so make up a generic one
		return QC_T("unknown error");
	}
#endif //WIN32
}


//==============================================================================
// NetUtils::SetBlockingSocket
//
// Helper function to set the blocking mode of a socket.
//==============================================================================
void NetUtils::SetBlockingSocket(SocketDescriptor* pSocketDescriptor, bool bBlocking)
{

#if defined(FIONBIO)

		// The FIONBIO flags sets a socket non-blocking if the parameter
		// is non-zero
		SocketIOControl(pSocketDescriptor, FIONBIO, !bBlocking);

#else

		int flags = ::fcntl(pSocketDescriptor->getFD(), F_GETFL, 0);
		if(flags == -1)
		{
			static const String err(QC_T("error calling fcntl: "));
			String errMsg = err + NetUtils::GetSocketErrorString();
			throw SocketException(errMsg);
		}

		if(bBlocking)
		{
			flags &= ~O_NONBLOCK;
		}
		else
		{
			flags |= O_NONBLOCK;
		}
		
		int rc = ::fcntl(pSocketDescriptor->getFD(), F_SETFL, flags);

		if(rc < 0)
		{
			static const String err(QC_T("error calling fcntl: "));
			String errMsg = err + NetUtils::GetSocketErrorString();
			throw SocketException(errMsg);
		}

#endif // FIONBIO

}

//==============================================================================
// NetUtils::ShutdownSocket
//
/**
   Shuts down a full duplex socket in a given direction.

   @param pSocketDescriptor a pointer to the SocketDescriptor for the socket
          to be shut down
   @param dir the direction in which the socket should be shut down
   @throws NullPointerException if @c pSocketDescriptor is null.
   @throws SocketException if an error occurs performing the shutdown request.
*/
//==============================================================================
void NetUtils::ShutdownSocket(SocketDescriptor* pSocketDescriptor, Direction dir)
{
	if(!pSocketDescriptor) throw NullPointerException();

	int how, flags;

	switch(dir)
	{
	case Read:  
		how = SHUT_RD; 
		flags = SocketDescriptor::ShutdownInput; 
		break;
	case Write:
		how = SHUT_WR; 
		flags = SocketDescriptor::ShutdownOutput;
		break;
	case Both:
	default:
		how = SHUT_RDWR;
		flags = SocketDescriptor::ShutdownInput|SocketDescriptor::ShutdownOutput;
		break;
	}

	if(Tracer::IsEnabled())
	{
		String traceMsg = QC_T("Shutting down socket: ");
		traceMsg += pSocketDescriptor->toString();
		traceMsg += QC_T(", how=");
		traceMsg += NumUtils::ToString(how);
		Tracer::Trace(Tracer::Net, Tracer::Medium, traceMsg);
	}

	int result = ::shutdown(pSocketDescriptor->getFD(), how);

	if(result != 0)
	{
		static const String err(QC_T("error shutting down socket: "));
		String errMsg = err + GetSocketErrorString();
		throw IOException(errMsg);
	}

	//
	// Set the socket descriptor's flags to reflect the shutdown operation.
	//
	pSocketDescriptor->modifySocketFlags(flags, 0);
}

//==============================================================================
// NetUtils::SelectSocket
//
/**
   Performs a timed select operation on a socket.

   @param pSocketDescriptor a pointer to the SocketDescriptor for the socket
          to be selected
   @param timeoutMS the timeout value (in milliseconds)
   @param bRead test the socket for readability
   @param bWrite test the socket for writability
   @throws SocketException if an error occurs
   @returns @c true if the socket is readabile or writeable (depending on
            which option has been specified); @c false if the timeout duration
            has expired.
*/
//==============================================================================
bool NetUtils::SelectSocket(SocketDescriptor* pSocketDescriptor, size_t timeoutMS,
                            bool bRead, bool bWrite)
{
	// Convert the timeout value (in milliseconds) into a timeval struct
	struct timeval timer;
	timer.tv_sec = timeoutMS / 1000;
	timer.tv_usec = (timeoutMS % 1000) * 1000;

	fd_set rs, ws;
	FD_ZERO(&rs);
	FD_ZERO(&ws);
	if(bRead)
		FD_SET(pSocketDescriptor->getFD(), &rs);
	if(bWrite)
		FD_SET(pSocketDescriptor->getFD(), &ws);
	int maxFD = pSocketDescriptor->getFD() + 1;
	int rc = ::select(maxFD, &rs, &ws, 0, &timer);

	//
	// On return from select, a -ive rc indicates an error,
	// 0 indicates timeout
	// a +ive number indicates the number of FDs selected
	//
	if(rc < 0)
	{
		static const String err(QC_T("select() failed: "));
		String errMsg = err + NetUtils::GetSocketErrorString();
		throw SocketException(errMsg);
	}

	return (rc > 0);
}

QC_NET_NAMESPACE_END

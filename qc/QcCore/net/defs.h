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
	@namespace qc::net

	Extends the I/O capabilities of @QuickCPP to include network access.  Includes
	classes for accessing World Wide Web resources via URLs as well as
	stream-oriented (TCP) and connectionless (UDP) sockets.
*/
//==============================================================================

#ifndef QC_NET_DEFS_h
#define QC_NET_DEFS_h

#include "QcCore/base/gendefs.h"
#include "QcCore/base/QCObject.h"
#include "QcCore/base/AutoPtr.h"
#include "QcCore/base/String.h"
#include "QcCore/io/InputStream.h"
#include "QcCore/io/OutputStream.h"

#ifdef WIN32
	#define QC_SOCKET_FD            unsigned int
	#define QC_INVALID_SOCKET       INVALID_SOCKET
#else
	#define QC_SOCKET_FD            int
	#define QC_INVALID_SOCKET       -1
#endif

#if defined (QC_IOS_EXPORT)
	//
	// Windows sockets uses different symbols for error numbers and
	// other flags.  To avoid conditional compilation in the code,
	// we define them here to be the same as the Unix codes
	//
	#ifdef WIN32

		#include <winsock2.h>
		#define SHUT_RD                 SD_RECEIVE
		#define SHUT_WR                 SD_SEND
		#define SHUT_RDWR               SD_BOTH
		#define HOST_NOT_FOUND          WSAHOST_NOT_FOUND
		#define QC_EWOULDBLOCK          WSAEWOULDBLOCK
		#define QC_EINPROGRESS          WSAEWOULDBLOCK
		#define QC_EISCONN              WSAEISCONN
		#define QC_ECONNABORTED         WSAECONNABORTED
		#define QC_ECONNRESET           WSAECONNRESET
		#define QC_ETIMEDOUT            WSAETIMEDOUT
		#define QC_ENETUNREACH          WSAENETUNREACH
		#define QC_EHOSTUNREACH         WSAEHOSTUNREACH
		#define QC_EMSGSIZE             WSAEMSGSIZE
		#define QC_EPROTO               0 // not present on WIN32

	#else // !WIN32

		#include <errno.h>              // reqd for testing presence of EXXXX defines
		#include <netinet/in.h>         // reqd for FreeBSD
		#include <arpa/inet.h>
		extern "C" {                    // reqd for HP-UX
			#include <netdb.h>
		}
		#include <sys/socket.h>         // reqd for SHUT_RD etc

		#define QC_EWOULDBLOCK			EWOULDBLOCK             
		#define QC_EINPROGRESS			EINPROGRESS             
		#define QC_EISCONN				EISCONN            
		#define QC_ECONNABORTED			ECONNABORTED      
		#define QC_ECONNRESET           ECONNRESET   
		#define QC_ETIMEDOUT            ETIMEDOUT   
		#define QC_ENETUNREACH          ENETUNREACH  
		#define QC_EHOSTUNREACH         EHOSTUNREACH   
		#define QC_EMSGSIZE             EMSGSIZE   
		#define QC_EPROTO               EPROTO

	#endif //WIN32

	//
	// Not all platforms define the following symbolic constants
	//
	#ifndef SHUT_RD
		#define SHUT_RD 0
	#endif //SHUT_RD

	#ifndef SHUT_WR
		#define SHUT_WR 1
	#endif //SHUT_WR

	#ifndef SHUT_RDWR
		#define SHUT_RDWR 2
	#endif //SHUT_WR

	// Some systems don't have INADDR_NONE defined...
	#ifndef INADDR_NONE
		#define INADDR_NONE 0xffffffff
	#endif

	// Some systems (win32) don't have MAXHOSTNAMELEN defined...
	#ifndef MAXHOSTNAMELEN
		#define MAXHOSTNAMELEN 255
	#endif

	// FreeBSD does not define EPROTO
	#ifndef EPROTO
		#define EPROTO EPROTONOSUPPORT // BSD has no eproto
	#endif

	#ifdef HAVE_WORKING_SOCKLEN_T
		typedef socklen_t cel_socklen_t;
	#else
		typedef int cel_socklen_t;
	#endif

#endif //QC_IOS_EXPORT


#define QC_NET_PKG QC_QUICKCPP_PKG
#define QC_NET_INCLUDE_INLINES QC_QUICKCPP_INCLUDE_INLINES
#define QC_NET_NAMESPACE_BEGIN QC_SUB_NAMESPACE_BEGIN(net)
#define QC_NET_NAMESPACE_END   QC_SUB_NAMESPACE_END(net)

//
// Under MSVC, automatically link to required library
//
#if defined(_MSC_VER) && (defined(QC_IOS_EXPORT) || !defined(QC_DLL))
	#pragma comment(lib, "ws2_32.lib")
#endif

QC_NET_NAMESPACE_BEGIN

using io::InputStream;
using io::OutputStream;

QC_NET_NAMESPACE_END

#endif //QC_NET_DEFS_h

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
	@class qc::net::InetAddress
	
	@brief An @a abstract class representing an Internet host's IP address.

	<p>This class provides static methods for resolving host names
	into IP addresses.  Presently, only IPv4 addresses are supported,
	but future versions of @QuickCPP will support IPv6 addresses also.</p>

	The following example resolves a hostname into an IP address using the
	GetByName() static member function:-
	
@code
#include "QcCore/net/InetAddress.h"
#include "QcCore/io/Console.h"

using namespace qc;
using namespace qc::io;
using namespace qc::net;

int main(int argc, char* argv[]) {
    try {
        // Get the IP address for "www.google.com"
        AutoPtr<InetAddress> rpAddr = InetAddress::GetByName(QC_T("www.google.com"));

        Console::cout() << QC_T("The IP Address for ") << rpAddr->getHostName() 
                        << QC_T(" is ") << rpAddr->getHostAddress() << endl;
    }
    catch(Exception& e) {
        Console::cerr() << e.toString() << endl;
    }
    return 0;
}
@endcode
*/
//==============================================================================

#include "InetAddress.h"
#include "UnknownHostException.h"
#include "Socket.h"
#include "NetUtils.h"

#include "QcCore/base/ArrayAutoPtr.h"
#include "QcCore/base/IllegalArgumentException.h"
#include "QcCore/base/NullPointerException.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/base/Tracer.h"
#include "QcCore/io/IOException.h"

#include <memory.h>

#ifndef WIN32
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif //WIN32

QC_NET_NAMESPACE_BEGIN

using io::IOException;

//==================================================================
// Multi-threaded resolver support
//
// On a small number of platforms (BSD derivatives such as
// FreeBSD and Mac OS/X) there are no _r versions of the host
// name resolver functions.  In these situations we ensure safety
// by wrapping access to these functions with a global mutex.
//
//==================================================================
#if defined(QC_MT) && !defined(HAVE_THREAD_SAFE_RESOLVER) && defined(MISSING_REENTRANT_RESOLVER)
      FastMutex ResolverMutex;
#endif 

//==============================================================================
// InetAddress::InetAddress
//
/**
   Protected default constructor.  
   Used to prevent the accidental creation of invalid InetAddress instances.
*/
//==============================================================================
InetAddress::InetAddress() :
	m_pAddr(new struct in_addr)
{
	m_pAddr->s_addr = INADDR_NONE;
}

//==============================================================================
// ManagedObject::ManagedObject
//
/**
   Copy constructor.

   A compiler-generated copy constructor would be unsuitable because this
   class contains pointer memebers.

   @param rhs InetAddress being copied.
*/
//==============================================================================
InetAddress::InetAddress(const InetAddress& rhs) :
	ManagedObject(rhs),
	m_pAddr(new struct in_addr)
{
	::memcpy(m_pAddr, rhs.m_pAddr, sizeof(in_addr));
}

//==============================================================================
// InetAddress::operator=
//
/**
   Assignment operator.

   A compiler-generated assignment operator would be unsuitable because this
   class contains pointer memebers.

   @param rhs InetAddress being copied.
*/
//==============================================================================
InetAddress& InetAddress::operator=(const InetAddress& rhs)
{
	if(&rhs != this)
		::memcpy(m_pAddr, rhs.m_pAddr, sizeof(in_addr));
	return *this;
}

//==============================================================================
// ManagedObject::~ManagedObject
//
/**
   Destructor.

   Frees resources held by this object.
*/
//==============================================================================
InetAddress::~InetAddress()
{
	delete m_pAddr;
}

//==============================================================================
// InetAddress::FromNetworkAddress
//
/**
   Creates an InetAddress instance from a @c sockaddr structure.

   @param pAddr pointer to a @c sockaddr structure containing a valid 
          Internet address.
   @param addrlen size of the passed @c sockaddr structure.
   @returns An InetAddress instance initialized with the passed IP address.
   @throws NullPointerException if @c pAddr is null.
   @throws IllegalArgumentException if @c pAddr points to a @c sockaddr
           whose @c sa_family member is not supported.
*/
//==============================================================================
AutoPtr<InetAddress> InetAddress::FromNetworkAddress(const struct sockaddr* pAddr,
                                                    size_t /*addrlen*/)
{
	if(!pAddr) throw NullPointerException();

	if(pAddr->sa_family == AF_INET)
	{
		const struct sockaddr_in* pIP4Addr = 
			reinterpret_cast<const struct sockaddr_in*>(pAddr);

		AutoPtr<InetAddress> rpRet = new InetAddress();
		::memcpy(rpRet->m_pAddr, &pIP4Addr->sin_addr, sizeof(in_addr));
		return rpRet;
	}
	else
	{
		throw IllegalArgumentException(QC_T("invalid network address length"));
	}
}

//==============================================================================
// InetAddress::GetByName
//
/**
   Returns an InetAddress by performing a look-up of a host's name.
   
   The host name can either be a machine name, such as "www.google.com",
   or a string representing its IP address in dotted decimal notation, such as 
   "192.168.10.1".

   @param host the name of the Internet host
   @returns An InetAddress instance for the specified host.
   @throws UnknownHostException if the host name cannot be resolved into
           an IP address.
*/
//==============================================================================
AutoPtr<InetAddress> InetAddress::GetByName(const String& host)
{
	if(Tracer::IsEnabled())
	{
		String traceMsg = QC_T("Resolving host name: ");
		traceMsg += host;
		Tracer::Trace(Tracer::Net, Tracer::Low, traceMsg);
	}

	//
	// To prevent buffer overrun attacks on the BIND library, hostnames
	// that exceed the maximum permitted size are not even queried
	//
	if(host.length() > MAXHOSTNAMELEN)
	{
		throw UnknownHostException(host);
	}

	AutoPtr<InetAddress> rpRet = new InetAddress();

	// Internet host names are currently restricted to a subset of US-ASCII
	// R.Lugt 7/12/2001

	const ByteString& asciiHost = StringUtils::ToAscii(host);

	rpRet->m_pAddr->s_addr = inet_addr(asciiHost.c_str());
	
	if(rpRet->m_pAddr->s_addr == INADDR_NONE)
	{
		NetUtils::InitializeSocketLibrary();

		//
		// Under Windows Sockets gethostbyname() is re-entrant (thread-safe)
		// but this is not the case under Unix/Linux
		//

#if defined(HAVE_THREAD_SAFE_RESOLVER) || !defined(QC_MT) || defined(MISSING_REENTRANT_RESOLVER)

	#if defined(MISSING_REENTRANT_RESOLVER)
		QC_AUTO_LOCK(FastMutex, ResolverMutex);
	#endif

		struct hostent* pHostEnt = ::gethostbyname(asciiHost.c_str());

#else

		//
		// Allocate a reasonable buffer for gethostbyname_r(),
		// and grow it if required.  Thanks to ArrayAutoBuffer, the buffer
		// will automatically be freed at exit from this scope.
		//
		int buffSize = 1024;
		ArrayAutoPtr<char> apBuffer(new char[buffSize]);
		int hostError=0;
		struct hostent* pHostEnt = 0;
		struct hostent myHostEnt;

		while(true)
		{

			//
			// Sadly, Solaris and Linux have incompatible gethostbyname_r
			// functions.
			//
#if defined(HAVE_GETHOSTBYNAME_R_5) // Solaris version
			
			pHostEnt = ::gethostbyname_r(asciiHost.c_str(),
			                             &myHostEnt,
			                             apBuffer.get(),
			                             buffSize,
			                             &hostError);
			const int rc = errno;

#elif defined(HAVE_GETHOSTBYNAME_R_6) // Linux version

			const int rc = ::gethostbyname_r(asciiHost.c_str(),
			                                 &myHostEnt,
			                                 apBuffer.get(),
			                                 buffSize,
			                                 &pHostEnt,
			                                 &hostError);

#else

	#error Unsupported gethostbyname_r

#endif

			if(0 == pHostEnt && rc == ERANGE)
			{
				buffSize *= 2;
				ArrayAutoPtr<char> apNewBuffer(new char[buffSize]);
				apBuffer = apNewBuffer;
			}
			else
			{
				break;
			}
		}
		                                             
#endif // HAVE_FUNC_GETHOSTBYNAME_R_5

		if(pHostEnt)
		{
			::memcpy(rpRet->m_pAddr, pHostEnt->h_addr_list[0], sizeof(struct in_addr));
			rpRet->m_hostName = host;

			if(Tracer::IsEnabled())
			{
				String traceMsg = QC_T("Resolved host name: ");
				traceMsg += rpRet->toString();
				Tracer::Trace(Tracer::Net, Tracer::Low, traceMsg);
			}
		}
		else
		{
			throw UnknownHostException(host);
		}
	}
	
	return rpRet;
}

//==============================================================================
// InetAddress::getAddress
//
/**
   Returns a pointer to a byte array containing the internal IP address.
*/
//==============================================================================
const Byte* InetAddress::getAddress() const
{
	return (const Byte*)m_pAddr;
}

//==============================================================================
// InetAddress::getAddressLength
//
/**
   Returns the length of the byte array returned when calling getAddress().
   
   @sa getAddress()
   @since 1.3
*/
//==============================================================================
size_t InetAddress::getAddressLength() const
{
	return sizeof(struct in_addr);
}

//==============================================================================
// InetAddress::GetLocalHost
//
/**
   Returns an InetAddress instance pertaining to the local host.

   On a multi-homed machine (a machine with multiple IP addresses), just one
   of the available addresses will be selected.
*/
//==============================================================================
AutoPtr<InetAddress> InetAddress::GetLocalHost()
{
	const String& localHost = NetUtils::GetLocalHostName();
	return GetByName(localHost);
}

//==============================================================================
// InetAddress::GetAnyHost
//
/**
   Returns a special InetAddress that does not pertain to any particular Internet 
   host.

   This is often used when binding server or UDP sockets to all local interfaces.
*/
//==============================================================================
AutoPtr<InetAddress> InetAddress::GetAnyHost()
{
	AutoPtr<InetAddress> rpRet = new InetAddress();
	rpRet->m_pAddr->s_addr = INADDR_ANY;
	return rpRet;
}

//==============================================================================
// InetAddress::GetLocalBroadcast
//
/**
   Returns a special InetAddress that can be used to broadcast UDP datagrams
   to all hosts on the local subnet.
*/
//==============================================================================
AutoPtr<InetAddress> InetAddress::GetLocalBroadcast()
{
	AutoPtr<InetAddress> rpRet = new InetAddress();
	rpRet->m_pAddr->s_addr = INADDR_BROADCAST;
	return rpRet;
}

//==============================================================================
// InetAddress::getHostAddress
//
/**
   Returns the IP address in dotted decimal notation.

   For example: "192.168.0.1"
   @sa getHostName()
*/
//==============================================================================
String InetAddress::getHostAddress() const
{
	//
	// inet_ntoa() is not guaranteed to be thread-safe, but it is on some
	// platforms.  Where inet_ntop is available we use that instead.
	//
#if defined(HAVE_INET_NTOP)

	char str[INET_ADDRSTRLEN];
	const char* pBuffer = ::inet_ntop(AF_INET, m_pAddr, str, sizeof(str));

#else

	const char* pBuffer = ::inet_ntoa(*m_pAddr);

#endif

	QC_DBG_ASSERT(pBuffer!=0);
	QC_DBG_ASSERT(strlen(pBuffer)!=0);
	return StringUtils::FromLatin1(pBuffer);
}

//==============================================================================
// InetAddress::toString
//
/**
   Returns a String representation of this InetAddress.
*/
//==============================================================================
String InetAddress::toString() const
{
	static const String sep = QC_T("/");
	return getHostName() + sep + getHostAddress();
}

//==============================================================================
// InetAddress::getHostName
//
/**
   Returns the host name for this InetAddress.

   Unless the host name has already been established (perhaps
   during construction) this routine uses the operating system's resolver 
   library to locate the host name using a reverse DNS look-up.

   No exceptions are thrown.  If the reverse look-up fails, the IP address
   is returned (in dotted decimal notation) instead.

   @sa getHostAddress()
*/
//==============================================================================
String InetAddress::getHostName() const
{
	if(m_hostName.empty())
	{
		if(m_pAddr->s_addr == INADDR_ANY)
		{
			((InetAddress*)this)->m_hostName = getHostAddress();
		}
		else
		{
			NetUtils::InitializeSocketLibrary();

			//
			// Under Windows Sockets gethostbyaddr() is re-entrant (thread-safe)
			// but this is not the case under Unix/Linux
			//

#if defined(HAVE_THREAD_SAFE_RESOLVER) || !defined(QC_MT) || defined(MISSING_REENTRANT_RESOLVER)

	#if defined(MISSING_REENTRANT_RESOLVER)
			QC_AUTO_LOCK(FastMutex, ResolverMutex);
	#endif
			struct hostent* pHostEnt = ::gethostbyaddr((char*)m_pAddr,
			                                           sizeof(struct in_addr),
			                                           AF_INET);

			/* int hostError = h_errno; */

#else
			//
			// Allocate a reasonable buffer for gethostbyaddr_r(),
			// and grow it if required.  Thanks to ArrayAutoBuffer, the buffer
			// will automatically be freed at exit from this scope.
			//
			int buffSize = 1024;
			ArrayAutoPtr<char> apBuffer(new char[buffSize]);
			int hostError=0;
			struct hostent* pHostEnt = 0;
			struct hostent myHostEnt;

			while(true)
			{

				//
				// Sadly, Solaris and Linux have incompatible gethostbyaddr_r
				// functions.
				//

#if defined(HAVE_GETHOSTBYADDR_R_7) // Solaris version
			
				pHostEnt = ::gethostbyaddr_r((char*)m_pAddr,
				                         sizeof(struct in_addr),
				                         AF_INET,
				                         &myHostEnt,
				                         apBuffer.get(),
				                         buffSize,
				                         &hostError);
				const int rc = errno;

#elif defined(HAVE_GETHOSTBYADDR_R_8) // Linux version

				const int rc = ::gethostbyaddr_r((char*)m_pAddr,
				                                 sizeof(struct in_addr),
				                                 AF_INET,
				                                 &myHostEnt,
				                                 apBuffer.get(),
				                                 buffSize,
				                                 &pHostEnt,
				                                 &hostError);

#else

	#error Unsupported gethostbyaddr_r

#endif

				if(0 == pHostEnt && rc == ERANGE)
				{
					buffSize *= 2;
					ArrayAutoPtr<char> apNewBuffer(new char[buffSize]);
					apBuffer = apNewBuffer;
				}
				else
				{
					break;
				}
			}
		                                             
#endif // HAVE_FUNC_GETHOSTBYNAME_R_5

			if(0 == pHostEnt)
			{
				//
				// If the host cannot be found then just use the ip address
				//
				((InetAddress*)this)->m_hostName = getHostAddress();
			}
			else
			{
				//
				// Cache hostname to avoid further look-ups
				//
				((InetAddress*)this)->m_hostName = StringUtils::FromLatin1(pHostEnt->h_name);
			}
		}
	}

	return m_hostName;
}

//==============================================================================
// InetAddress::equals
//
/**
   Tests if two InetAddress instances refer to the same IP address.

   @param rhs an InetAddress to compare against.
   @returns @c true if they are equal; @c false otherwise.
*/
//==============================================================================
bool InetAddress::equals(const InetAddress& rhs) const
{
	return (::memcmp(m_pAddr, rhs.m_pAddr, sizeof(struct in_addr))==0);
}

//==============================================================================
// InetAddress::operator==
//
/**
   Tests if two InetAddress instances refer to the same IP address.

   @param rhs an InetAddress to compare against.
   @returns @c true if they are equal; @c false otherwise.
*/
//==============================================================================
bool InetAddress::operator==(const InetAddress& rhs) const
{
	return equals(rhs);
}

//==============================================================================
// InetAddress::operator!=
//
/**
   Tests if two InetAddress instances refer to different IP addresses.

   @param rhs an InetAddress to compare against.
   @returns @c true if they are not equal; @c false otherwise.
*/
//==============================================================================
bool InetAddress::operator!=(const InetAddress& rhs) const
{
	return !equals(rhs);
}

QC_NET_NAMESPACE_END

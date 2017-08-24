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

#include "PlainDatagramSocketImpl.h"
#include "BindException.h"
#include "InetAddress.h"
#include "NetUtils.h"
#include "DatagramPacket.h"
#include "Socket.h"
#include "SocketDescriptor.h"
#include "SocketException.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "SocketTimeoutException.h"

#include "QcCore/base/IllegalStateException.h"
#include "QcCore/base/NullPointerException.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/base/SystemUtils.h"
#include "QcCore/base/Tracer.h"

#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#endif //WIN32

QC_NET_NAMESPACE_BEGIN

//==============================================================================
// PlainDatagramSocketImpl::PlainDatagramSocketImpl
//
// Default (and only) constructor.
//
// Instances of PlainDatagramSocketImpl are created exclusively by a 
// DatagramSocketImplFactory.
//==============================================================================
PlainDatagramSocketImpl::PlainDatagramSocketImpl() : 
	m_localPort(-1),
	m_remotePort(-1),
	m_nTimeoutMS(0)
{
}

//==============================================================================
// PlainDatagramSocketImpl::~PlainDatagramSocketImpl
//
//==============================================================================
PlainDatagramSocketImpl::~PlainDatagramSocketImpl()
{
}

//==============================================================================
// PlainDatagramSocketImpl::create
//
//==============================================================================
void PlainDatagramSocketImpl::create()
{
	if(m_rpSocketDescriptor)
		throw SocketException(QC_T("socket already created"));

	SocketDescriptor::OSSocketDescriptorType socketFD;

	// Reset member control flags just in case this DatagramSocketImpl has been 
	// used and closed before.  This probably isn't possible, but
	// it is sensible to guard against it anyway.
	m_localPort = -1;
	m_remotePort = -1;

	//
	// Attempt to create a stream socket.
	//
	if( (socketFD = socket(AF_INET, SOCK_DGRAM, 0)) == QC_INVALID_SOCKET)
	{
		static const String err(QC_T("unable to create datagram socket: "));
		String errMsg = err + NetUtils::GetSocketErrorString();
		throw SocketException(errMsg);
	}

	//
	// After successfully creating a socket, we must immediately wrap
	// the os-supplied socket descriptor in our own reference-counted
	// SocketDescriptor object.
	//
	m_rpSocketDescriptor = new SocketDescriptor(socketFD);

	//
	// DatagramSockets require SO_BROADCAST enabled by default
	//
	setIntOption(SOL_SOCKET, SO_BROADCAST, 1);
}

//==============================================================================
// PlainDatagramSocketImpl::connect
//
// Connects the datagram socket with the host specified in the passed InetAddress
// using the port passed.
//
//==============================================================================
void PlainDatagramSocketImpl::connect(InetAddress* pAddress, int port)
{
	if(!pAddress) throw NullPointerException();
	connectToAddress(pAddress, port);
}

//==============================================================================
// PlainDatagramSocketImpl::close
//
// Close the underlying operating system socket.
//
// This forces a close of the OS socket descriptor, this is different to the
// action we take on destruction, where the reference count of the
// SocketDescriptor is simply decremented.
//
// In common with most io close methods, further close operation have no effect.
//==============================================================================
void PlainDatagramSocketImpl::close()
{
	if(m_rpSocketDescriptor)
	{
		m_rpSocketDescriptor->close();
		m_rpSocketDescriptor.release();
	}
}

//==============================================================================
// PlainDatagramSocketImpl::connectToAddress
//
// Common socket initialization function
//==============================================================================
void PlainDatagramSocketImpl::connectToAddress(InetAddress* pAddress, int port)
{
	testSocketIsValid();

	//
	// create and initialize a sockaddr_in structure
	// specifying the requested port.
	//
	struct sockaddr_in sa;
	::memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	if(pAddress && port!=-1)
	{
		sa.sin_port = htons(port); // convert port to network byte order 
		::memcpy(&sa.sin_addr, pAddress->getAddress(), pAddress->getAddressLength());
	}
	else
	{
		port = -1;
	}

	//
	// Call ::connect() to create the socket connection.
	// ::connect() will return zero is successful, otherwise -1.
	//
	if(::connect(m_rpSocketDescriptor->getFD(), reinterpret_cast<struct sockaddr*>(&sa), sizeof(struct sockaddr_in)) < 0)
	{
		String errMsg = NetUtils::GetSocketErrorString();
		throw SocketException(errMsg);
	}

	m_remotePort = port;
	if(pAddress)
	{
		m_rpRemoteAddr = new InetAddress(*pAddress);
	}
	else
	{
		m_rpRemoteAddr.release();
	}

	if(Tracer::IsEnabled())
	{
		String traceMsg = QC_T("socket: ");
		traceMsg += m_rpSocketDescriptor->toString() + QC_T(" connected to ");
		traceMsg += pAddress->toString() + QC_T(":") + NumUtils::ToString(port);
		Tracer::Trace(Tracer::Net, Tracer::Medium, traceMsg);
	}
}

//==============================================================================
// PlainDatagramSocketImpl::disconnect
//
//==============================================================================
void PlainDatagramSocketImpl::disconnect()
{
	testSocketIsValid();
	connectToAddress(0,-1);
}

//==============================================================================
// PlainDatagramSocketImpl::getLocalAddress
//
//==============================================================================
AutoPtr<InetAddress> PlainDatagramSocketImpl::getLocalAddress() const
{
	//
	// If the local address has not been requested before, do so now...
	//
	if(!m_rpLocalAddr && m_rpSocketDescriptor)
	{
		struct sockaddr_in sa;
		::memset(&sa, 0, sizeof(sa));
		sa.sin_family = AF_INET;
		cel_socklen_t sasize = sizeof(struct sockaddr_in);
		struct sockaddr* pAddr = reinterpret_cast<struct sockaddr*>(&sa);

		if(::getsockname(m_rpSocketDescriptor->getFD(), pAddr, &sasize) < 0)
		{
			String errMsg = NetUtils::GetSocketErrorString();
			throw SocketException(errMsg);
		}

		// Handle unsupported mutable
		((PlainDatagramSocketImpl*)this)->m_rpLocalAddr = InetAddress::FromNetworkAddress(pAddr, sasize);
		((PlainDatagramSocketImpl*)this)->m_localPort = ntohs(sa.sin_port);
	}
	
	return m_rpLocalAddr;
}

//==============================================================================
// PlainDatagramSocketImpl::getInetAddress
//
//==============================================================================
AutoPtr<InetAddress> PlainDatagramSocketImpl::getInetAddress() const
{
	return m_rpRemoteAddr;
}

//==============================================================================
// PlainDatagramSocketImpl::toString
//
//==============================================================================
String PlainDatagramSocketImpl::toString() const
{
	String ret = QC_T("addr=");
	ret += getInetAddress()->toString();
	ret += QC_T(",port=");
	ret += NumUtils::ToString(getPort());
	ret += QC_T(",localport=");
	ret += NumUtils::ToString(getLocalPort());
	return ret;
}

//==============================================================================
// PlainDatagramSocketImpl::getSocketDescriptor
//
//==============================================================================
AutoPtr<SocketDescriptor> PlainDatagramSocketImpl::getSocketDescriptor() const
{
	return m_rpSocketDescriptor;
}

//==============================================================================
// PlainDatagramSocketImpl::getLocalPort
//
//==============================================================================
int PlainDatagramSocketImpl::getLocalPort() const
{
	//
	// If the local port has not yet been assigned, detect it
	//
	if(m_localPort == -1)
		getLocalAddress();

	return m_localPort;
}

//==============================================================================
// PlainDatagramSocketImpl::getPort
//
//==============================================================================
int PlainDatagramSocketImpl::getPort() const
{
	return m_remotePort;
}

//==============================================================================
// PlainDatagramSocketImpl::bind
//
// Before a server socket enters the passive state (by listen()) it must be
// bound to a port and (optionally) a local interface.
//
// A null InetAddress means that the socket will be bound to ANY interface.
// A zero port indicates that the system will choose the next available port.
//==============================================================================
void PlainDatagramSocketImpl::bind(InetAddress* pAddress, int port)
{
	if(port == -1)
		throw IllegalArgumentException(QC_T("invalid port number"));

	testSocketIsValid();

	//
	// Create a sockaddr structure with the requested port and i/p addr
	//
	struct sockaddr_in sa;
	::memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port); // convert port to network byte order 

	if(pAddress)
	{
		m_rpLocalAddr = pAddress;
	}
	else
	{
		m_rpLocalAddr = InetAddress::GetAnyHost();
	}
	QC_DBG_ASSERT(m_rpLocalAddr);

	::memcpy(&sa.sin_addr.s_addr, m_rpLocalAddr->getAddress(), m_rpLocalAddr->getAddressLength());

	sockaddr* pLocalAddr = reinterpret_cast<struct sockaddr*>(&sa);

	if(::bind(m_rpSocketDescriptor->getFD(), pLocalAddr, sizeof(struct sockaddr_in)) < 0)
	{
		String errMsg = NetUtils::GetSocketErrorString();
		throw BindException(errMsg);
	}

	//
	// If a port number of zero was specified, the underlying system will assign
	// a port number automatically.  We must call getsockname() to retrieve the
	// port number assigned.
	//
	if(port == 0)
	{
		cel_socklen_t sasize = sizeof(struct sockaddr_in);
		if(::getsockname(m_rpSocketDescriptor->getFD(), pLocalAddr, &sasize) < 0)
		{
			String errMsg = NetUtils::GetSocketErrorString();
			throw SocketException(errMsg);
		}
		port = ntohs(sa.sin_port);
	}

	m_localPort = port;
}

//==============================================================================
// PlainDatagramSocketImpl::getIntOption
//
// Helper function to get a socket option
//==============================================================================
int PlainDatagramSocketImpl::getIntOption(int level, int option) const
{
	testSocketIsValid();
	
	int retValue = 0;
	cel_socklen_t retLen = sizeof(retValue);

	if(::getsockopt(m_rpSocketDescriptor->getFD(),
	                level,
	                option,
	                (char*)&retValue,
	                &retLen) < 0)
	{
		static const String err(QC_T("error retrieving socket option: "));
		String errMsg = err + NetUtils::GetSocketErrorString();
		throw SocketException(errMsg);
	}

	return retValue;
}

//==============================================================================
// PlainDatagramSocketImpl::setIntOption
//
// Helper function to set a socket option
//==============================================================================
void PlainDatagramSocketImpl::setIntOption(int level, int option, int value)
{
	testSocketIsValid();

	if(::setsockopt(m_rpSocketDescriptor->getFD(),
	                level,
	                option,
	                (char*)&value,
	                sizeof(value)) < 0)
	{
		static const String err(QC_T("error setting socket option: "));
		String errMsg = err + NetUtils::GetSocketErrorString();
		throw SocketException(errMsg);
	}
}

//==============================================================================
// PlainDatagramSocketImpl::getVoidOption
//
// Helper function for generic option retrieval
//==============================================================================
void PlainDatagramSocketImpl::getVoidOption(int level, int option, void* pOut,
                                            size_t* pLen) const
{
	QC_DBG_ASSERT(pOut && pLen);
	if(!pOut || !pLen) throw NullPointerException();

	testSocketIsValid();
	
	cel_socklen_t retLen = *pLen;

	if(::getsockopt(m_rpSocketDescriptor->getFD(),
	                level,
	                option,
	                (char*)pOut,
	                &retLen) < 0)
	{
		static const String err(QC_T("error retrieving socket option"));
		String errMsg = err + NetUtils::GetSocketErrorString();	}
	
	*pLen = retLen;
}

//==============================================================================
// PlainDatagramSocketImpl::setVoidOption
//
// Helper function to set a generic socket option
//==============================================================================
void PlainDatagramSocketImpl::setVoidOption(int level, int option, void* pValue,
                                            size_t valLen)
{
	QC_DBG_ASSERT(pValue!=0);
	if(!pValue) throw NullPointerException();

	testSocketIsValid();

	if(::setsockopt(m_rpSocketDescriptor->getFD(),
	                level,
	                option,
	                (char*)pValue,
	                valLen) < 0)
	{
		static const String err(QC_T("error setting socket option: "));
		String errMsg = err + NetUtils::GetSocketErrorString();
		throw SocketException(errMsg);
	}
}

//==============================================================================
// PlainDatagramSocketImpl::testSocketIsValid
//
// Helper function to implement common tests
//==============================================================================
void PlainDatagramSocketImpl::testSocketIsValid() const
{
	if(!m_rpSocketDescriptor)
	{
		const static String err = QC_T("Socket closed");
		throw SocketException(err);
	}
}

//==============================================================================
// PlainDatagramSocketImpl::getTimeout
//
// Return a value for the pseudo-option SO_TIMEOUT
//==============================================================================
size_t PlainDatagramSocketImpl::getTimeout() const
{
	return m_nTimeoutMS;
}

//==============================================================================
// PlainDatagramSocketImpl::setTimeout
//
// Set the value for the pseudo-option SO_TIMEOUT.  As the socket option
// SO_RCVTIMEO is not widely supported (okay on winsock 2, not on Linux),
// we simulate it using a select() call.
//
// The SocketInputStream does its own i/o, so for active sockets
// we pass the time limit on to the InputStream.  For passive sockets
// we store the value so that it is available the next time we perform
// an accept() call.
//==============================================================================
void PlainDatagramSocketImpl::setTimeout(size_t timeoutMS)
{
	m_nTimeoutMS = timeoutMS;
}

//==============================================================================
// PlainDatagramSocketImpl::isBound
//
//==============================================================================
bool PlainDatagramSocketImpl::isBound() const
{
	return (m_rpSocketDescriptor && m_localPort != -1);
}

//==============================================================================
// PlainDatagramSocketImpl::isClosed
//
//==============================================================================
bool PlainDatagramSocketImpl::isClosed() const
{
	return (m_rpSocketDescriptor.isNull());
}

//==============================================================================
// PlainDatagramSocketImpl::isConnected
//
//==============================================================================
bool PlainDatagramSocketImpl::isConnected() const
{
	return (m_rpSocketDescriptor && m_remotePort != -1);
}

//==============================================================================
// PlainDatagramSocketImpl::send
//
//==============================================================================
void PlainDatagramSocketImpl::send(const DatagramPacket& p)
{
	testSocketIsValid();

	int flags = 0;
	struct sockaddr* to=0;
	struct sockaddr_in sa;

	if(isConnected())
	{
		if(p.getAddress() && !(m_rpRemoteAddr->equals(*(p.getAddress()))))
		{
			throw IllegalArgumentException(QC_T("Address in datagram packet does not match connected address"));
		}
		if(p.getPort()!= -1 && p.getPort()!=m_remotePort)
		{
			throw IllegalArgumentException(QC_T("Port in datagram packet does not match connected port"));
		}
	}
	else
	{
		//
		// If the socket is not connected, then the passed datagram packet must contain
		// valid information.
		//
		if(!p.getAddress() || p.getPort()==-1)
		{
			throw IllegalArgumentException(QC_T("datagram packet does not contain required address/port information"));
		}
		//
		// Use the InetAddress and port contained in the packet to 
		// create and initialize a sockaddr_in structure.
		//
		::memset(&sa, 0, sizeof(sa));
		sa.sin_family = AF_INET;
		sa.sin_port = htons(p.getPort()); // convert port to network byte order 
		::memcpy(&sa.sin_addr, p.getAddress()->getAddress(), p.getAddress()->getAddressLength());
		to = reinterpret_cast<struct sockaddr*>(&sa);
	}

	if(Tracer::IsEnabled())
	{
		Tracer::TraceBytes(Tracer::Net, Tracer::Low, QC_T("Datagram send:"), p.getData(), p.getLength());
	}

	int rc = ::sendto(m_rpSocketDescriptor->getFD(),
	                  (const char*) p.getData(),
	                  p.getLength(),
	                  flags,
	                  to,
	                  to ? sizeof(struct sockaddr_in) : 0);

	if(rc<0)
	{
		static const String err(QC_T("error calling sendto "));
		String errMsg = err + NetUtils::GetSocketErrorString();
		throw SocketException(errMsg);
	}

	//
	// When an unbound datagram socket is used to send data, the system will allocate
	// it an address and ephemeral port.  Let's see what weve been given!
	//
	if(!isBound())
	{
	}
}

//==============================================================================
// PlainDatagramSocketImpl::receive
//
//==============================================================================
void PlainDatagramSocketImpl::receive(DatagramPacket& p)
{
	testSocketIsValid();

	sockaddr_in remoteAddr;
	cel_socklen_t addrLen;
	sockaddr* pRemoteAddr = reinterpret_cast<struct sockaddr*>(&remoteAddr);
	int recvLength;

	//
	// The following code is placed within a while() block to attempt to unify
	// the behaviour on multiple platforms.  If this datagram socket has been
	// used to send a dataram packet, an ICMP response may be received to
	// indicate that the send request was unsuccessful.  This behaviour is not
	// always helpful and is not guaranteed to occur (but seems to on Windows).
	// There is a far greater liklihood of this behaviour occurring on all platforms
	// when the socket has been connected to a particular host.  For this reason,
	// connect errors are ignored unless the socket has been connected.
	//
	while(true)
	{
		// simulate SO_TIMEOUT
		if(m_nTimeoutMS && !NetUtils::SelectSocket(m_rpSocketDescriptor.get(), m_nTimeoutMS,true,false))
		{
			static const String err(QC_T("receive timed out"));
			throw SocketTimeoutException(err);
		}

		int flags = 0;
		addrLen = sizeof(remoteAddr);
		recvLength = recvfrom(m_rpSocketDescriptor->getFD(),
		                      (char*) p.getData(),
		                      p.getLength(),
		                      flags,
		                      pRemoteAddr,
		                      &addrLen);

		if(recvLength < 0)
		{
			const int errorNum = NetUtils::GetLastSocketError();

			// Failure of recvfrom may be cause by the buffer length being smaller
			// than the message (in which case we silently ignore it, and set the
			// received length equal to the buffer length supplied), or due to
			// some other serious problem which will result in a SocketException
			//
			if(errorNum == QC_EMSGSIZE)
			{
				recvLength = p.getLength();
				break;
			}
			else if(!isConnected() && (errorNum == QC_ECONNRESET || errorNum == QC_ECONNABORTED || errorNum == QC_EHOSTUNREACH))
			{
				// See detailed comment above
				continue;
			}
			else
			{
				static const String err(QC_T("error calling recvfrom "));
				String errMsg = err + NetUtils::GetSocketErrorString(errorNum);
				throw SocketException(errMsg);
			}
		}
		else
		{
			break;
		}
	}
	
	//
	// Note: this test should migrate to the InetAddress class
	//
	if(addrLen != sizeof(sockaddr_in))
	{
		static const String err(QC_T("recvfrom() returned invalid address size"));
		throw SocketException(err);
	}

	//
	// Update the DatagramPacket with the address/length info
	//
	p.setPort(ntohs(remoteAddr.sin_port));
	p.setAddress(InetAddress::FromNetworkAddress(pRemoteAddr, addrLen).get());
	p.setLength(recvLength);

	if(Tracer::IsEnabled())
	{
		Tracer::TraceBytes(Tracer::Net, Tracer::Low, QC_T("Datagram rcvd:"), p.getData(), recvLength);
	}
}

QC_NET_NAMESPACE_END

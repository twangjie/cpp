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
	@class qc::net::MulticastSocket
	
	@brief A socket used for sending and receiving IP multicast packets.

	A MulticastSocket is a (UDP) DatagramSocket with the additional capabilities
	required for sending and receiving multicast packets.  This is reflected
	by the fact that MulticastSocket publicly <i>inherits</i> from DatagramSocket.

	IP multicasting is an efficient model for sending large quantities of data 
	over a network while minimizing the amount of bandwidth used.  Multicasting
	is based on the concept of groups, where datagrams are sent to a multicast 
	group and any number of clients can join one or more groups.  

	A multicast group is specified by a class D IP address and UDP port number.
	Class D IP addresses are in the range 224.0.0.0 to 239.255.255.255 inclusive
	(but 224.0.0.0 is reserved and may not be used). 

	When a datagram packet is sent to a multicast group, all subscribers
	to that group (IP address and port) receive the message if they are within
	the time-to-live range of the packet.  The time-to-live range of a datagram
	packet is set when the packet is sent, and is used to limit the number of
	multicast routing points that the packet will traverse.
	
	The actual work of the MulticastSocket is performed by an instance of the 
	DatagramSocketImpl class (the implementation class is loaded with knowledge
	about multi-casting even though the DatagramSocket interface is not). 
	Instances of DatagramSocketImpl are created using a factory class: 
	DatagramSocketImplFactory. An application can change
	the factory that creates the DatagramSocket implementation by calling 
	MulticastSocket::SetDatagramSocketImplFactory().  QuickCPP provides a default
	factory that creates plain DatagramSocketImpl instances which are suitable for
	most purposes.

	@since 1.3
*/
//==============================================================================

#include "MulticastSocket.h"
#include "NetUtils.h"
#include "SocketException.h"
#include "DatagramSocketImpl.h"
#include "InetAddress.h"
#include "SocketDescriptor.h"

#include "QcCore/base/NullPointerException.h"

#ifdef WIN32
	#include <Ws2tcpip.h>
#endif //WIN32

QC_NET_NAMESPACE_BEGIN

//==============================================================================
// MulticastSocket::MulticastSocket
//
/**
   Creates an unbound multicast socket.

   The registered DatagramSocket factory is used to create an instance of
   DatagramSocketImpl to perform the actual work for the multicast socket.

   When the socket is created the SO_REUSEADDR socket option is enabled.
   
   @throws SocketException if an error occurs creating the socket.
*/
//==============================================================================
MulticastSocket::MulticastSocket()
{
	setReuseAddress(true);
}

//==============================================================================
// MulticastSocket::MulticastSocket
//
/**
	Creates a multicast socket and binds it to the specified port on
	the local machine.

	The registered DatagramSocket factory is used to create an instance of
	DatagramSocketImpl to perform the actual work for the DatagramSocket.

    When the socket is created the SO_REUSEADDR socket option is enabled.

	@param port the local port number to bind to
	@throws SocketException if an error occurs creating or binding
	        the socket.
	@throws IllegalArgumentException if port is not in the valid range
	@sa connect()
*/
//==============================================================================
MulticastSocket::MulticastSocket(int port)
{
	setReuseAddress(true);
	bind(port, 0);
}

//==============================================================================
// MulticastSocket::getNetworkInterface
//
/**
	Returns an InetAddress representing the local interface from which
	multicast packets will be sent.

	This is the value associated with the @c IP_MULTICAST_IF socket option.

	@returns an InetAddress representing the multicast network interface.
	@throws SocketException is an error occurs getting the interface.
	@sa setNetworkInterface()
	`*/
//==============================================================================
AutoPtr<InetAddress> MulticastSocket::getNetworkInterface() const
{
	sockaddr addr;
	size_t addrLen = sizeof(addr);
	getDatagramSocketImpl()->getVoidOption(IPPROTO_IP, IP_MULTICAST_IF, &addr, &addrLen);
	return InetAddress::FromNetworkAddress(&addr, addrLen);
}

//==============================================================================
// MulticastSocket::setNetworkInterface
//
/**
	Sets the local multicast network interface from which multicast
	packets will be sent.

	This sets the value of the @c IP_MULTICAST_IF socket option.

	@param inf an InetAddress representing the required network interface.
	@throws NullPointerException if @c pInf is null.
	@throws SocketException if an error occurs while setting the @c IP_MULTICAST_IF option.
	@sa getNetworkInterface()
*/
//==============================================================================
void MulticastSocket::setNetworkInterface(const InetAddress* pInf)
{
	if(!pInf) throw NullPointerException();
	getDatagramSocketImpl()->setVoidOption(IPPROTO_IP, IP_MULTICAST_IF, (void*)pInf->getAddress(), pInf->getAddressLength());
}

//==============================================================================
// MulticastSocket::getTimeToLive
//
/**
	Returns the time-to-live value for multicast packets sent using this multicast socket.

	The @c IP_MULTICAST_TTL option is used.

	@sa setTimeToLive()
*/
//==============================================================================
int MulticastSocket::getTimeToLive() const
{
	return getDatagramSocketImpl()->getIntOption(IPPROTO_IP, IP_MULTICAST_TTL);
}

//==============================================================================
// MulticastSocket::setTimeToLive
//
/**
	Set the time-to-live (TTL) value for multicast packets
	sent using this multicast socket.

	The TTL value for a multicast packet specifies how many "hops" between 
	multicast routers the packet can make before it expires. 

	@param ttl the time-to-live value.
	@throws SocketException if an error occurs setting the @c IP_MULTICAST_TTL
	        option.
*/
//==============================================================================
void MulticastSocket::setTimeToLive(int ttl)
{
	getDatagramSocketImpl()->setIntOption(IPPROTO_IP, IP_MULTICAST_TTL, ttl);
}

//==============================================================================
// MulticastSocket::joinGroup
//
/**
	Joins a multicast group.
	@param pMulticastAddr the IP address of the multicast group to join.
	@throws NullPointerException if @c pMulticastAddr is null.
	@throws SocketException if an error occurs joining the multicast group.
	@sa setNetworkInterface()
*/
//==============================================================================
void MulticastSocket::joinGroup(const InetAddress* pMulticastAddr)
{
	joinGroup(pMulticastAddr,0);
}

//==============================================================================
// MulticastSocket::joinGroup
//
/**
	Joins a multicast group.

	@param pMulticastAddr the IP address of the multicast group to join.
	@param pInf the network interface on which to join or null to allow the system
	       to select an interface
	@throws NullPointerException if @c pMulticastAddr is null.
	@throws SocketException if an error occurs joining the multicast group.
	@sa setNetworkInterface()
*/
//==============================================================================
void MulticastSocket::joinGroup(const InetAddress* pMulticastAddr, const InetAddress* pInf)
{
	if(!pMulticastAddr) throw NullPointerException();

	ip_mreq mreq;
	::memcpy(&mreq.imr_multiaddr, pMulticastAddr->getAddress(), sizeof(mreq.imr_multiaddr));
	if(pInf)
		::memcpy(&mreq.imr_interface, pInf->getAddress(), sizeof(mreq.imr_interface));
	else
		mreq.imr_interface.s_addr = INADDR_ANY;
	getDatagramSocketImpl()->setVoidOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
}

//==============================================================================
// MulticastSocket::leaveGroup
//
/**
	Leaves a multicast group.

	It is not necessary to leave a multicast group in order to terminate. 
	QuickCPP closes the socket when all references are released and at this
	time the operating system ensures that all memberships associated with the
	socket are dropped.

	@param pMulticastAddr the IP address of the multicast group to leave.
    @throws NullPointerException if @c pMulticastAddr is null.
	@throws SocketException if an error occurs leaving the multicast group.
	@sa setNetworkInterface()
*/
//==============================================================================
void MulticastSocket::leaveGroup(const InetAddress* pMulticastAddr)
{
	leaveGroup(pMulticastAddr, 0);
}

//==============================================================================
// MulticastSocket::leaveGroup
//
/**
	Leaves a multicast group.

	It is not necessary to leave a multicast group in order to terminate. 
	QuickCPP closes the socket when all references are released and at this
	time the operating system ensures that all memberships associated with the
	socket are dropped.

	@param pMulticastAddr the IP address of the multicast group to leave.
	@param pInf the network interface from which to leave or null to allow the system
	       to select an interface
	@throws NullPointerException if @c pMulticastAddr is null.
	@throws SocketException if an error occurs leaving the multicast group.
	@sa setNetworkInterface()
*/
//==============================================================================
void MulticastSocket::leaveGroup(const InetAddress* pMulticastAddr, const InetAddress* pInf)
{
	if(!pMulticastAddr) throw NullPointerException();
	ip_mreq mreq;
	::memcpy(&mreq.imr_multiaddr, pMulticastAddr->getAddress(), sizeof(mreq.imr_multiaddr));
	if(pInf)
		::memcpy(&mreq.imr_interface, pInf->getAddress(), sizeof(mreq.imr_interface));
	else
		mreq.imr_interface.s_addr = INADDR_ANY;
	getDatagramSocketImpl()->setVoidOption(IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
}

QC_NET_NAMESPACE_END

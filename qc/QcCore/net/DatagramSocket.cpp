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
	@class qc::net::DatagramSocket
	
	@brief Provides a Berkeley socket interface for UDP datagram sockets.

	A DatagramSocket is an end-point for unreliable (UDP) communication
	between two or more networked machines. 

	The DatagramSocket class manages a datagram socket handle which is provided by the
	underlying operating system.  This handle is represented by a 
	SocketDescriptor, which enables QuickCPP to close the socket handle 
	when it is no longer required.

	When a DatagramSocket is created, it can be bound to a specific local interface
	by using an appropriate constructor, or it can be bound after 
	construction using the bind() method.

	The ability to send and receive broadcast packets is enabled by default
	for DatagramSockets.  This option may be controlled using the setBroadcast()
	method.

	The actual work of the DatagramSocket is performed by an instance of the 
	DatagramSocketImpl class.  Instances of DatagramSocketImpl are created
	using a factory class: DatagramSocketImplFactory. An application can change
	the factory that creates the DatagramSocket implementation by calling 
	SetDatagramSocketImplFactory() on this class.  QuickCPP provides a default
	factory that creates plain DatagramSocketImpl instances which are suitable for
	most purposes.

	@since 1.3
*/
//==============================================================================

#include "DatagramSocket.h"
#include "NetUtils.h"
#include "SocketException.h"
#include "DatagramSocketImpl.h"
#include "DatagramSocketImplFactory.h"
#include "InetAddress.h"
#include "SocketDescriptor.h"

#include "QcCore/base/System.h"
#include "QcCore/base/ObjectManager.h"
#include "QcCore/base/NullPointerException.h"
#include "QcCore/base/FastMutex.h"

#ifndef WIN32
#include <netinet/tcp.h>
#endif //WIN32

QC_NET_NAMESPACE_BEGIN

//==================================================================
// Multi-threaded locking strategy
//
// To ensure that singleton resources are not created by multiple
// concurrent threads, the pointers are protected using a
// single static mutex.
//
// Update access to static variables is mutex protected, but to 
// minimize the runtime cost, read access is not protected.  This
// gives an exposure to the so-called "relaxed" memory model that
// exists on some multi-processor machines.  We minimize this
// exposure by declaring the static variables as 'volatile'.
//==================================================================
#ifdef QC_MT
	FastMutex DatagramSocketMutex;
#endif //QC_MT

DatagramSocketImplFactory* QC_MT_VOLATILE DatagramSocket::s_pDatagramSocketImplFactory = NULL;

//==============================================================================
// DatagramSocket::DatagramSocket
//
/**
   Creates an unbound datagram socket.

   The registered DatagramSocket factory is used to create an instance of
   DatagramSocketImpl to perform the actual work for the DatagramSocket.

   @throws SocketException if an error occurs creating the DatagramSocket.
*/
//==============================================================================
DatagramSocket::DatagramSocket()
{
	//
	// Create a DatagramSocketImpl instance
	//
	createDatagramSocketImpl();
}

//==============================================================================
// DatagramSocket::DatagramSocket
//
/**
	Creates a DatagramSocket and binds it to the specified port on
	and wildcard address on the local machine.

	The registered DatagramSocket factory is used to create an instance of
	DatagramSocketImpl to perform the actual work for the DatagramSocket.

	@param port the local port number to bind to
	@throws IllegalArgumentException if port is not in the valid range
	@throws SocketException if an error occurs creating or binding
	        the DatagramSocket.
	@sa connect()
*/
//==============================================================================
DatagramSocket::DatagramSocket(int port)
{
	//
	// Create a DatagramSocketImpl instance
	//
	createDatagramSocketImpl();

	//
	// And bind it to the requested addr/port
	//
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	m_rpDatagramSocketImpl->bind(0, port);
}

//==============================================================================
// DatagramSocket::DatagramSocket
//
/**
	Creates a DatagramSocket and binds it to the specified interface and port on
	the local machine.

	The registered DatagramSocket factory is used to create an instance of
	DatagramSocketImpl to perform the actual work for the DatagramSocket.
  
	@throws NullPointerException if pAddress is null.
	@throws SocketException if an error occurs creating or binding
	        the DatagramSocket.
	@sa connect()
*/
//==============================================================================
DatagramSocket::DatagramSocket(int port, InetAddress* pAddress)
{
	if(!pAddress) throw NullPointerException();

	//
	// Create a DatagramSocketImpl instance
	//
	createDatagramSocketImpl();

	//
	// And bind it to the requested addr/port
	//
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	m_rpDatagramSocketImpl->bind(pAddress, port);
}

//==============================================================================
// DatagramSocket::bind()
//
/**
	Binds this datagram socket to a particular local IP address and port.

	@param port the local port number to bind to.  If zero, the system will
	       pick an ephemeral port (usually above 1024)
	@param pLocalAddress an InetAddress pointer representing the interface
	       to bind to.  When this is null a valid local address will be used.
	@throws SocketException if an error occurs binding the DatagramSocket.
	@sa connect()
*/
//==============================================================================
void DatagramSocket::bind(int port, InetAddress* pLocalAddr)
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	m_rpDatagramSocketImpl->bind(pLocalAddr, port);
}

//==============================================================================
// DatagramSocket::close
//
/**
	Closes the DatagramSocket and releases any system resources associated with it.

	Generally, applications will not need to call this function because the
	DatagramSocket is automatically closed when the reference-count of the
	DatagramSocketDescriptor is decremented to zero.

	Subsequent send or receive operations will fail with an IOException.

	@throws SocketException if an error occurs closing the DatagramSocket.
*/
//==============================================================================
void DatagramSocket::close()
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	m_rpDatagramSocketImpl->close();
}

//==============================================================================
// DatagramSocket::connect
//
/**
	Connects a DatagramSocket to a specified remote address and port.

	When a datagram socket is connected to a remote address, packets may only
	be sent to or received from that address.  When the remote address
	is a multicast address, only send operations are permitted.
	
	When a datagram socket is connected, packets received from other 
	addresses are silently ignored.  By default a datagram socket is not
	connected.

	@param pAddress an InetAddress representing the remote host
	@param port the port number on the remote host
	@throws NullPointerException if @c pAddress is null.
	@throws SocketException if an error occurs connecting to the remote host.
	@sa isConnected()
	@sa disconnect()
*/
//==============================================================================
void DatagramSocket::connect(InetAddress* pAddress, int port)
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	m_rpDatagramSocketImpl->connect(pAddress, port);
}

//==============================================================================
// DatagramSocket::disconnect
//
/**
	Disconnects the DatagramSocket from its remote destination.

	When a datagram socket is disconnected, packets may
	be sent to any host.
	
	@throws SocketException if an error occurs disconnecting the socket.
	@sa connect()
*/
//==============================================================================
void DatagramSocket::disconnect()
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	m_rpDatagramSocketImpl->disconnect();
}

//==============================================================================
// DatagramSocket::createDatagramSocketImpl
//
// Common DatagramSocket initialization function
//==============================================================================
void DatagramSocket::createDatagramSocketImpl()
{
	QC_DBG_ASSERT(!m_rpDatagramSocketImpl);
	NetUtils::InitializeSocketLibrary();
	m_rpDatagramSocketImpl = GetDatagramSocketImplFactory()->createDatagramSocketImpl();
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	m_rpDatagramSocketImpl->create();
}

//==============================================================================
// DatagramSocket::getInetAddress
//
/**
	Returns an InetAddress representing the remote host to which this DatagramSocket
	is connected.

	If this DatagramSocket has never been connected or is not currently 
	connected a null AutoPtr is returned.
	@returns the remote InetAddress or null.

	@sa getLocalAddress
*/
//==============================================================================
AutoPtr<InetAddress> DatagramSocket::getInetAddress() const
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	return m_rpDatagramSocketImpl->getInetAddress();
}

//==============================================================================
// DatagramSocket::SetDatagramSocketImplFactory
//
/**
	Sets the client DatagramSocket implementation factory for the application.

	When an application creates a new DatagramSocket, the registered DatagramSocket implementation
	factory's createDatagramSocketImpl() method is called to create the actual DatagramSocketImpl
	instance which performs the work for the DatagramSocket.

	An application should only call this function if it wishes to employ a
	customized DatagramSocketImpl class.

	To ensure that the passed object exists for as long as the application needs it, 
	the DatagramSocketImplFactory object is registered with the system's ObjectManager 
	which holds a (counted) reference to it until system termination.  This
	absolves the application from having to manage the lifetime of the passed
	object.  For example, the following code is perfectly valid and does not
	cause a memory leak:-

    @code
    int main(int argc, char* argv[])
    {
      SystemMonitor _monitor();   // ensures clean termination
      // register a customized DatagramSocket implementation factory
      DatagramSocket::SetDatagramSocketImplFactory(new MyDatagramSocketImplFactory);
      ...
    }
    @endcode

	@param pFac a pointer to a DatagramSocketImplFactory
	@throws NullPointerException if @c pFac is null.
	@mtsafe
*/
//==============================================================================
void DatagramSocket::SetDatagramSocketImplFactory(DatagramSocketImplFactory* pFac)
{
	//==================================================================
	// Multi-threaded locking strategy
	//
	// See top of file.
	//==================================================================

	if(pFac) 
	{
		System::GetObjectManager().registerObject(pFac);
	}

	DatagramSocketImplFactory* pExisting;

	// create a scope for the mutex lock
	{
		QC_AUTO_LOCK(FastMutex, DatagramSocketMutex);
		pExisting = s_pDatagramSocketImplFactory;
		s_pDatagramSocketImplFactory = pFac;
	}

	if(pExisting) 
	{
		System::GetObjectManager().unregisterObject(pExisting);
	}
}

//==============================================================================
// DatagramSocket::GetDatagramSocketImplFactory
//
/**
	Returns the static object that is the DatagramSocketImplFactory for 
	DatagramSockets.

	An instance of the default DatagramSocketImplFactory is created if a factory
	has not already been registered by the application.
*/
//==============================================================================
AutoPtr<DatagramSocketImplFactory> DatagramSocket::GetDatagramSocketImplFactory()
{
	if(s_pDatagramSocketImplFactory == NULL)
	{
		SetDatagramSocketImplFactory(new DatagramSocketImplFactory);
	}
	QC_DBG_ASSERT(s_pDatagramSocketImplFactory!=0);
	return s_pDatagramSocketImplFactory;
}

//==============================================================================
// DatagramSocket::getLocalAddress
//
/**
	Returns an InetAddress representing the local interface to which this
	datagram socket is bound (if any).

	If this DatagramSocket has never been bound a null AutoPtr is returned.
	@returns the local InetAddress or null if the datagram socket has not been
	         bound to a local interface.
	@sa getInetAddress()
*/
//==============================================================================
AutoPtr<InetAddress> DatagramSocket::getLocalAddress() const
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	return m_rpDatagramSocketImpl->getLocalAddress();
}

//==============================================================================
// DatagramSocket::getLocalPort
//
/**
	Returns the local port number to which this DatagramSocket is bound.
	@returns the local port number or -1 if this DatagramSocket is not yet bound.
*/
//==============================================================================
int DatagramSocket::getLocalPort() const
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	return m_rpDatagramSocketImpl->getLocalPort();
}

//==============================================================================
// DatagramSocket::getPort
//
/**
	Returns the remote port to which this DatagramSocket is connected. 
	@returns the remote port number of -1 if this DatagramSocket is not connected.
*/
//==============================================================================
int DatagramSocket::getPort() const
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	return m_rpDatagramSocketImpl->getPort();
}

//==============================================================================
// DatagramSocket::getReceiveBufferSize
//
/**
	Returns the @c SO_RVCBUF option for this DatagramSocket, which is the size
	of the buffer used by the operating system to hold received data before it 
	is read by the application.

	@returns the value of the @c SO_RCVBUF option for this DatagramSocket
	@throws SocketException if an error occurs while interrogating the DatagramSocket.
	@sa setReceiveBufferSize()
*/
//==============================================================================
int DatagramSocket::getReceiveBufferSize() const
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	return m_rpDatagramSocketImpl->getIntOption(SOL_SOCKET, SO_RCVBUF);
}

//==============================================================================
// DatagramSocket::setReceiveBufferSize
//
/**
	Sets the @c SO_RCVBUF option for this datagram socket, which is the size of 
	the buffer used by the operating system to hold received data before it is 
	read by the application.

	The underlying operating system need not use the precise buffer size
	specified in this call (it is treated more like a hint than a command).
	To find out the actual receive buffer size being used, applications should
	call the getReceiveBufferSize() method.

	@param size the requested buffer size (in bytes)
	@throws SocketException if an error occurs while setting the @c SO_RCVBUF option
	@sa getReceiveBufferSize()
*/
//==============================================================================
void DatagramSocket::setReceiveBufferSize(size_t size)
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	m_rpDatagramSocketImpl->setIntOption(SOL_SOCKET, SO_RCVBUF, size);
}

//==============================================================================
// DatagramSocket::getSendBufferSize
//
/**
	Returns the value of the @c SO_SNDBUF option for this datagram socket, which
	is the size of the buffer used by the operating system to store output data 
	from the application before it is sent to (and acknowledged by) the remote host. 

	@throws SocketException if an error occurs while interrogating the datagram
	        socket.
*/
//==============================================================================
int DatagramSocket::getSendBufferSize() const
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	return m_rpDatagramSocketImpl->getIntOption(SOL_SOCKET, SO_SNDBUF);
}

//==============================================================================
// DatagramSocket::setSendBufferSize
//
/**
	Sets the @c SO_SNDBUF option for this datagram socket, which is the size
	of the buffer used by the operating system to store output data from the 
	application before it is sent to a remote host.

	@param size the requested buffer size (in bytes)
	@throws SocketException if an error occurs while setting the @c SO_SNDBUF option
	@sa getReceiveBufferSize()
*/
//==============================================================================
void DatagramSocket::setSendBufferSize(size_t size)
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	m_rpDatagramSocketImpl->setIntOption(SOL_SOCKET, SO_SNDBUF, size);
}

//==============================================================================
// DatagramSocket::getSoTimeout
//
/**
	Returns the value of the @c SO_TIMEOUT pseudo option.

	@c SO_TIMEOUT is not one of the options defined for Berkeley DatagramSockets, but
	was actually introduced as part of the Java API.  For client DatagramSockets
	it has the same meaning as the @c SO_RCVTIMEO option, which specifies
	the maximum number of milliseconds that a blocking @c read() call will
	wait for data to arrive on the DatagramSocket.
*/
//==============================================================================
size_t DatagramSocket::getSoTimeout() const
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	return m_rpDatagramSocketImpl->getTimeout();
}

//==============================================================================
// DatagramSocket::setSoTimeout
//
/**
	Enables/disables the @c SO_TIMEOUT pseudo option.

	@c SO_TIMEOUT is not one of the options defined for Berkeley sockets, but
	was actually introduced as part of the Java API.  For datagram sockets
	it has the same meaning as the @c SO_RCVTIMEO option, which specifies
	the maximum number of milliseconds that a blocking @c receive() call will
	wait for data to arrive on the datagram socket.

	@param timeoutMS the specified timeout value, in milliseconds.  A value
	       of zero indicates no timeout, i.e. an infinite wait.
*/
//==============================================================================
void DatagramSocket::setSoTimeout(size_t timeoutMS)
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	m_rpDatagramSocketImpl->setTimeout(timeoutMS);
}

//==============================================================================
// DatagramSocket::toString
//
/**
	Returns a string representation of this DatagramSocket.

	The format of the returned string is dependent on the associated 
	DatagramSocketImpl instance, but plain DatagramSockets return a string in 
	the form:- DatagramSocket[localPort=n]
*/
//==============================================================================
String DatagramSocket::toString() const
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	String ret = QC_T("DatagramSocket[");
	ret += m_rpDatagramSocketImpl->toString();
	ret += QC_T("]");
	return ret;
}

//==============================================================================
// DatagramSocket::isBound
//
/**
   Tests if the datagram socket has been bound.
   @return @c true if the DatagramSocket has been bound; false otherwise.
*/
//==============================================================================
bool DatagramSocket::isBound() const
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	return m_rpDatagramSocketImpl->isBound();
}

//==============================================================================
// DatagramSocket::isClosed
//
/**
   Tests if the datagram socket has been closed.
   @return @c true if the DatagramSocket is closed; false otherwise.
*/
//==============================================================================
bool DatagramSocket::isClosed() const
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	return m_rpDatagramSocketImpl->isClosed();
}

//==============================================================================
// DatagramSocket::isConnected
//
/**
   Tests the connected status of this DatagramSocket.
   @return @c true if the DatagramSocket is connected; false otherwise.
*/
//==============================================================================
bool DatagramSocket::isConnected() const
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	return m_rpDatagramSocketImpl->isConnected();
}

//==============================================================================
// DatagramSocket::send
//
/**
	Sends a datagram packet using this datagram socket.

	The datagram packet to send is represented by a reference to a DatagramPacket
	object, which contains the data buffer to send, its length (which may be zero)
	and the IP address/port number of the remote host to which the packet will be sent.

	If this datagram socket has been connected to a remote host, the packet will
	be sent to the connected host.  

	@param p the datagram to send.
	@throws IllegalArgumentException if this datagram socket is connected to
	        a specific host/port and the datagram packet contains destination
	        information which does not match.
	@throws SocketException if an error occurs during the receive operation.
*/
//==============================================================================
void DatagramSocket::send(const DatagramPacket& p)
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	m_rpDatagramSocketImpl->send(p);
}

//==============================================================================
// DatagramSocket::receive
//
/**
	Receives a datagram packet from the network using this datagram socket.

	The datagram packet is received into a buffer provided by a DatagramPacket
	object.  The DatagramPacket's @c length field is used to control
	the number of bytes that are copied into the data buffer, any remaining
	bytes in the message are silently discarded.  
	
	Following a successful receive operation, the @c length field of the 
	DatagramPacket is updated to reflect the number of bytes copied.  In
	addition, the IP address and port number of the sender are copied into
	the respective fields of the passed DatagramPacket.

	If this datagram socket has been connected to a remote host, only packets
	originating from the connected host/port will be received.  Other packets
	will be silently dropped.

	This method will block until a datagram packet becomes available.  The call
	will block indefinitely unless a specific timeout value has been set with 
	setSoTimeout().

	@param p the DatagramPacket into which the datagram will be received.
	@throws SocketException if an error occurs during the receive operation.
	@throws SocketTimeoutException if setSoTimeout was previously called 
	        and the specified timeout value has expired.
*/
//==============================================================================
void DatagramSocket::receive(DatagramPacket& p)
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	m_rpDatagramSocketImpl->receive(p);
}

//==============================================================================
// DatagramSocket::getReuseAddress
//
/**
	Tests if the SO_REUSEADDR option is enabled for this datagram socket.

	By default, the SO_REUSEADDR option is not enabled for datagram sockets.

	@returns true if the SO_REUSEADDR option is enabled; false otherwise.
*/
//==============================================================================
bool DatagramSocket::getReuseAddress() const
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	return (m_rpDatagramSocketImpl->getIntOption(SOL_SOCKET, SO_REUSEADDR) != 0);
}

//==============================================================================
// DatagramSocket::setReuseAddress
//
/**
	Sets the SO_REUSEADDR option for this datagram socket.

	@param bEnable enables SO_REUSEADDR when true, disables it otherwise

	This method must be called before bind(), so should be used in conjunction
	with the default DatagramSocket() constructor.

	@note It is not usually necessary to set this option for UDP
	datagram sockets, but it is required for multicast sockets
	where several applications on the same host can all bind to and receive
	multicast packets on a single port.
*/
//==============================================================================
void DatagramSocket::setReuseAddress(bool bEnable)
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	m_rpDatagramSocketImpl->setIntOption(SOL_SOCKET, SO_REUSEADDR, bEnable);
}

//==============================================================================
// DatagramSocket::getBroadcast
//
/**
	Tests if the SO_BROADCAST option is enabled for this datagram socket.

	By default, the SO_BROADCAST option is enabled for datagram sockets.

	@returns true if the SO_BROADCAST option is enabled; false otherwise.
*/
//==============================================================================
bool DatagramSocket::getBroadcast() const
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	return (m_rpDatagramSocketImpl->getIntOption(SOL_SOCKET, SO_BROADCAST) != 0);
}

//==============================================================================
// DatagramSocket::setBroadcast
//
/**
	Sets the SO_BROADCAST option for this datagram socket.

	@param bEnable enables SO_BROADCAST when true, disables it otherwise

	This method must be called before bind(), so should be used in conjunction
	with the default DatagramSocket() constructor.
*/
//==============================================================================
void DatagramSocket::setBroadcast(bool bEnable)
{
	QC_DBG_ASSERT(m_rpDatagramSocketImpl);
	m_rpDatagramSocketImpl->setIntOption(SOL_SOCKET, SO_REUSEADDR, bEnable);
}

//==============================================================================
// DatagramSocket::getDatagramSocketImpl
//
/**
	Returns the DatagramSocketImp object used by this DatagramSocket.
*/
//==============================================================================
AutoPtr<DatagramSocketImpl> DatagramSocket::getDatagramSocketImpl() const
{
	return m_rpDatagramSocketImpl;
}

QC_NET_NAMESPACE_END

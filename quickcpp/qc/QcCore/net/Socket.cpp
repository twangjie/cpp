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
	@class qc::net::Socket
	
	@brief Provides a Berkeley socket interface for TCP/IP stream sockets.

    A TCP/IP socket is an end-point for reliable communication
	between two networked machines. 

    The Socket class manages a socket handle which is provided by the
	underlying operating system.  This handle is represented by a 
	SocketDescriptor, which enables QuickCPP to close the socket handle 
	when it is no longer required.

    When a Socket is created, it can be connected to a specific end-point
	by using an appropriate constructor, or it can be connected after 
	construction using one of the connect() methods.
	
    Data is sent over the Socket connection using an OutputStream
	returned from getOutputStream() and received using an InputStream returned 
	from getInputStream().

    The actual work of the Socket is performed by an instance of the 
    SocketImpl class.  Instances of SocketImpl are created using a factory
	class: SocketImplFactory. An application can change the socket factory that creates 
    the socket implementation by calling SetSocketImplFactory() on this class.
	The default socket factory creates plain SocketImpl instances which 
	do not perform any special processing.

    @mt
	Static functions are synchronized for safe access from multiple concurrent
	threads but instance member functions are not.  A Socket may be shared between
	multiple threads if the application provides adequate synchronization.  For 
	example, it is acceptable for one thread to read from a socket while another
	thread writes to the same socket, but if two threads wish to read from the
	same socket they will have to synchronize their access to the InputStream.
*/
//==============================================================================

#include "Socket.h"
#include "NetUtils.h"
#include "SocketException.h"
#include "SocketImpl.h"
#include "SocketImplFactory.h"
#include "InetAddress.h"
#include "SocketDescriptor.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"

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
// minimise the runtime cost, read access is not protected.  This
// gives an exposure to the so-called "relaxed" memory model that
// exists on some multi-processor machines.  We minimise this
// exposure by declaring the static variables as 'volatile'.
//==================================================================
#ifdef QC_MT
	FastMutex SocketMutex;
#endif //QC_MT

SocketImplFactory* QC_MT_VOLATILE Socket::s_pSocketImplFactory = NULL;

//==============================================================================
// Socket::Socket
//
/**
   Creates an unconnected socket.

   The registered socket factory is used to create an instance of
   SocketImpl to perform the actual work for the socket.

   Before the socket can be used to communicate with an end-point, it must
   first be connected using connect().

   @throws SocketException if an error occurs creating the socket.
*/
//==============================================================================
Socket::Socket()
{
	//
	// Create a SocketImpl instance
	//
	createSocketImpl();
}

//==============================================================================
// Socket::Socket
//
/**
   Creates a Socket and connects it to the specified port on the network host
   designated by the provided InetAddress.

   The registered socket factory is used to create an instance of
   SocketImpl to perform the actual work for the socket.
      
   @throws SocketException if an error occurs creating or connecting the socket.
   @throws NullPointerException if pAddress is null.
   @sa connect()
*/
//==============================================================================
Socket::Socket(InetAddress* pAddress, int port)
{
	if(!pAddress) throw NullPointerException();

	//
	// Create a SocketImpl instance
	//
	createSocketImpl();

	//
	// And connect it to the requested addr/port
	//
	QC_DBG_ASSERT(m_rpSocketImpl);
	m_rpSocketImpl->connect(pAddress, port);
}

//==============================================================================
// Socket::Socket
//
/**
   Creates a Socket that is connected to a specified host name and port.

   The host name is first resolved into an InetAddress before connecting
   to it.

   @param host the name of the remote host or a string representation
          of its IP-address in the form "xxx.xxx.xxx.xxx".
   @throws UnknownHostException if @c host cannot be resolved into
           an Internet address.
   @throws SocketException if an error occurs creating or connecting the socket.
   @sa connect()
*/
//==============================================================================
Socket::Socket(const String& host, int port)
{
	//
	// First lookup the host name and convert to an InetAddress instance
	//
	AutoPtr<InetAddress> rpAddr = InetAddress::GetByName(host);
	QC_DBG_ASSERT(rpAddr);

	//
	// Create a SocketImpl instance
	//
	createSocketImpl();

	//
	// And connect it to the requested addr/port
	//
	QC_DBG_ASSERT(m_rpSocketImpl);
	m_rpSocketImpl->connect(rpAddr.get(), port);
}

//==============================================================================
// Socket::Socket
//
/**
   Creates a Socket and connects it to the specified remote port on the network host
   designated by the provided InetAddress.  The created Socket is bound
   to the specified local port and InetAddress.

   If @c pLocalAddr is null, the socket is bound to any interface on the local
   host.  If @c localPort is 0, the operating system chooses an unused local
   port number.

   @throws NullPointerException if @c pAddress is null.
   @throws SocketException if an error occurs creating, binding or connecting 
           the socket.
   @sa connect()
   @sa bind()
*/
//==============================================================================
Socket::Socket(InetAddress* pAddress, int port, InetAddress* pLocalAddr, int localPort)
{
	//
	// Create a SocketImpl instance
	//
	createSocketImpl();

	m_rpSocketImpl->bind(pLocalAddr, localPort);
	
	// nulll pointers are detected by SocketImpl
	m_rpSocketImpl->connect(pAddress, port);
}

//==============================================================================
// Socket::Socket
//
/**
   Protected constructor that creates a Socket using the passed SocketImpl.

   @throws NullPointerException if @c pSocketImpl is null.
*/
//==============================================================================
Socket::Socket(SocketImpl* pSocketImpl) : 
	m_rpSocketImpl(pSocketImpl)
{
	if(!pSocketImpl) throw NullPointerException();
}

//==============================================================================
// Socket::close
//
/**
   Closes the Socket and releases any system resources associated with it.

   This forces the underlying socket handle to be closed, even if the
   reference count of the contained SocketDescriptor indicates that it is being
   shared by other objects such as an InputStream or OutputStream.

   Generally, applications will not need to call this function because the
   socket is automatically closed when the reference-count of the
   SocketDescriptor is decremented to zero.

   Subsequent read or write operations will fail with an IOException except
   when using buffered streams, which may not register that the socket has been
   closed until the next time the buffer is refreshed or flushed.

   @throws SocketException if an error occurs closing the socket.
*/
//==============================================================================
void Socket::close()
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	m_rpSocketImpl->close();
}

//==============================================================================
// Socket::connect
//
/**
   Connects an unconnected socket to a port on a remote host.  This overloaded
   method takes a timeout value which is used to specify (in milliseconds) the
   maximum time that the connection attempt should last before being aborted.

   @param pAddress an InetAddress representing the remote host
   @param port the port number on the remote host
   @param timeoutMS a time-out value in milliseconds
   @throws NullPointerException if @c pAddress is null.
   @throws SocketException if an error occurs connecting to the remote host.
   @throws SocketTimeoutException if @c timeoutMS milliseconds elapses before
           the connection is successfully established.
*/
//==============================================================================
void Socket::connect(InetAddress* pAddress, int port, size_t timeoutMS)
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	m_rpSocketImpl->connect(pAddress, port, timeoutMS);
}

//==============================================================================
// Socket::connect
//
/**
   Connects an unconnected socket to a port on a remote host.

   @param pAddress an InetAddress representing the remote host
   @param port the port number on the remote host
   @throws NullPointerException if @c pAddress is null.
   @throws SocketException if an error occurs connecting to the remote host.
*/
//==============================================================================
void Socket::connect(InetAddress* pAddress, int port)
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	m_rpSocketImpl->connect(pAddress, port);
}

//==============================================================================
// Socket::connect
//
/**
   Connects an unconnected socket to a port on a remote host.

   @param host the name of the remote host or a string representation
          of its IP-address in the form "xxx.xxx.xxx.xxx".
   @param port the port number on the remote host
   @throws UnknownHostException if @c host cannot be resolved into
           an Internet address.
   @throws SocketException if an error occurs connecting to the remote host.
*/
//==============================================================================
void Socket::connect(const String& host, int port)
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	m_rpSocketImpl->connect(host, port);
}

//==============================================================================
// Socket::createSocketImpl
//
// Common socket initialization function
//==============================================================================
void Socket::createSocketImpl()
{
	QC_DBG_ASSERT(!m_rpSocketImpl);

	NetUtils::InitializeSocketLibrary();

	m_rpSocketImpl = GetSocketImplFactory()->createSocketImpl();

	QC_DBG_ASSERT(m_rpSocketImpl);
	m_rpSocketImpl->create(true);
}

//==============================================================================
// Socket::getInetAddress
//
/**
   Returns an InetAddress representing the remote host to which this Socket
   is connected.

   If this socket has never been connected a null AutoPtr is returned.
   @returns the remote InetAddress or null.
   @sa getLocalAddress()
*/
//==============================================================================
AutoPtr<InetAddress> Socket::getInetAddress() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	return m_rpSocketImpl->getInetAddress();
}

//==============================================================================
// Socket::getInputStream
//
/**
   Returns an InputStream that can be used to read data from this Socket.

   Multiple calls to getInputStream() are guaranteed to always return
   the same object.
   @sa getOutputStream()
*/
//==============================================================================
AutoPtr<InputStream> Socket::getInputStream() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	return m_rpSocketImpl->getInputStream();
}

//==============================================================================
// Socket::getOutputStream
//
/**
   Returns an OutputStream that can be used to write data to this Socket.

   Multiple calls to getOutputStream() are guaranteed to always return
   the same object.
   @sa getInputStream()
*/
//==============================================================================
AutoPtr<OutputStream> Socket::getOutputStream() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	return m_rpSocketImpl->getOutputStream();
}

//==============================================================================
// Socket::SetSocketImplFactory
//
/**
   Sets the client socket implementation factory for the application.
   
   When an application creates a new Socket, the registered socket implementation
   factory's createSocketImpl() method is called to create the actual SocketImpl
   instance which performs the work for the Socket.
   
   An application should only call this function if it wishes to employ a
   customized SocketImpl class.

   To ensure that the passed object exists for as long as the application needs it, 
   the SocketImplFactory object is registered with the system's ObjectManager 
   which holds a (counted) reference to it until system termination.  This
   absolves the application from having to manage the lifetime of the passed
   object.  For example, the following code is perfectly valid and does not
   cause a memory leak:-
   
   @code
   int main(int argc, char* argv[])
   {
      SystemMonitor _monitor();   // ensures clean termination
      // register a customized socket implementation factory
      Socket::SetSocketImplFactory(new MySocketImplFactory);
      ...
   }
   @endcode

   @param pFac a pointer to a SocketImplFactory
   @throws NullPointerException if @c pFac is null.
   @mtsafe
*/
//==============================================================================
void Socket::SetSocketImplFactory(SocketImplFactory* pFac)
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

	SocketImplFactory* pExisting;

	// create a scope for the mutex lock
	{
		QC_AUTO_LOCK(FastMutex, SocketMutex);
		pExisting = s_pSocketImplFactory;
		s_pSocketImplFactory = pFac;
	}

	if(pExisting) 
	{
		System::GetObjectManager().unregisterObject(pExisting);
	}
}

//==============================================================================
// Socket::GetSocketImplFactory
//
/**
	Returns the static object that is the SocketImplFactory for client sockets.

    An instance of the default SocketImplFactory is created if a factory has not 
	already been registered by the application.
*/
//==============================================================================
AutoPtr<SocketImplFactory> Socket::GetSocketImplFactory()
{
	if(s_pSocketImplFactory == NULL)
	{
		SetSocketImplFactory(new SocketImplFactory);
	}
	QC_DBG_ASSERT(s_pSocketImplFactory!=0);
	return s_pSocketImplFactory;
}

//==============================================================================
// Socket::getLocalAddress
//
/**
   Returns an InetAddress representing the local address to which this
   Socket is bound.
   If this socket has never been bound a null AutoPtr is returned.
   @returns the local InetAddress or null if the socket has not been bound.
   @sa bind()
   @sa getInetAddress()
*/
//==============================================================================
AutoPtr<InetAddress> Socket::getLocalAddress() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	return m_rpSocketImpl->getLocalAddress();
}

//==============================================================================
// Socket::getLocalPort
//
/**
   Returns the local port number to which this socket is bound.
   @returns the local port number of -1 if this Socket is not yet connected.
*/
//==============================================================================
int Socket::getLocalPort() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	return m_rpSocketImpl->getLocalPort();
}

//==============================================================================
// Socket::getPort
//
/**
   Returns the remote port to which this socket is connected. 
   @returns the remote port number of -1 if this Socket is not yet connected.
*/
//==============================================================================
int Socket::getPort() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	return m_rpSocketImpl->getPort();
}

//==============================================================================
// Socket::getReceiveBufferSize
//
/**
   Returns the @c SO_RVCBUF option for this Socket, which is the size of the buffer
   used by the operating system to hold received data before it is read by 
   the application.

   @returns the value of the @c SO_RCVBUF option for this Socket
   @throws SocketException if an error occurs while interrogating the socket.
   @sa setReceiveBufferSize()
*/
//==============================================================================
int Socket::getReceiveBufferSize() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	return m_rpSocketImpl->getIntOption(SOL_SOCKET, SO_RCVBUF);
}

//==============================================================================
// Socket::setReceiveBufferSize
//
/**
   Sets the @c SO_RCVBUF option for this Socket, which is the size of the buffer 
   used by the operating system to hold received data before it is read by 
   the application.

   <p>The @c SO_RCVBUF option can only be set before a client socket is connected
   to a remote host (buffer sizes are negotiated between hosts when the
   connection is established).  A SocketException will be thrown if this method
   is called on a connected client Socket.</p>

   @param size the requested buffer size (in bytes)
   @throws SocketException if an error occurs while setting the @c SO_RCVBUF option
   @sa isConnected()
   @sa getReceiveBufferSize()
*/
//==============================================================================
void Socket::setReceiveBufferSize(size_t size)
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	
	if(isConnected())
		throw SocketException(QC_T("cannot set option on a connected socket"));

	m_rpSocketImpl->setIntOption(SOL_SOCKET, SO_RCVBUF, size);
}

//==============================================================================
// Socket::getSendBufferSize
//
/**
   Returns the value of the @c SO_SNDBUF option for this Socket, which is the size
   of the buffer used by the operating system to store output data from the 
   application before it is sent to (and acknowledged by) the connected host. 

   @throws SocketException if an error occurs while interrogating the socket.
*/
//==============================================================================
int Socket::getSendBufferSize() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	return m_rpSocketImpl->getIntOption(SOL_SOCKET, SO_SNDBUF);
}

//==============================================================================
// Socket::setSendBufferSize
//
/**
   Sets the @c SO_SNDBUF option for this Socket, which is the size
   of the buffer used by the operating system to store output data from the 
   application before it is sent to (and acknowledged by) the connected host.

   <p>The @c SO_SNDBUF option can only be set before a client socket is connected
   to a remote host (buffer sizes are negotiated between hosts when the
   connection is established).  A SocketException will be thrown if this method
   is called on a connected client Socket.</p>

   @param size the requested buffer size (in bytes)
   @throws SocketException if an error occurs while setting the @c SO_SNDBUF option
   @sa isConnected()
   @sa getReceiveBufferSize()
*/
//==============================================================================
void Socket::setSendBufferSize(size_t size)
{
	QC_DBG_ASSERT(m_rpSocketImpl);

	if(isConnected())
		throw SocketException(QC_T("cannot set option on a connected socket"));

	m_rpSocketImpl->setIntOption(SOL_SOCKET, SO_SNDBUF, size);
}

//==============================================================================
// Socket::getSoLinger
//
/**
   Returns the setting for the @c SO_LINGER option for this Socket.

   @returns the value of the @c SO_LINGER option.  A return value of -1
            indicates that @c SO_LINGER is disabled for this Socket.
   @throws SocketException if an error occurs while interrogating the socket.
*/
//==============================================================================
int Socket::getSoLinger() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);

	struct linger val;
	size_t valLen = sizeof(val);
	m_rpSocketImpl->getVoidOption(SOL_SOCKET, SO_LINGER, &val, &valLen);
	if(val.l_onoff)
	{
		return val.l_linger;
	}
	else
	{
		return -1;
	}
}

//==============================================================================
// Socket::setSoLinger
//
/**
   Sets the @c SO_LINGER option for this Socket.

   <p>The @c SO_LINGER option specifies how the close() function operates.
   By default, close() returns immediately, but if there is any data still
   remaining in the socket's send buffer, the operating system will 
   attempt to deliver that data to the connected host.</p>

   <p>When the @c SO_LINGER option is enabled, calls to close() will wait for
   up to a specified length of time until that data has been sent and
   acknowledged.</p>

   @param bEnable @c true to enable the @c SO_LINGER option, @c false to disable it.
   @param linger the desired time-out (in seconds).
   @throws SocketException if an error occurs while setting the @c SO_LINGER option.
*/
//==============================================================================
void Socket::setSoLinger(bool bEnable, size_t linger)
{
	QC_DBG_ASSERT(m_rpSocketImpl);

	struct linger val;
	val.l_onoff = bEnable;
	val.l_linger = linger;
	size_t valLen = sizeof(val);

	m_rpSocketImpl->setVoidOption(SOL_SOCKET, SO_LINGER, &val, valLen);
}

//==============================================================================
// Socket::getSoTimeout
//
/**
   Returns the value of the @c SO_TIMEOUT pseudo option.

   @c SO_TIMEOUT is not one of the options defined for Berkeley sockets, but
   was actually introduced as part of the Java API.  For client sockets
   it has the same meaning as the @c SO_RCVTIMEO option, which specifies
   the maximum number of milliseconds that a blocking @c read() call will
   wait for data to arrive on the socket.
*/
//==============================================================================
size_t Socket::getSoTimeout() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);

	return m_rpSocketImpl->getTimeout();

	//
	// SO_RCVTIMEO is poorly documented in Winsock API, but it appears
	// to be measured as an int value in milliseconds, whereas BSD-style
	// sockets use a timeval
	//
	//
	// For compatability between platforms, we do not use the SO_RCVTIMEO
	// option.  It is read-only under Linux, so we implement it internally
	// by using a non-blocking recv.
#if 0
#if defined(WIN32)
	return m_rpSocketImpl->getIntOption(SOL_SOCKET, SO_RCVTIMEO);
#else
	// Convert the timeout value (in milliseconds) into a timeval struct
	struct timeval timer;
	size_t valLen = sizeof(timer);
	m_rpSocketImpl->getVoidOption(SOL_SOCKET, SO_RCVTIMEO, &timer, &valLen);
	size_t ret = (timer.tv_sec * 1000) + (timer.tv_usec / 1000);
	return ret;
#endif
#endif
}

//==============================================================================
// Socket::setSoTimeout
//
/**
   Enables/disables the @c SO_TIMEOUT pseudo option.

   @c SO_TIMEOUT is not one of the options defined for Berkeley sockets, but
   was actually introduced as part of the Java API.  For client sockets
   it has the same meaning as the @c SO_RCVTIMEO option, which specifies
   the maximum number of milliseconds that a blocking @c read() call will
   wait for data to arrive on the socket.

   @param timeoutMS the specified timeout value, in milliseconds.  A value
          of zero indicates no timeout, i.e. an infinite wait.
*/
//
// This is not a real option, but it closely resembles the SO_RCVTIMEO
// option.  Sadly, SO_RCVTIMEO is not widely supported.  Winsock 2 supports
// it but Linux does not allow it to be updated.
//
// For this reason the SocketImpl has a custom method to set the timeout, so
// that it can use its intelligence to emulate the timeout (which it does
// with a select() call.
//==============================================================================
void Socket::setSoTimeout(size_t timeoutMS)
{
	QC_DBG_ASSERT(m_rpSocketImpl);

	m_rpSocketImpl->setTimeout(timeoutMS);

	//
	// SO_RCVTIMEO is poorly documented in Winsock API, but it appears
	// to be measured as an int value in milliseconds, whereas BSD-style
	// sockets use a timeval
	//

#if 0
#if defined(WIN32)
	m_rpSocketImpl->setIntOption(SOL_SOCKET, SO_RCVTIMEO, timeoutMS);
#else
	// Convert the timeout value (in milliseconds) into a timeval struct
	struct timeval timer;
	timer.tv_sec = timeoutMS / 1000;
	timer.tv_usec = (timeoutMS % 1000) * 1000;
	size_t valLen = sizeof(timer);
	m_rpSocketImpl->setVoidOption(SOL_SOCKET, SO_RCVTIMEO, &timer, valLen);
#endif
#endif
}

//==============================================================================
// Socket::getTcpNoDelay
//
/**
   Tests if the @c TCP_NODELAY option is enabled for this Socket.

   When set, the @c TCP_NODELAY option disables <em>Nagles algorithm</em>,
   which is enabled by default.

   @returns @c true if @c TCP_NODELAY is enabled; false otherwise.
   @throws SocketException if an error occurs while interrogating the socket.
*/
//==============================================================================
bool Socket::getTcpNoDelay() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	return (m_rpSocketImpl->getIntOption(IPPROTO_TCP, TCP_NODELAY) != 0);
}

//==============================================================================
// Socket::setTcpNoDelay
//
/**
   Enables/disables the @c TCP_NODELAY option for this Socket.

   When set, the @c TCP_NODELAY option disables <em>Nagles algorithm</em>,
   which is enabled by default.

   @param bEnable if @c true the option is enabled; otherwise it is disabled.
   @throws SocketException if an error occurs while setting the @c TCP_NODELAY 
           option.
*/
//==============================================================================
void Socket::setTcpNoDelay(bool bEnable)
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	m_rpSocketImpl->setIntOption(IPPROTO_TCP, TCP_NODELAY, bEnable);
}

//==============================================================================
// Socket::getAutoClose
//
/**
   Tests if @a auto_close is enabled.

   @returns @c true if @a auto_close is enabled; false otherwise.
   @sa setAutoClose()
*/
//==============================================================================
bool Socket::getAutoClose() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	return m_rpSocketImpl->getAutoClose();
}

//==============================================================================
// Socket::getKeepAlive
//
/**
   Tests if @c SO_KEEPALIVE is enabled.

   @returns @c true if @c SO_KEEPALIVE is enabled; false otherwise.
   @throws SocketException if an error occurs while interrogating the socket.
   @sa setKeepAlive()
*/
//==============================================================================
bool Socket::getKeepAlive() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	return (m_rpSocketImpl->getIntOption(SOL_SOCKET, SO_KEEPALIVE) != 0);
}

//==============================================================================
// Socket::setAutoClose
//
/**
   Enables/disables the @a auto_close feature.

   The QuickCPP @a auto_close facility is used to close a socket automatically
   when no OutputStream is referenced for the socket and an EndOfFile marker
   is read from the socket's InputStream. This feature may be useful for 
   classes which return an InputStream and require the user to close the stream
   when OndOfFile is reached.

   @param bEnable if @c true the option is enabled; otherwise it is disabled.
   @throws SocketException if an error occurs while setting the @a auto_close
           option.
   @sa getAutoClose()
*/
//==============================================================================
void Socket::setAutoClose(bool bEnable)
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	m_rpSocketImpl->setAutoClose(bEnable);
}

//==============================================================================
// Socket::setKeepAlive
//
/**
   Enables/disables the @c SO_KEEPALIVE option for this Socket.

   When set, the @c SO_KEEPALIVE option causes the operating system to
   send a heartbeat packet at regular intervals when no other data is being
   sent over the socket.

   Note that the time interval between heartbeat packets is controlled
   by the operating system, and may be as long as 2 hours.

   @param bEnable if @c true the option is enabled; otherwise it is disabled.
   @throws SocketException if an error occurs while setting the @c SO_KEEPALIVE
           option.
   @sa getKeepAlive()
*/
//==============================================================================
void Socket::setKeepAlive(bool bEnable)
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	m_rpSocketImpl->setIntOption(SOL_SOCKET, SO_KEEPALIVE, bEnable);
}

//==============================================================================
// Socket::shutdownInput
//
/**
   Shuts down this Socket for input operations.

   Any unread data in the socket's receive buffer is discarded.
   Any read operations from this socket's InputStream
   will result in an EndOfFile condition.

   @throws SocketException if an error occurs shutting down the socket
*/
//==============================================================================
void Socket::shutdownInput()
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	m_rpSocketImpl->shutdownInput();
}

//==============================================================================
// Socket::shutdownOutput
//
/**
   Shuts down this Socket for output operations.

   Any previously written data will be flushed to the peer host, 
   followed by TCP's normal connection termination sequence. If an attempt
   is made to write data to the socket's output stream after invoking
   shutdownOutput(), an IOException will be thrown.
   @throws SocketException if an error occurs shutting down the socket
*/
//==============================================================================
void Socket::shutdownOutput()
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	m_rpSocketImpl->shutdownOutput();
}

//==============================================================================
// Socket::toString
//
/**
   Returns a string representation of this Socket.

   The format of the returned string is dependent on the associated SocketImpl
   instance, but plain sockets return a string in the form:-
   Socket[addr=host/1.2.3.4,port=n,localport=n]
*/
//==============================================================================
String Socket::toString() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	String ret = QC_T("Socket[");
	ret += m_rpSocketImpl->toString();
	ret += QC_T("]");
	return ret;
}

//==============================================================================
// Socket::getSocketImpl
//
// Private helper for use by ServerSocket.
//==============================================================================
AutoPtr<SocketImpl> Socket::getSocketImpl() const
{
	return m_rpSocketImpl;
}

//==============================================================================
// Socket::isConnected
//
/**
   Tests the connected status of this Socket.
   @return @c true if the Socket is connected; false otherwise.
*/
//==============================================================================
bool Socket::isConnected()
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	return m_rpSocketImpl->isConnected();
}

//==============================================================================
// Socket::isClosed
//
/**
   Tests if this socket is closed.
   @return @c true if the Socket is closed; false otherwise.
   @since 1.3
*/
//==============================================================================
bool Socket::isClosed()
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	return m_rpSocketImpl->isClosed();
}

QC_NET_NAMESPACE_END

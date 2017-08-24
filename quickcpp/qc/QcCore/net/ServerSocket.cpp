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
// Class ServerSocket
/**
	@class qc::net::ServerSocket
	
	@brief Handles incoming TCP/IP connections.

    The ServerSocket class contains the methods needed to write a portable
	TCP/IP server.
	
    In general, a TCP/IP server listens for incoming network requests on a
	well-known IP address and port number.  When a connection request is received,
	the ServerSocket makes this connection available to the server program as
	a new Socket.  The new Socket represents a two-way (full-duplex) connection 
	with the client, with an InputStream for the data flowing from the client
	to the server and an OutputStream going the other way.

    In common with normal socket programming, the life-cycle of a ServerSocket
	follows this basic course:-
	-# bind() to an IP-address/port number and listen for incoming connections
	-# accept() a connection request
	-# deal with the request, or pass the created Socket to another Thread or process 
	  to be dealt with
	-# return to step 2 for the next client connection request

    The actual work of the ServerSocket is performed by an instance of the 
    SocketImpl class.  Instances of SocketImpl are created using a factory
	class: SocketImplFactory. An application can change the socket factory that creates 
    the socket implementation by calling SetSocketImplFactory() on this class.
*/
//==============================================================================

#include "ServerSocket.h"
#include "InetAddress.h"
#include "NetUtils.h"
#include "Socket.h"
#include "SocketDescriptor.h"
#include "SocketException.h"
#include "SocketImpl.h"
#include "SocketImplFactory.h"

#include "QcCore/base/System.h"
#include "QcCore/base/ObjectManager.h"
#include "QcCore/base/FastMutex.h"

#ifndef WIN32
#include <sys/socket.h>
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
	FastMutex ServerSocketMutex;
#endif //QC_MT

const String sNet = QC_T("net");

SocketImplFactory* QC_MT_VOLATILE ServerSocket::s_pSocketImplFactory = NULL;

static int ListenDefault = 50;

//==============================================================================
// ServerSocket::ServerSocket
//
/**
   Constructs a ServerSocket that is not bound to an IP-address or port number.
   
   Before this ServerSocket can be used to accept() incoming connection
   requests, bind() must be called.

   Use this constructor when you need to set options that must be set before
   calling bind() such as setReuseAddress() and setReceiveBufferSize().

   @throws SocketException if an error occurs whilst creating the socket.
   @sa bind()
   @sa isBound()
*/
//==============================================================================
ServerSocket::ServerSocket() :
	m_bIsBound(false)
{
	init();
}

//==============================================================================
// ServerSocket::ServerSocket
//
/**
   Constructs a ServerSocket that is bound to the port number @c port.

   The maximum queue length for incoming connection requests is set to a default
   value.  If a connection request arrives when the queue is full, the request
   is refused. 
   
   @param port the port number to bind to.  If the port number is zero the system
          will assign an unused port number.
          
   @throws SocketException if an error occurs whilst creating the socket.
   @throws BindException if an error occurs whilst binding to the requested port.
*/
//==============================================================================
ServerSocket::ServerSocket(int port) :
	m_bIsBound(false)
{
	init();
	bind(port);
}

//==============================================================================
// ServerSocket::ServerSocket
//
/**
   Constructs a ServerSocket that is bound to the port number @c port.

   The maximum queue length for incoming connection requests is set to @c backlog.
   If a connection request arrives when the queue is full, the request is refused. 
   
   @param port the port number to bind to.  If the port number is zero the system
          will assign an unused port number.
   @param backlog the maximum queue length for pending connection requests.
   @throws SocketException if an error occurs whilst creating the socket.
   @throws BindException if an error occurs whilst binding to the requested port.
*/
//==============================================================================
ServerSocket::ServerSocket(int port, int backlog) :
	m_bIsBound(false)
{
	init();
	bind(port, backlog);
}

//==============================================================================
// ServerSocket::ServerSocket
//
/**
   Constructs a ServerSocket that is bound to the port number @c port and the 
   local IP-address @c pBindAddr.

   The maximum queue length for incoming connection requests is set to @c backlog.
   If a connection request arrives when the queue is full, the request is refused. 
   
   @param port the port number to bind to.  If the port number is zero the system
          will assign an unused port number.
   @param backlog the maximum queue length for pending connection requests.
   @param pBindAddr a pointer to an InetAddress representing the IP-address
          of a local network interface on which to listen for incoming requests.
   @throws SocketException if an error occurs whilst creating the socket.
   @throws BindException if an error occurs whilst binding to the requested port
           and IP-address.
   @throws NullPointerException if @c pBindAddr is null.
*/
//==============================================================================
ServerSocket::ServerSocket(int port, int backlog, InetAddress* pBindAddr) :
	m_bIsBound(false)
{
	init();
	bind(port, backlog, pBindAddr);
}

//==============================================================================
// ServerSocket::bind
//
/**
   Binds this socket to the port number @c port and the local IP-address @c pBindAddr.

   The maximum queue length for incoming connection requests is set to @c backlog.
   If a connection request arrives when the queue is full, the request is refused. 
   
   @param port the port number to bind to.  If the port number is zero the system
          will assign an unused port number.
   @param backlog the maximum queue length for pending connection requests.
   @param pBindAddr a pointer to an InetAddress representing the IP-address
          of a local network interface on which to listen for incoming requests.
   @throws SocketException if an error occurs whilst obtaining the local
           port number if a port number of zero was provided.
   @throws BindException if an error occurs whilst binding to the requested port
           and IP-address.
   @throws NullPointerException if @c pBindAddr is null
*/
//==============================================================================
void ServerSocket::bind(int port, int backlog, InetAddress* pBindAddr)
{
	m_rpSocketImpl->bind(pBindAddr, port);
	m_rpSocketImpl->listen(backlog);
}
	
//==============================================================================
// ServerSocket::bind
//
/**
   Binds this socket to the port number @c port.

   The maximum queue length for incoming connection requests is set to @c backlog.
   If a connection request arrives when the queue is full, the request is refused. 
   
   @param port the port number to bind to.  If the port number is zero the system
          will assign an unused port number.
   @param backlog the maximum queue length for pending connection requests.
   @throws SocketException if an error occurs whilst obtaining the local
           port number if a port number of zero was provided.
   @throws BindException if an error occurs whilst binding to the requested port.
*/
//==============================================================================
void ServerSocket::bind(int port, int backlog)
{
	m_rpSocketImpl->bind(0, port);
	m_rpSocketImpl->listen(backlog);
}
	
//==============================================================================
// ServerSocket::bind
//
/**
   Binds this socket to the port number @c port.

   The maximum queue length for incoming connection requests is set to a default
   value.  If a connection request arrives when the queue is full, the request
   is refused. 

   @param port the port number to bind to.  If the port number is zero the system
          will assign an unused port number.
   @throws SocketException if an error occurs whilst obtaining the local
           port number if a port number of zero was provided.
   @throws BindException if an error occurs whilst binding to the requested port.
*/
//==============================================================================
void ServerSocket::bind(int port)
{
	m_rpSocketImpl->bind(0, port);
	m_rpSocketImpl->listen(ListenDefault);
}

//==============================================================================
// ServerSocket::close
//
/**
   Closes the SocketImpl socket wrapped by this ServerSocket.

   Note that on some platforms close() is implemented as a blocking call that may
   not return until the socket has been successfully closed.  If close() is called
   by a separate thread while a blocking accept() call is active, the close()
   may not succeed until the next client connection request is received.

   @note The ServerSocket will be made non-blocking if a timeout value
   is specified using setSoTimeout().
*/
// This forces a close of the OS ServerSocket descriptor, this is different to the
// action we take on destruction, where the reference count of the
// SocketImpl is simply decremented, resulting in a corresponding decrement
// of the OSServerSocketDescriptor's reference count.
//==============================================================================
void ServerSocket::close()
{
	if(m_rpSocketImpl)
	{
		m_rpSocketImpl->close();
	}
}

//==============================================================================
// ServerSocket::getInetAddress
//
/**
   Returns an InetAddress object representing the local IP-address
   to which this ServerSocket is bound.

   @returns the bound InetAddress.
*/
//==============================================================================
AutoPtr<InetAddress> ServerSocket::getInetAddress() const
{
	return m_rpSocketImpl->getInetAddress();
}

//==============================================================================
// ServerSocket::isBound
//
/**
   Returns the binding state of the ServerSocket.

   @returns true if this ServerSocket has been bound; false otherwise.
   @sa bind()
*/
//==============================================================================
bool ServerSocket::isBound() const
{
	return m_rpSocketImpl->isBound();
}

//==============================================================================
// ServerSocket::SetServerSocketImplFactory
//
/**
   Registers a single instance of a ServerSocketImplFactory to act as the
   factory for creating new SocketImpl instances for use by ServerSockets.
  
   The ServerSocket shares ownership of the passed factory by incrementing
   the reference count.  This ensures the factory is not destroyed while
   it is still needed.  The reference count is decremented at system
   termination or when another SocketImplFactory is registered.
  
   @param pFac pointer to the SocketImplFactory to be registered.
   @mtsafe
*/
//==============================================================================
void ServerSocket::SetSocketImplFactory(SocketImplFactory* pFac)
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
		QC_AUTO_LOCK(FastMutex, ServerSocketMutex);
		pExisting = s_pSocketImplFactory;
		s_pSocketImplFactory = pFac;
	}

	if(pExisting) 
	{
		System::GetObjectManager().unregisterObject(pExisting);
	}
}

//==============================================================================
// ServerSocket::GetServerSocketImplFactory()
//
/**
   Returns the single instance of SocketImplFactory that has been registered
   with ServerSocket for the application.

   An instance of SocketImplFactory is automatically created if the application
   has not already registered one.

   The application should not use the returned factory to create instances of
   SocketImpl.  The only reason this method is present is to allow an application
   or third-party @QuickCPP extension to chain SocketImplFactories together.

   @returns The SocketImplFactory used for creating instances of SocketImpl
            for ServerSockets.
*/
//==============================================================================
AutoPtr<SocketImplFactory> ServerSocket::GetSocketImplFactory()
{
	if(s_pSocketImplFactory == NULL)
	{
		SetSocketImplFactory(new SocketImplFactory);
	}
	QC_DBG_ASSERT(s_pSocketImplFactory!=0);
	return s_pSocketImplFactory;
}

//==============================================================================
// ServerSocket::accept
//
/**
   Returns a new Socket object representing a client connection to the server.

   accept() blocks until a connection request is received or the timeout value
   specified with setSoTimeout() is exceeded.

   <h4>Cancelling an active accept call</h4>
   The only portable way to stop a thread that's in an accept() loop is to 
   poll for completion by setting a reasonably short timeout value with
   setSoTimeout().

   On some platforms, calling close() on the ServerSocket
   from another thread will force the accept() to abort immediately and
   throw a SocketException.  Unfortunately this behaviour is not guaranteed on
   all platforms, so does not present a portable solution.  On some platforms,
   if the socket is a blocking socket, a close() call will block until the next
   connection request is received.  This is rarely the desired outcome.

   @returns A new Socket representing a two-way connection with the client.
   @throws InterruptedIOException if a network connection request is not
           received within the time limit set by an earlier call to setSoTimeout().
   @throws SocketException if an error occurs on the underlying socket.
   @sa setSoTimeout()
*/
//==============================================================================
AutoPtr<Socket> ServerSocket::accept()
{
	AutoPtr<Socket> rpSocket = new Socket();
	implAccept(rpSocket.get());
	return rpSocket;
}

//==============================================================================
// ServerSocket::getLocalPort
//
/**
   Returns the number of the local port on which this ServerSocket is listening.

   @returns the bound port number.
*/
//==============================================================================
int ServerSocket::getLocalPort() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	return m_rpSocketImpl->getLocalPort();
}
    
//==============================================================================
// ServerSocket::getSoTimeout
//
/**
   Returns the timeout value for this ServerSocket.

   This represents the maximum duration that a call to accept() will wait
   for an incoming connection request before timing-out.

   A value of zero indicates infinity (i.e. no timeout).

   @returns the timeout value (in milliseconds).
*/
//==============================================================================
size_t ServerSocket::getSoTimeout() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	return m_rpSocketImpl->getTimeout();
}

//==============================================================================
// ServerSocket::setSoTimeout
//
/**
   Sets the timeout value for this ServerSocket.
   For ServerSocket, the timeout value controls the length of time that calls
   to accept() will block while waiting for a client connection request.

   @param timeoutMS the timeout value in milliseconds.  A value of zero indicates
          infinity (i.e. no timeout).
   @sa accept()
*/
//==============================================================================
void ServerSocket::setSoTimeout(size_t timeoutMS)
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	m_rpSocketImpl->setTimeout(timeoutMS);
}

//==============================================================================
// ServerSocket::toString
//
/**
   Returns a string representation of this ServerSocket.

   @returns a string representation of this ServerSocket.
*/
//==============================================================================
String ServerSocket::toString() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	String ret = QC_T("ServerSocket[");
	ret += m_rpSocketImpl->toString();
	ret += QC_T("]");
	return ret;
}

//==============================================================================
// ServerSocket::implAccept
//
/**
   Implements accept().

   Derived classes that have overridden accept() may call this function
   to implement the accept logic.

   @param pSocket a pointer to a new Socket created by accept().
*/
//==============================================================================
void ServerSocket::implAccept(Socket* pSocket)
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	// need to call the QC extension func to get the socketImpl from
	// the passed socket
	AutoPtr<SocketImpl> rpSocketImpl = pSocket->getSocketImpl();
	m_rpSocketImpl->accept(rpSocketImpl.get());
}

//==============================================================================
// ServerSocket::getReceiveBufferSize
//
/**
   Returns the value of the @c SO_RCVBUF option for this ServerSocket.

   The value of the @c SO_RCVBUF is used by the underlying sockets implementation
   to negotiate the receive buffer size that will be used for Sockets accepted
   from this ServerSocket. 
   @returns the value of the @c SO_RCVBUF option for this ServerSocket.
*/
//==============================================================================
size_t ServerSocket::getReceiveBufferSize() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	return m_rpSocketImpl->getIntOption(SOL_SOCKET, SO_RCVBUF);
}

//==============================================================================
// ServerSocket::setReceiveBufferSize
//
/**
   Sets the value of the @c SO_RCVBUF option for this ServerSocket.
   
   The @c SO_RCVBUF option is used by the underlying sockets implementation
   as the proposed buffer size that will be used for Sockets accepted from this
   ServerSocket.

   Even though the receive buffer belongs to sockets accepted from this
   ServerSocket, the child sockets inherit the value of @c SO_RCVBUF from the
   listening ServerSocket because TCP will negotiate the buffer size during
   the 3-way handshake which occurs before the accept() function returns.

   This method must be called before bind(), so should be used in conjunction
   with the default ServerSocket() constructor.
*/
//==============================================================================
void ServerSocket::setReceiveBufferSize(size_t size)
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	m_rpSocketImpl->setIntOption(SOL_SOCKET, SO_RCVBUF, size);
}

//==============================================================================
// ServerSocket::getReuseAddress
//
/**
   Tests if the SO_REUSEADDR option is enabled.

   @returns true if the SO_REUSEADDR option is enabled; false otherwise.
*/
//==============================================================================
bool ServerSocket::getReuseAddress() const
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	return (m_rpSocketImpl->getIntOption(SOL_SOCKET, SO_REUSEADDR) != 0);
}

//==============================================================================
// ServerSocket::setReuseAddress
//
/**
   Sets the SO_REUSEADDR option for this ServerSocket.

   @param bEnable enables SO_REUSEADDR when true, disables it otherwise

   This method must be called before bind(), so should be used in conjunction
   with the default ServerSocket() constructor.
*/
//==============================================================================
void ServerSocket::setReuseAddress(bool bEnable)
{
	QC_DBG_ASSERT(m_rpSocketImpl);
	m_rpSocketImpl->setIntOption(SOL_SOCKET, SO_REUSEADDR, bEnable);
}

//==============================================================================
// ServerSocket::init
//
// Private, common ServerSocket initialization function
//==============================================================================
void ServerSocket::init()
{
	NetUtils::InitializeSocketLibrary();
	m_rpSocketImpl = GetSocketImplFactory()->createSocketImpl();
	QC_DBG_ASSERT(m_rpSocketImpl);
	m_rpSocketImpl->create(true);
}

QC_NET_NAMESPACE_END

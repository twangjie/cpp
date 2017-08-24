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
// 
// Class: TcpNetworkClient
// 
/**
	@class qc::net::TcpNetworkClient
	
	@brief An @a abstract base class for utilities that require a TCP/IP
	       connection to a network server. 

	This class doesn't do very much except manage a Socket connection.
	Unlike a Socket, the connection can be opened and closed multiple times.
*/
//=============================================================================

#include "TcpNetworkClient.h"
#include "InetAddress.h"

#include "QcCore/base/Tracer.h"
#include "QcCore/io/BufferedInputStream.h"
#include "QcCore/io/BufferedOutputStream.h"

QC_NET_NAMESPACE_BEGIN

using io::BufferedInputStream;
using io::BufferedOutputStream;

//==============================================================================
// TcpNetworkClient::connect
//
/**
   Connects this TcpNetworkClient to a server on the specified host.

   If the TcpNetworkClient is already connected, no action is taken.

   If @c timeoutMS is specified and is greater than zero, the connection attempt
   will time-out and a SocketTimeoutException will be thrown if the specified
   number of milliseconds elapses before the connection is fully established.

   If @c port is less than or equal to zero (the default), then a default
   port number will be obtained by calling getDefaultPort().

   @param host the host name of the server
   @param port the port number of the server
   @param timeoutMS the time-out value in milliseconds.  A value of zero indicates
          that the client will wait forever, or until the underlying
          operating system decides that the connection cannot be established.
   @throws IOException if an error occurs trying to connect to the server
   @throws ProtocolException if an invalid response is received from the server
   @throws UnknownHostException if an error occurs trying to resolve the host name
   @throws SocketTimeoutException if the specified timeout value expires while
           attempting to connect to the server
   @sa disconnect()
*/
//==============================================================================
void TcpNetworkClient::connect(const String& host, int port, size_t timeoutMS)
{
	if(isConnected())
		return;

	if(port <= 0)
	{
		port = getDefaultPort();
	}

	if(Tracer::IsEnabled())
	{
		String traceMsg = QC_T("Connecting to TCP Server: ");
		traceMsg += host;
		Tracer::Trace(Tracer::Net, Tracer::High, traceMsg);
	}

	m_rpSocket = createConnection(host, port, timeoutMS);

	postConnect(host, port, timeoutMS);

	Tracer::Trace(Tracer::Net, Tracer::High, QC_T("TCP connection established"));
}

//==============================================================================
// TcpNetworkClient::disconnect
//
/**
   Disconnects from the server.  It is acceptable to re-connect to another
   server once disconnect() has been called.

   Further calls to disconnect() have no effect.
*/
//==============================================================================
void TcpNetworkClient::disconnect()
{
	if(m_rpSocket)
	{
		if(Tracer::IsEnabled())
		{
			Tracer::Trace(Tracer::Net, Tracer::High, QC_T("Disconnecting from TCP Server"));
		}

		if(m_rpOutputStream)
			m_rpOutputStream->flush();

		m_rpSocket->close();
	}

	m_rpInputStream.release();
	m_rpOutputStream.release();
	m_rpSocket.release();
}

//==============================================================================
// TcpNetworkClient::isConnected
//
/**
   Tests if this TcpNetworkClient is currently connected to a server.

   @returns true if connected; false otherwise
*/
//==============================================================================
bool TcpNetworkClient::isConnected() const
{
	return (!m_rpSocket.isNull());
}

//==============================================================================
// TcpNetworkClient::createConnection
//
/**
   Protected virtual method to create a Socket connection
   to the TCP/IP network server.

   The base class implementation creates a standard socket and attempts
   to make a TCP/IP connection to the specified server.

   @param host the host name of the server
   @param port the port number of the server
   @param timeoutMS a time out value in milliseconds
   @throws IOException if an error occurs trying to connect to the server
   @throws UnknownHostException if an error occurs trying to resolve the host name
   @throws SocketTimeoutException if the specified timeout value expires while
           attempting to connect to the server
*/
//==============================================================================
AutoPtr<Socket> TcpNetworkClient::createConnection(const String& host, int port, size_t timeoutMS)
{
	QC_DBG_ASSERT(port > 0);

	AutoPtr<Socket> rpSocket = new Socket();
	
	rpSocket->connect(InetAddress::GetByName(host).get(), port, timeoutMS);
	return rpSocket;
}

//==============================================================================
// TcpNetworkClient::getInputStream
//
/**
   Returns an InputStream for reading from the server.

   @returns an InputStream for reading from the server or null if this
            TcpNetworkClient is not currently connected.
*/
//==============================================================================
AutoPtr<InputStream> TcpNetworkClient::getInputStream() const
{
	if(!m_rpInputStream)
	{
		if(m_rpSocket)
		{
			const_cast<TcpNetworkClient*>(this)->m_rpInputStream =
				new BufferedInputStream(m_rpSocket->getInputStream().get());
		}
	}
	return m_rpInputStream;
}

//==============================================================================
// TcpNetworkClient::getOutputStream
//
/**
   Returns an OutputStream for writing to the server.

   @returns an OutputStream for writing to the server or null if this
            TcpNetworkClient is not currently connected.
*/
//==============================================================================
AutoPtr<OutputStream> TcpNetworkClient::getOutputStream() const
{
	if(!m_rpOutputStream)
	{
		if(m_rpSocket)
		{
			const_cast<TcpNetworkClient*>(this)->m_rpOutputStream = 
				new BufferedOutputStream(m_rpSocket->getOutputStream().get());
		}
	}
	return m_rpOutputStream;
}

//==============================================================================
// TcpNetworkClient::getServerAddress
//
/**
   Returns the IP Address of the remote server to which this TcpNetworkClient
   is connected.

   @returns the InetAddress of the remote host or null if this
            TcpNetworkClient is not currently connected.
*/
//==============================================================================
AutoPtr<InetAddress> TcpNetworkClient::getServerAddress() const
{
	if(m_rpSocket)
	{
		return m_rpSocket->getInetAddress();
	}
	else
	{
		return 0;
	}
}

//==============================================================================
// TcpNetworkClient::getServerPort
//
/**
   Returns the port number of the remote server to which this TcpNetworkClient
   is connected.

   @returns the port number or -1 if this TcpNetworkClient is not
            currently connected.
*/
//==============================================================================
int TcpNetworkClient::getServerPort() const
{
	if(m_rpSocket)
	{
		return m_rpSocket->getPort();
	}
	else
	{
		return -1;
	}
}

//==============================================================================
// TcpNetworkClient::getLocalAddress
//
/**
   Returns the local IP Address that the connected socket is bound to.

   @returns the InetAddress of the local host or null if this
            TcpNetworkClient is not currently connected.
*/
//==============================================================================
AutoPtr<InetAddress> TcpNetworkClient::getLocalAddress() const
{
	if(m_rpSocket)
	{
		return m_rpSocket->getLocalAddress();
	}
	else
	{
		return 0;
	}
}

//==============================================================================
// TcpNetworkClient::postConnect
//
/**
   Protected function called when a socket connection has been established with 
   the TCP/IP network server.
   
   The base class implementation does nothing.  @a Derived classes may override
   this to perform custom connection processing.

   @param server the host name 
   @param port the port number 
   @param timeoutMS the timeout value passed to the connect() call
*/
//==============================================================================
void TcpNetworkClient::postConnect(const String& /*server*/, int /*port*/, size_t /*timeoutMS*/)
{
	// do nothing
}

//==============================================================================
// TcpNetworkClient::preDisconnect
//
/**
   Protected function called when a socket connection is about to be closed
   from the client side.
   
   The base class implementation does nothing.
*/
//==============================================================================
void TcpNetworkClient::preDisconnect()
{
}

#ifdef QC_DOCUMENTATION_ONLY
//=============================================================================
//
// Documentation for pure virtual methods follows:
//
//=============================================================================

//==============================================================================
// TcpNetworkClient::getDefaultPort
//
/**
   Virtual member function overridden in derived classes to return a 
   default port number for the network service.
   This is called by connect() when a valid port number has not been provided.
*/
//==============================================================================
int TcpNetworkClient::getDefaultPort() const=0;

#endif  //QC_DOCUMENTATION_ONLY

QC_NET_NAMESPACE_END

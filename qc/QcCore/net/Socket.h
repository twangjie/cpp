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
// Class: Socket
//
// This class should not need much introduction.  It provides a portable wrapper
// around BSD-Style sockets under Linux/Unix and WinSock sockets under Windows.
//
// Sockets provide a means of creating a data connection to another host.
//
//==============================================================================

#ifndef QC_NET_Socket_h
#define QC_NET_Socket_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "SocketImpl.h"

QC_NET_NAMESPACE_BEGIN

class InetAddress;
class SocketImplFactory;

class QC_NET_PKG Socket : public virtual QCObject 
{
	friend class ServerSocket;

	// Construction:

protected:
	// Creates an unconnected Socket with a user-specified SocketImpl. 
	Socket(SocketImpl* pImpl);

public:
	Socket();
	Socket(InetAddress* pAddress, int port);
	Socket(InetAddress* pAddress, int port, InetAddress* pLocalAddr, int localPort);
	Socket(const String& host, int port);
	Socket(const String& host, int port, InetAddress* pLocalAddr, int localPort);

	virtual void connect(InetAddress* pAddress, int port, size_t timeoutMS);
	virtual void connect(InetAddress* pAddress, int port);
	virtual void connect(const String& host, int port);
	virtual void close();
	virtual bool getAutoClose() const;
	virtual AutoPtr<InetAddress> getInetAddress() const;
	virtual AutoPtr<InputStream> getInputStream() const;
	virtual bool getKeepAlive() const;
	virtual AutoPtr<InetAddress> getLocalAddress() const;
	virtual int getLocalPort() const;
	virtual AutoPtr<OutputStream> getOutputStream() const;
	virtual int getPort() const;
	virtual int getReceiveBufferSize() const;
	virtual int getSendBufferSize() const;
	virtual int getSoLinger() const;
	virtual size_t getSoTimeout() const;
	virtual bool getTcpNoDelay() const;
	virtual bool isClosed();
	virtual bool isConnected();
	virtual void setAutoClose(bool bEnable);
	virtual void setKeepAlive(bool bEnable);
	virtual void setReceiveBufferSize(size_t size);
	virtual void setSendBufferSize(size_t size);
	virtual void setSoLinger(bool bEnable, size_t linger);
	virtual void setSoTimeout(size_t timeoutMS);
	virtual void setTcpNoDelay(bool bEnable);
	virtual void shutdownInput();
	virtual void shutdownOutput();
	virtual String toString() const;

public: // static methods
	static void SetSocketImplFactory(SocketImplFactory* pFac);
	static AutoPtr<SocketImplFactory> GetSocketImplFactory();

private:
	void createSocketImpl();
	AutoPtr<SocketImpl> getSocketImpl() const;

private:
	AutoPtr<SocketImpl> m_rpSocketImpl;

	static SocketImplFactory* QC_MT_VOLATILE s_pSocketImplFactory;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_Socket_h

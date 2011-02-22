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

#ifndef QC_NET_ServerSocket_h
#define QC_NET_ServerSocket_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

QC_NET_NAMESPACE_BEGIN

class InetAddress;
class Socket;
class SocketImpl;
class SocketImplFactory;

class QC_NET_PKG ServerSocket : public virtual ManagedObject 
{
public:

	ServerSocket();
	ServerSocket(int port);
	ServerSocket(int port, int backlog);
	ServerSocket(int port, int backlog, InetAddress* pBindAddr);

	virtual AutoPtr<Socket> accept();
	virtual void bind(int port, int backlog, InetAddress* pBindAddr);
	virtual void bind(int port, int backlog);
	virtual void bind(int port);
	virtual void close();
	virtual AutoPtr<InetAddress> getInetAddress() const;
	virtual int getLocalPort() const;
	virtual size_t getReceiveBufferSize() const;
	virtual bool getReuseAddress() const;
	virtual size_t getSoTimeout() const;
	virtual bool isBound() const;
	virtual void setReceiveBufferSize(size_t size);
	virtual void setReuseAddress(bool bEnable);
	virtual void setSoTimeout(size_t timeoutMS);
	virtual String toString() const;

public: // static functions 
	static void SetSocketImplFactory(SocketImplFactory* pFac);
	static AutoPtr<SocketImplFactory> GetSocketImplFactory();

protected:
	void implAccept(Socket* pSocket);

private:
	void init();

private:
	AutoPtr<SocketImpl> m_rpSocketImpl;
	bool m_bIsBound;

	static SocketImplFactory* QC_MT_VOLATILE s_pSocketImplFactory;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_Socket_h

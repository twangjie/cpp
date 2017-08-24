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
// Class: PlainSocketImpl
//
// Class PlainSocketImpl is a concrete implementation of the abstract class
// SocketImpl.  As its name implies, this class implements a plain vanilla
// socket as provided by the underlying operating system.
//
// Notes
// -----
// We hold members for the AutoPtrs to the input/output streams.  This is 
// so that we may return the same stream whenever we are asked for it.
//
// The socket descriptor is held in a separate object to avoid a circular
// reference.
//==============================================================================

#ifndef QC_NET_PlainSocketImpl_h
#define QC_NET_PlainSocketImpl_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "SocketImpl.h"
#include "InetAddress.h"
#include "SocketDescriptor.h"

QC_NET_NAMESPACE_BEGIN

class QC_NET_PKG PlainSocketImpl : public SocketImpl
{
public:
	PlainSocketImpl();
	~PlainSocketImpl();

protected:
	virtual void accept(SocketImpl* pSocket);
	virtual size_t available() const;
	virtual void bind(InetAddress* pAddress, int port);

	virtual void close();
	virtual void connect(InetAddress* pAddress, int port);
	virtual void connect(const String& host, int port);
	virtual void connect(InetAddress* pAddress, int port, size_t timeoutMS);
	virtual void create(bool bStream);

	virtual bool getAutoClose() const;
	virtual AutoPtr<InetAddress> getInetAddress() const;
	virtual AutoPtr<InetAddress> getLocalAddress() const;
	virtual AutoPtr<InetAddress> getRemoteAddress() const;

	virtual AutoPtr<InputStream> getInputStream() const;
	virtual int getLocalPort() const;
	virtual AutoPtr<OutputStream> getOutputStream() const;
	virtual int getPort() const;
	virtual AutoPtr<SocketDescriptor> getSocketDescriptor() const;

	virtual void listen(size_t backlog);
	virtual String toString() const;

	// Socket Option methods
	virtual int getIntOption(int level, int option) const;
	virtual void getVoidOption(int level, int option, void* pOut, size_t* pLen) const;
	virtual void setIntOption(int level, int option, int value);
	virtual void setVoidOption(int level, int option, void* pValue, size_t valLen);
	virtual size_t getTimeout() const;
	virtual void setTimeout(size_t timeoutMS);

	virtual void setAutoClose(bool bEnable);
	virtual void setSocketDescriptor(SocketDescriptor* pSocketDescriptor);
	virtual void setInetAddress(InetAddress* pAddress);
	virtual void setLocalPort(int localPort);
	virtual void setPort(int port);

	virtual void shutdownInput();
	virtual void shutdownOutput();
	virtual bool isClosed() const;
	virtual bool isConnected() const;
	virtual bool isBound() const;

private:
	void connectToAddress(InetAddress* pAddress, int port, size_t timeoutMS);
	void testSocketIsValid(bool bTestConnected) const;
	void setBlocking(bool bBlocking);

private:
	AutoPtr<SocketDescriptor>   m_rpSocketDescriptor;
	mutable AutoPtr<InputStream>  m_rpInputStream;
	mutable AutoPtr<OutputStream> m_rpOutputStream;
	mutable AutoPtr<InetAddress>  m_rpLocalAddr;
	AutoPtr<InetAddress>        m_rpRemoteAddr;
	int                        m_localPort;
	int                        m_remotePort;
	bool                       m_bBlocking;
	bool                       m_bPassive;
	size_t                     m_nTimeoutMS;
	int                        m_sockType;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_PlainSocketImpl_h

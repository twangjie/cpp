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
// Class: PlainDatagramSocketImpl
//
// Class PlainDatagramSocketImpl is a concrete implementation of the abstract class
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

#ifndef QC_NET_PlainDatagramSocketImpl_h
#define QC_NET_PlainDatagramSocketImpl_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "DatagramSocketImpl.h"
#include "InetAddress.h"
#include "SocketDescriptor.h"

QC_NET_NAMESPACE_BEGIN

class QC_NET_PKG PlainDatagramSocketImpl : public DatagramSocketImpl
{
public:
	PlainDatagramSocketImpl();
	~PlainDatagramSocketImpl();

protected:
	virtual void bind(InetAddress* pAddress, int port);

	virtual void close();
	virtual void connect(InetAddress* pAddress, int port);
	virtual void create();
	virtual void disconnect();

	virtual AutoPtr<InetAddress> getInetAddress() const;
	virtual AutoPtr<InetAddress> getLocalAddress() const;

	virtual int getLocalPort() const;
	virtual int getPort() const;
	virtual AutoPtr<SocketDescriptor> getSocketDescriptor() const;

	virtual String toString() const;

	virtual void send(const DatagramPacket& p);
	virtual void receive(DatagramPacket& p);

	// Socket Option methods
	virtual int getIntOption(int level, int option) const;
	virtual void getVoidOption(int level, int option, void* pOut, size_t* pLen) const;
	virtual void setIntOption(int level, int option, int value);
	virtual void setVoidOption(int level, int option, void* pValue, size_t valLen);
	virtual size_t getTimeout() const;
	virtual void setTimeout(size_t timeoutMS);

	virtual bool isClosed() const;
	virtual bool isConnected() const;
	virtual bool isBound() const;

private:
	void connectToAddress(InetAddress* pAddress, int port);
	void testSocketIsValid() const;

private:
	AutoPtr<SocketDescriptor>   m_rpSocketDescriptor;
	mutable AutoPtr<InetAddress>  m_rpLocalAddr;
	AutoPtr<InetAddress>        m_rpRemoteAddr;
	int                        m_localPort;
	int                        m_remotePort;
	size_t                     m_nTimeoutMS;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_PlainDatagramSocketImpl_h

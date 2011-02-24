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
// Class: DatagramSocket
//
// This class should not need much introduction.  It provides a portable wrapper
// around BSD-Style datagram sockets under Linux/Unix and WinSock 
// datagram sockets under Windows.
//
//==============================================================================

#ifndef QC_NET_DatagramSocket_h
#define QC_NET_DatagramSocket_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "DatagramSocketImpl.h"

QC_NET_NAMESPACE_BEGIN

class InetAddress;
class DatagramSocketImpl;
class DatagramSocketImplFactory;

class QC_NET_PKG DatagramSocket : public virtual QCObject 
{
public:
	DatagramSocket();
	DatagramSocket(int port);
	DatagramSocket(int port, InetAddress* pLocalAddr);

	virtual void bind(int port, InetAddress* pLocalAddr);
	virtual void close();
	virtual void connect(InetAddress* pAddress, int port);
	virtual void disconnect();
	virtual bool getBroadcast() const;
	virtual AutoPtr<InetAddress> getInetAddress() const;
	virtual AutoPtr<InetAddress> getLocalAddress() const;
	virtual int getLocalPort() const;
	virtual int getPort() const;
	virtual int getReceiveBufferSize() const;
	virtual bool getReuseAddress() const;
	virtual int getSendBufferSize() const;
	virtual size_t getSoTimeout() const;
	virtual bool isBound() const;
	virtual bool isClosed() const;
	virtual bool isConnected() const;
	virtual void setBroadcast(bool bBroadcast);
	virtual void setReceiveBufferSize(size_t size);
	virtual void setReuseAddress(bool bEnable);
	virtual void setSendBufferSize(size_t size);
	virtual void setSoTimeout(size_t timeoutMS);
	virtual String toString() const;

	virtual void send(const DatagramPacket& p);
	virtual void receive(DatagramPacket& p);

public: // static methods
	static void SetDatagramSocketImplFactory(DatagramSocketImplFactory* pFac);
	static AutoPtr<DatagramSocketImplFactory> GetDatagramSocketImplFactory();

protected:
	void createDatagramSocketImpl();
	AutoPtr<DatagramSocketImpl> getDatagramSocketImpl() const;

private:
	AutoPtr<DatagramSocketImpl> m_rpDatagramSocketImpl;

	static DatagramSocketImplFactory* QC_MT_VOLATILE s_pDatagramSocketImplFactory;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_DatagramSocket_h

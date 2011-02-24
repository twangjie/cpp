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

#ifndef QC_NET_SocketImpl_h
#define QC_NET_SocketImpl_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

QC_NET_NAMESPACE_BEGIN

class InetAddress;
class SocketDescriptor;

class QC_NET_PKG SocketImpl : public virtual QCObject
{
	friend class Socket;
	friend class ServerSocket;

protected:
	virtual void accept(SocketImpl* pSocket)=0;
	virtual size_t available() const=0;
	virtual void bind(InetAddress* pAddress, int port)=0;
	virtual void close()=0;
	virtual void connect(InetAddress* pAddress, int port)=0;
	virtual void connect(const String& host, int port)=0;
	virtual void connect(InetAddress* pAddress, int port, size_t timeoutMS)=0;
	virtual void create(bool bStream)=0;

	virtual AutoPtr<InetAddress> getInetAddress() const=0;
	virtual AutoPtr<InetAddress> getLocalAddress() const=0;
	virtual AutoPtr<InetAddress> getRemoteAddress() const=0;

	virtual AutoPtr<InputStream> getInputStream() const=0;
	virtual int getLocalPort() const=0;
	virtual AutoPtr<OutputStream> getOutputStream() const=0;
	virtual int getPort() const=0;
	virtual AutoPtr<SocketDescriptor> getSocketDescriptor() const=0;

	virtual void listen(size_t backlog)=0;
	virtual String toString() const=0;

	// Socket Option methods
	virtual bool getAutoClose() const=0;
	virtual int getIntOption(int level, int option) const=0;
	virtual void getVoidOption(int level, int option, void* pOut, size_t* pLen) const=0;
	virtual void setIntOption(int level, int option, int value)=0;
	virtual void setVoidOption(int level, int option, void* pValue, size_t valLen)=0;
	virtual size_t getTimeout() const=0;
	virtual void setTimeout(size_t timeoutMS)=0;
	
	virtual void setAutoClose(bool bEnable)=0;
	virtual void setSocketDescriptor(SocketDescriptor* pSocketDescriptor)=0;
	virtual void setInetAddress(InetAddress* pAddress)=0;
	virtual void setLocalPort(int localPort)=0;
	virtual void setPort(int port)=0;

	virtual void shutdownInput()=0;
	virtual void shutdownOutput()=0;

	virtual bool isClosed() const=0;
	virtual bool isConnected() const=0;
	virtual bool isBound() const=0;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_SocketImpl_h

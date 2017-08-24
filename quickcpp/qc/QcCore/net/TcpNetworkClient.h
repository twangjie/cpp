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

#ifndef QC_NET_TcpNetworkClient_h
#define QC_NET_TcpNetworkClient_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "Socket.h"
#include "InetAddress.h"

QC_NET_NAMESPACE_BEGIN

class QC_NET_PKG TcpNetworkClient : public virtual QCObject
{
public:

	virtual void connect(const String& host, int port=-1, size_t timeoutMS=0);
	virtual void disconnect();
	virtual bool isConnected() const;
	virtual AutoPtr<InetAddress> getServerAddress() const;
	virtual AutoPtr<InetAddress> getLocalAddress() const;
	virtual AutoPtr<InputStream> getInputStream() const;
	virtual AutoPtr<OutputStream> getOutputStream() const;
	virtual int getServerPort() const;

protected:

	virtual int getDefaultPort() const=0;
	virtual void postConnect(const String& server, int port, size_t timeoutMS);
	virtual void preDisconnect();
	virtual AutoPtr<Socket> createConnection(const String& host, int port, size_t timeoutMS);
	
private:
	AutoPtr<Socket> m_rpSocket;
	AutoPtr<InputStream> m_rpInputStream;
	AutoPtr<OutputStream> m_rpOutputStream;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_TcpNetworkClient_h

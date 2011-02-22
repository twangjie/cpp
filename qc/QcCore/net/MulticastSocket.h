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
// Class: MulticastSocket
//
//==============================================================================

#ifndef QC_NET_MulticastSocket_h
#define QC_NET_MulticastSocket_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "DatagramSocket.h"

QC_NET_NAMESPACE_BEGIN

class QC_NET_PKG MulticastSocket : public DatagramSocket
{
public:
	MulticastSocket();
	MulticastSocket(int port);

	virtual AutoPtr<InetAddress> getNetworkInterface() const;
	virtual int getTimeToLive() const;
	virtual void joinGroup(const InetAddress* pMulticastAddr);
	virtual void joinGroup(const InetAddress* pMulticastAddr, const InetAddress* pInf);
	virtual void leaveGroup(const InetAddress* pMulticastAddr);
	virtual void leaveGroup(const InetAddress* pMulticastAddr, const InetAddress* pInf);
	virtual void setNetworkInterface(const InetAddress* pInf);
	virtual void setTimeToLive(int ttl);
};

QC_NET_NAMESPACE_END

#endif //QC_NET_MulticastSocket_h

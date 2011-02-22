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
// class DatagramPacket
//
// A simple class used as the holder of information sent and received via
// a DatagramSocket.
//==============================================================================

#ifndef QC_NET_DatagramPacket_h
#define QC_NET_DatagramPacket_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

QC_NET_NAMESPACE_BEGIN

class InetAddress;

class QC_NET_PKG DatagramPacket
{

public:
	explicit DatagramPacket(size_t bufSize);
	DatagramPacket(Byte* pBuffer, size_t bufSize, bool bAssignBuffer=false);
	DatagramPacket(Byte* pBuffer, size_t bufSize, InetAddress* pAddress, int port=-1, bool bAssignBuffer=false);
	~DatagramPacket();

	Byte* getData() const;
	AutoPtr<InetAddress> getAddress() const;
	size_t getLength() const;
	size_t getBufferSize() const;
	int getPort() const;

	void setData(size_t bufSize);
	void setData(Byte* pBuffer, size_t bufSize, bool bAssignBuffer=false);
	void setAddress(InetAddress* pAddress);
	void setLength(size_t length);
	void setPort(int port);

private: // not implemented
	DatagramPacket(const DatagramPacket& rhs);            // cannot be copied
	DatagramPacket& operator=(const DatagramPacket& rhs); // nor assigned

private:
	int                 m_port;
	size_t              m_bufSize;
	size_t              m_dataLength;
	bool                m_bBufferOwner;
	Byte*               m_pBuffer;
	AutoPtr<InetAddress> m_rpAddress;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_DatagramPacket_h

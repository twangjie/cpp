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
// Class: InetAddress
// 
// Overview
// --------
// based on the java.net.InetAddress class.  Wraps an IP address and provides
// convenience functions for interogating the system's DNS query mechanism.
//
// Note: Currently we only wrap an IPv4 IP address but it should be 
// quite straight forward to enhance this for IPv6
//==============================================================================

#ifndef QC_NET_InetAddress_h
#define QC_NET_InetAddress_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

struct in_addr; // forward declaration

QC_NET_NAMESPACE_BEGIN

class QC_NET_PKG InetAddress : public virtual ManagedObject
{
public:
	~InetAddress();
	InetAddress(const InetAddress& rhs);
	InetAddress& operator=(const InetAddress& rhs);

	virtual String getHostAddress() const;
	virtual String getHostName() const;
	virtual String toString() const;

	virtual const Byte* getAddress() const;
	virtual size_t getAddressLength() const;

	virtual bool equals(const InetAddress& rhs) const;

	bool operator==(const InetAddress& rhs) const;
	bool operator!=(const InetAddress& rhs) const;

public: // Static functions

	static AutoPtr<InetAddress> GetByName(const String& host);
	static AutoPtr<InetAddress> GetLocalHost();
	static AutoPtr<InetAddress> GetAnyHost();
	static AutoPtr<InetAddress> GetLocalBroadcast();
	static AutoPtr<InetAddress> FromNetworkAddress(const struct sockaddr* pAddr, size_t addrlen);

protected:
	InetAddress(); // default constructor protected, copying is okay though

private:
	struct in_addr* m_pAddr;
	mutable String m_hostName;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_InetAddress_h

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
// Class: SocketDescriptor
//
// A reference-counted object containing an OS Socket Descriptor.
//
// The purpose of this class is to mirror the action an OS takes in
// reference-counting file descriptors - but without the downside
// of using a scarse OS resource.
//
// When the SocketDescriptor reference count goes to zero, and this
// class is destroyed, the contained OS socket descriptor is closed.
//==============================================================================

#ifndef QC_NET_SocketDescriptor_h
#define QC_NET_SocketDescriptor_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "QcCore/base/SynchronizedObject.h"
#include "QcCore/io/ResourceDescriptor.h"

QC_NET_NAMESPACE_BEGIN

using io::ResourceDescriptor;

class QC_NET_PKG SocketDescriptor : public ResourceDescriptor, public SynchronizedObject
{
public:
	typedef QC_SOCKET_FD OSSocketDescriptorType;

	enum Flags {ShutdownInput=1,
	            ShutdownOutput=2,
				HasOutputStream=4,
				DescriptorClosed=8,
				AutoCloseEnabled=16};

	SocketDescriptor(OSSocketDescriptorType fd);
	~SocketDescriptor();

	void close();
	virtual String toString() const;
	OSSocketDescriptorType getFD() const;

	int getSocketFlags() const;
	void setSocketFlags(int flags);
	int modifySocketFlags(int flagsToSet, int flagsToUnset);

private:
	OSSocketDescriptorType m_fd;
	int                    m_flags;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_SocketDescriptor_h

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

#ifndef QC_NET_SocketInputStream_h
#define QC_NET_SocketInputStream_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

QC_NET_NAMESPACE_BEGIN

class SocketDescriptor;

class QC_NET_PKG SocketInputStream : public InputStream
{
public:
	SocketInputStream(SocketDescriptor* pDescriptor);
	~SocketInputStream();

	virtual size_t available();
	virtual void close();

#ifdef QC_USING_DECL_BROKEN
	virtual int read() {return InputStream::read();}
#else
	using InputStream::read; 	// unhide inherited read methods
#endif

	virtual long read(Byte* pBuffer, size_t bufLen);

public:
	size_t getTimeout() const;
	void setTimeout(size_t timeoutMS);

private:
	AutoPtr<SocketDescriptor> m_rpSocketDescriptor;
	size_t m_timeoutMS;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_SocketInputStream_h

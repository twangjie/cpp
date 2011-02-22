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
// Class: NvtAsciiInputStream
// 
//=============================================================================

#ifndef QC_NET_NvtAsciiInputStream_h
#define QC_NET_NvtAsciiInputStream_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "QcCore/io/FilterInputStream.h"

QC_NET_NAMESPACE_BEGIN

using io::FilterInputStream;

class QC_NET_PKG NvtAsciiInputStream : public FilterInputStream
{
public:
	NvtAsciiInputStream(InputStream* pInputStream);

#ifdef QC_USING_DECL_BROKEN
	virtual int read() {return FilterInputStream::read();}
#else
	using FilterInputStream::read; 	// unhide inherited read methods
#endif

	virtual long read(Byte* pBuffer, size_t bufLen);
};

QC_NET_NAMESPACE_END

#endif //QC_NET_NvtAsciiInputStream_h


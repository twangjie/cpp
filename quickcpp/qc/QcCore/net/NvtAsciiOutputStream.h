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
// Class: NvtAsciiOutputStream
// 
//=============================================================================

#ifndef QC_NET_NvtAsciiOutputStream_h
#define QC_NET_NvtAsciiOutputStream_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "QcCore/io/FilterOutputStream.h"

QC_NET_NAMESPACE_BEGIN

using io::FilterOutputStream;

class QC_NET_PKG NvtAsciiOutputStream : public FilterOutputStream
{
public:
	NvtAsciiOutputStream(OutputStream* pOutputStream);

#ifdef QC_USING_DECL_BROKEN
	virtual void write(Byte x) {FilterOutputStream::write(x);}
#else
	using FilterOutputStream::write; 	// unhide inherited write method
#endif

	virtual void write(const Byte* pBuffer, size_t bufLen);

private:
	bool m_bCRSeen;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_NvtAsciiOutputStream_h


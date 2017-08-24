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
// Class: StringReader
// 
//=============================================================================

#ifndef QC_IO_StringReader_h
#define QC_IO_StringReader_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "Reader.h"

QC_IO_NAMESPACE_BEGIN

class InputStream;

class QC_IO_PKG StringReader : public Reader
{
public:

	StringReader(const String& string);
	StringReader(const CharType* pStr, size_t length);

	virtual void close();
	virtual void mark(size_t readLimit);
	virtual bool markSupported() const;

#ifdef QC_USING_DECL_BROKEN
	virtual IntType read() {return Reader::read();}
#else
	using Reader::read; 	// unhide inherited read methods
#endif

	virtual long read(CharType* pBuffer, size_t bufLen);
	virtual long readAtomic(CharType* pBuffer, size_t bufLen);
	virtual Character readAtomic();
	virtual void reset();

private:
	String m_string;
	size_t m_pos;
	int m_markPos;
	bool m_bClosed;
};

QC_IO_NAMESPACE_END

#endif //QC_IO_StringReader_h


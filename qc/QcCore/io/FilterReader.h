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
// Class: FilterReader
// 
// Overview
// --------
// Based on the java.io.FilterReader class.  Provides a base class
// for reading filtered character streams. 
//
// All methods in this implementation delegate directly to the contained
// Reader.  Subclasses should override methods as appropriate.
//
//=============================================================================

#ifndef QC_IO_FilterReader_h
#define QC_IO_FilterReader_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "Reader.h"

QC_IO_NAMESPACE_BEGIN

class QC_IO_PKG FilterReader : public Reader
{
public:

	FilterReader(Reader* pReader);

	virtual void close();
	virtual void mark(size_t readLimit);
	virtual bool markSupported() const;
	virtual IntType read();
	virtual long read(CharType* pBuffer, size_t bufLen);
	virtual long readAtomic(CharType* pBuffer, size_t bufLen);
	virtual Character readAtomic();
	virtual void reset();
	virtual size_t skip(size_t n);
	virtual size_t skipAtomic(size_t n);

protected:
	AutoPtr<Reader> getReader() const;

private:
	AutoPtr<Reader> m_rpReader;
};

QC_IO_NAMESPACE_END

#endif //QC_IO_FilterReader_h


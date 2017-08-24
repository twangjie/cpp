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

#ifndef QC_IO_ByteArrayInputStream_h
#define QC_IO_ByteArrayInputStream_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "InputStream.h"

#include "QcCore/base/ArrayAutoPtr.h"

QC_IO_NAMESPACE_BEGIN

class QC_IO_PKG ByteArrayInputStream : public InputStream
{
public:
	ByteArrayInputStream(const Byte* pData, size_t dataLen);

	virtual void mark(size_t readLimit);
	virtual bool markSupported() const;
	virtual void reset();
	virtual size_t available();
	virtual void close();

#ifdef QC_USING_DECL_BROKEN
	virtual int read() {return InputStream::read();}
#else
	using InputStream::read; 	// unhide inherited read methods
#endif

	virtual long read(Byte* pBuffer, size_t bufLen);

	size_t m_pos;
	ArrayAutoPtr<Byte> m_apBuffer;
	size_t m_bufSize;

private:
	ByteArrayInputStream(const ByteArrayInputStream& rhs);            // cannot be copied
	ByteArrayInputStream& operator=(const ByteArrayInputStream& rhs); // nor assigned

	int m_markPos;
	bool m_bClosed;
};

QC_IO_NAMESPACE_END

#endif //QC_IO_ByteArrayInputStream_h


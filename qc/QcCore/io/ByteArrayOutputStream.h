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
// Class: ByteArrayOutputStream
// 
//==============================================================================

#ifndef QC_IO_ByteArrayOutputStream_h
#define QC_IO_ByteArrayOutputStream_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "OutputStream.h"

#include "QcCore/base/AutoBuffer.h"

QC_IO_NAMESPACE_BEGIN

class QC_IO_PKG ByteArrayOutputStream : public OutputStream
{
public:
	ByteArrayOutputStream();
	ByteArrayOutputStream(size_t size);

	virtual void close();

#ifdef QC_USING_DECL_BROKEN
	virtual void write(Byte x) {OutputStream::write(x);}
#else
	using OutputStream::write; 	// unhide inherited write method
#endif

	virtual void write(const Byte* pBuffer, size_t bufLen);

	void reset();
	void writeTo(OutputStream* pOut) const;
	size_t size() const;
	String toString(const String& encoding) const;
	String toString(CodeConverter* pDecoder) const;
	String toString() const;
	const Byte* data() const;

private:
	ByteArrayOutputStream(const ByteArrayOutputStream& rhs);            // cannot be copied
	ByteArrayOutputStream& operator=(const ByteArrayOutputStream& rhs); // nor assigned

	AutoBuffer<Byte> m_buffer;
	bool m_bClosed;
};

QC_IO_NAMESPACE_END

#endif //QC_IO_ByteArrayOutputStream_h

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
// Class: BufferedOutputStream
// 
//==============================================================================

#ifndef QC_IO_BufferedOutputStream_h
#define QC_IO_BufferedOutputStream_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "OutputStream.h"

QC_IO_NAMESPACE_BEGIN

class QC_IO_PKG BufferedOutputStream : public OutputStream
{
public:

	BufferedOutputStream(OutputStream* pOutputStream);
	BufferedOutputStream(OutputStream* pOutputStream, size_t bufSize);
	virtual ~BufferedOutputStream();

	virtual void close();
	virtual void flush();
	virtual void flushBuffers();

#ifdef QC_USING_DECL_BROKEN
	virtual void write(Byte x) {OutputStream::write(x);}
#else
	using OutputStream::write; 	// unhide inherited write method
#endif

	virtual void write(const Byte* pBuffer, size_t bufLen);

private:
	BufferedOutputStream(const BufferedOutputStream& rhs);            // cannot be copied
	BufferedOutputStream& operator=(const BufferedOutputStream& rhs); // nor assigned

	void init(size_t bufferSize);
	void freeBuffers();
	void writeBuffer();

private:
	Byte*  m_pBuffer;
	size_t m_bufferSize;
	size_t m_used;
	AutoPtr<OutputStream> m_rpOutputStream;
};

QC_IO_NAMESPACE_END

#endif //QC_IO_BufferedOutputStream_h

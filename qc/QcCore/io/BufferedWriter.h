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
// Class: BufferedWriter
// 
//==============================================================================

#ifndef QC_IO_BufferedWriter_h
#define QC_IO_BufferedWriter_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "Writer.h"

QC_IO_NAMESPACE_BEGIN

class QC_IO_PKG BufferedWriter : public Writer
{
public:

	BufferedWriter(Writer* pWriter);
	BufferedWriter(Writer* pWriter, size_t bufSize);
	virtual ~BufferedWriter();

	virtual void close();
	virtual void flush();
	virtual void flushBuffers();

#ifdef QC_USING_DECL_BROKEN
	virtual void write(CharType c)           {Writer::write(c);}
	virtual void write(const Character& ch)  {Writer::write(ch);}
	virtual void write(const String& str)    {Writer::write(str);}
#else
	using Writer::write; 	// unhide inherited write methods
#endif

	virtual void write(const CharType* pStr, size_t len);

private:
	void flushBuffersImpl();
	void init(size_t bufferSize);

private:
	AutoPtr<Writer> m_rpWriter;
	CharType*  m_pBuffer;
	size_t m_bufferSize;
	size_t m_used;
};

QC_IO_NAMESPACE_END

#endif //QC_IO_BufferedWriter_h

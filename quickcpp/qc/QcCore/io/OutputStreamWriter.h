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

#ifndef QC_IO_OutputStreamWriter_h
#define QC_IO_OutputStreamWriter_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "Writer.h"

QC_IO_NAMESPACE_BEGIN

class OutputStream;
class CodeConverter;

class QC_IO_PKG OutputStreamWriter : public virtual Writer
{
public:
	OutputStreamWriter(OutputStream* pOutputStream);
	OutputStreamWriter(OutputStream* pOut, const String& encoding);
	OutputStreamWriter(OutputStream* pOut, const String& encoding, bool bStrict);
	OutputStreamWriter(OutputStream* pOut, CodeConverter* pEncoder);

	virtual ~OutputStreamWriter();

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

	virtual void write(const CharType* pBuf, size_t len);
	// End of Writer interface

	String getEncoding() const;
	AutoPtr<CodeConverter> getEncoder() const;

private:
	OutputStreamWriter(const OutputStreamWriter& rhs); // not implemented
	void operator=(const OutputStreamWriter& rhs);     // not implemented

	void init(const String& encoding, bool bStrictEncoding);
	void initEncoder(CodeConverter* pEncoder);
	void freeBuffers();
	bool doEncoding(const CharType* pBuffer, size_t bufLen);
	void writeByteBuffer();

private:
	AutoPtr<OutputStream> m_rpOutputStream;
	AutoPtr<CodeConverter> m_rpEncoder;
	Byte* m_pByteBuffer;
	size_t m_byteBufferSize;
	size_t m_byteBufferUsed;
	CharType* m_pCharSeqBuffer;
	size_t m_charSeqBufferUsed;
	bool m_bRequiresEncoding;
	bool m_bAtStart;
};

QC_IO_NAMESPACE_END

#endif //QC_IO_OutputStreamWriter_h

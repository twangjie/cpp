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
// Class: InputStreamReader
// 
//=============================================================================

#ifndef QC_IO_InputStreamReader_h
#define QC_IO_InputStreamReader_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "Reader.h"

QC_IO_NAMESPACE_BEGIN

class InputStream;

class QC_IO_PKG InputStreamReader : public Reader
{
public:

	InputStreamReader(InputStream* pInputStream);
	InputStreamReader(InputStream* pInputStream, const String& encoding);
	InputStreamReader(InputStream* pInputStream, const String& encoding, bool bStrict);
	InputStreamReader(InputStream* pInputStream, CodeConverter* pDecoder);

	virtual ~InputStreamReader();

	virtual void close();

#ifdef QC_USING_DECL_BROKEN
	virtual IntType read() {return Reader::read();}
#else
	using Reader::read; 	// unhide inherited read methods
#endif

	virtual long read(CharType* pBuffer, size_t bufLen);
	virtual long readAtomic(CharType* pBuffer, size_t bufLen);
	virtual Character readAtomic();

	String getEncoding() const;
	AutoPtr<CodeConverter> getDecoder() const;

public: // static methods
	static String SenseEncoding(InputStream* pInputStream, size_t& BOMSize);

private: // private helper methods
	InputStreamReader(const InputStreamReader& rhs); // not implemented
	void operator=(const InputStreamReader& rhs);    // not implemented

	void init(const String& encoding, bool bStrict);
	void initDecoder(CodeConverter* pDecoder);
	void freeBuffers();
	void fillByteBuffer();
	size_t readAndDecode(bool bBlock, CharType* pBuffer, size_t bufLen, bool bAtomicRead);
	bool readDirect(CharType* pBuffer, size_t bufLen, bool bAtomicRead, size_t& charsRead);
	void suspendDecodingOptimization(const Byte* pExtraBytes, size_t extraLen);

private:
	enum {CharSeqBufferSize = 8};
	AutoPtr<InputStream> m_rpInputStream;
	AutoPtr<CodeConverter> m_rpDecoder;
	Byte* m_pByteBuffer;
	Byte* m_pNextByteAvailable;
	Byte* m_pNextByteFree;
	size_t m_byteBufferSize;
	CharType m_charSeqBuffer[CharSeqBufferSize];
	CharType* m_pCharSeqNext;
	size_t m_charSeqLen;
	bool m_bRequiresDecoding;
	bool m_bAtEof;
};

QC_IO_NAMESPACE_END

#endif //QC_IO_InputStreamReader_h


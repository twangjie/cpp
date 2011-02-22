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
// Class: OutputStreamWriter
/**
	@class qc::io::OutputStreamWriter
	
	@brief An OutputStreamWriter uses an instance of the CodeConverter class
	to translate Unicode characters into sequences of bytes and writes
	these bytes to an underlying OutputStream.

    The encoding of the underlying byte stream may be specified by
	name or by directly providing an instance of a CodeConverter.  If no encoding is
	specified the system will use a default encoding.

    A CodeConverter can be configured to behave in one of two specified
	ways when it encounters a Unicode character that cannot be mapped into the
	target encoding.  By default,
	unmappable characters are silently dealt with by replacing the character
	with a default replacement character.  A stricter
	scheme is also available that treats unmappable characters as non-recoverable 
	errors and throws an UnmappableCharacterException.  Overloaded versions of the OutputStreamWriter
	constructors can be used to explicitly set the required policy.

    To improve efficiency, the OutputStreamWriter contains a byte buffer
	into which it encodes Unicode characters.  It is only when the buffer
	fills or one of the flush() methods are called that the bytes are actually
	written to the underlying byte stream. 

    See the description of InputStreamReader for a sample piece of
	code that uses an OutputStreamWriter.

    @mt
    As shown on the inheritance graph, OutputStreamWriter @a derives from 
	SynchronizedObject, which gives it the ability to protect its internal
	state from concurrent access from multiple threads.  All public methods 
	are synchronized for safe concurrent access.
    
	@sa InputStreamReader
*/
//==============================================================================

#include "OutputStreamWriter.h"
#include "OutputStream.h"
#include "UnsupportedEncodingException.h"

#include "QcCore/base/StringUtils.h"
#include "QcCore/cvt/CodeConverterFactory.h"

QC_IO_NAMESPACE_BEGIN

using namespace cvt;

const size_t ByteBufferSize = 2000;
const size_t CharSeqBufferSize = 32;

//==============================================================================
// OutputStreamWriter::OutputStreamWriter
//
/**
   Constructs an OutputStreamWriter with @c pOutputStream as the contained 
   OutputStream.

   The OutputStream will be encoded using the default CodeConverter
   returned from the CodeConverterFactory.  Unmappable characters 
   will be silently converted into replacement characters.
   
   @param pOutputStream the contained OutputStream.
   @throws NullPointerException if @c pOutputStream is null.

   @sa cvt::CodeConverterFactory::getDefaultConverter()
*/
//==============================================================================
OutputStreamWriter::OutputStreamWriter(OutputStream* pOutputStream) :
	m_rpOutputStream(pOutputStream),
	m_pByteBuffer(0),
	m_byteBufferSize(0),
	m_byteBufferUsed(0),
	m_pCharSeqBuffer(0),
	m_charSeqBufferUsed(0),
	m_bAtStart(true)
{
	if(!pOutputStream) throw NullPointerException();
	init(String(), false);
}

//==============================================================================
// OutputStreamWriter::OutputStreamWriter
//
/**
   Constructs an OutputStreamWriter with @c pOutputStream as the contained 
   OutputStream and @c encoding as the specified encoding name.

   The OutputStream will be encoded using a CodeConverter obtained from the
   CodeConverterFactory.  Unmappable characters  will be silently converted
   into replacement characters.

   @param pOutputStream the contained OutputStream.
   @param encoding the name of the encoding (e.g. "UTF-8")
   @throws NullPointerException if @c pOutputStream is null.
   @throws UnsupportedEncodingException if the CodeConverterFactory is unable
           to create a CodeConverter for the specified @c encoding
*/
//==============================================================================
OutputStreamWriter::OutputStreamWriter(OutputStream* pOutputStream, const String& encoding) :
	m_rpOutputStream(pOutputStream),
	m_pByteBuffer(0),
	m_byteBufferSize(0),
	m_byteBufferUsed(0),
	m_pCharSeqBuffer(0),
	m_charSeqBufferUsed(0),
	m_bAtStart(true)
{
	if(!pOutputStream) throw NullPointerException();
	init(encoding, false);
}

//==============================================================================
// OutputStreamWriter::OutputStreamWriter
//
/**
   Constructs an OutputStreamWriter with @c pOutputStream as the contained 
   OutputStream and @pEncoder as the CodeConverter which will encode Unicode
   characters into bytes for the underlying output stream.

   The policy for dealing with unmappable characters can be 
   specified using the CodeConverter::setUnmappableCharAction() method.

   @param pOutputStream the contained OutputStream.
   @param pEncoder the CodeConverter to use for encoding Unicode characters into bytes
   @throws NullPointerException if either @c pOutputStream or @c pEncoder is null.
*/
//==============================================================================
OutputStreamWriter::OutputStreamWriter(OutputStream* pOutputStream, CodeConverter* pEncoder) :
	m_rpOutputStream(pOutputStream),
	m_pByteBuffer(0),
	m_byteBufferSize(0),
	m_byteBufferUsed(0),
	m_pCharSeqBuffer(0),
	m_charSeqBufferUsed(0),
	m_bAtStart(true)
{
	if(!pOutputStream) throw NullPointerException();
	initEncoder(pEncoder);
}

//==============================================================================
// OutputStreamWriter::OutputStreamWriter
//
/**
   Constructs an OutputStreamWriter with @c pOutputStream as the contained 
   OutputStream and @c encoding as the specified encoding name.

   The OutputStream will be encoded using a CodeConverter obtained from the
   CodeConverterFactory.  The policy for the treatment of malformed
   byte sequences is specified using the @c bStrict parameter.

   @param pOutputStream the contained OutputStream.
   @param encoding the name of the encoding (e.g. "UTF-8")
   @param bStrict when true, the CodeConverter is instructed to throw
          an UnmappableCharacterException when it encounters a Unicode
		  character that cannot be encoded into the specified byte encoding;
		  otherwise unmappable characters are silently converted into a replacement
		  character
   @throws NullPointerException if @c pOutputStream is null.
   @throws UnsupportedEncodingException if the CodeConverterFactory is unable
           to create a CodeConverter for the specified @c encoding
*/
//==============================================================================
OutputStreamWriter::OutputStreamWriter(OutputStream* pOutputStream,
                                       const String& encoding,
                                       bool bStrict) :
	m_rpOutputStream(pOutputStream),
	m_pByteBuffer(0),
	m_byteBufferSize(0),
	m_byteBufferUsed(0),
	m_pCharSeqBuffer(0),
	m_charSeqBufferUsed(0),
	m_bAtStart(true)
{
	if(!pOutputStream) throw NullPointerException();
	init(encoding, bStrict);
}

//==============================================================================
// OutputStreamWriter::~OutputStreamWriter
//
/**
   The destructor flushes the byte buffer to the underlying OutputStream before
   freeing resources associated with this OutputStreamWriter.

   The underlying OutputStream is not explicitly closed, but it will be
   automatically closed when no further references to it exist.
*/
//==============================================================================
OutputStreamWriter::~OutputStreamWriter()
{
	if(m_rpOutputStream)
	{
		try
		{
			flush();
		}
		catch(Exception& /*e*/)
		{
		}
	}
	freeBuffers();
}

//==============================================================================
// OutputStreamWriter::freeBuffers
//
// Private helper function to free the buffers.
//==============================================================================
void OutputStreamWriter::freeBuffers()
{
	delete [] m_pByteBuffer; m_pByteBuffer = 0;
	m_byteBufferUsed = m_byteBufferSize = 0;
	delete [] m_pCharSeqBuffer; m_pCharSeqBuffer = 0;
	m_charSeqBufferUsed = 0;
}

//==============================================================================
// OutputStreamWriter::init
//
// Private initialization function.
//==============================================================================
void OutputStreamWriter::init(const String& encoding, bool bStrictEncoding)
{
	AutoPtr<CodeConverter> rpEncoder;

	if(encoding.empty())
	{
		rpEncoder = CodeConverterFactory::GetInstance().getDefaultConverter();
	}
	else
	{
		rpEncoder = CodeConverterFactory::GetInstance().getConverter(encoding);
	}

	if(rpEncoder.isNull())
	{
		throw UnsupportedEncodingException(encoding);
	}

	if(bStrictEncoding)
	{
		rpEncoder->setUnmappableCharAction(CodeConverter::abort);
	}
	
	initEncoder(rpEncoder.get());
}

//==============================================================================
// OutputStreamWriter::initEncoder
//
// Private initialization function.
//==============================================================================
void OutputStreamWriter::initEncoder(CodeConverter* pEncoder)
{
	if(!pEncoder) throw NullPointerException();

	m_rpEncoder = pEncoder;

	// Note: this is checked again after the first write (for BOM dependency)
	m_bRequiresEncoding = !(m_rpEncoder->alwaysNoConversion());
	
	// If decoding is required (ie the underlying byte stream
	// is not encoded in the same way as the internal quickcpp encoding)
	// then allocate a buffer for the efficient writing of Bytes.
	if(m_bRequiresEncoding)
	{
		m_byteBufferSize = ByteBufferSize;
		m_pByteBuffer = new Byte [m_byteBufferSize];
	}
}

//==============================================================================
// OutputStreamWriter::close
//
//==============================================================================
void OutputStreamWriter::close()
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	if(m_rpOutputStream)
	{
		flush();
		m_rpOutputStream->close();
		m_rpOutputStream.release();
	}
	freeBuffers();
}

//==============================================================================
// OutputStreamWriter::flush
//
//==============================================================================
void OutputStreamWriter::flush()
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	if(!m_rpOutputStream) throw IOException(QC_T("stream is closed"));

	writeByteBuffer();
	m_rpOutputStream->flush();
}

//==============================================================================
// OutputStreamWriter::flushBuffers
//
//==============================================================================
void OutputStreamWriter::flushBuffers()
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	if(!m_rpOutputStream) throw IOException(QC_T("stream is closed"));

	writeByteBuffer();
	m_rpOutputStream->flushBuffers();
}

//==============================================================================
// OutputStreamWriter::writeByteBuffer
//
// Write the byte buffer to the underlying OutputStream.
// Note: This does not flush the underlying stream, because to do so
//       would undermine the buffering scheme of the OutputStream.
// MT Note: m_rpLock must be held prior to calling
//==============================================================================
void OutputStreamWriter::writeByteBuffer()
{
	QC_DBG_ASSERT(m_rpOutputStream);
	if(m_byteBufferUsed)
	{
		m_rpOutputStream->write(m_pByteBuffer, m_byteBufferUsed);
		m_byteBufferUsed = 0;
	}
}

//==============================================================================
// OutputStreamWriter::getEncoding
//
/**
   Returns the canonical name of the encoding employed by the underlying
   byte stream.
   @synchronized
*/
//==============================================================================
String OutputStreamWriter::getEncoding() const
{
	return m_rpEncoder->getEncodingName();
}

//==============================================================================
// OutputStreamWriter::write
//
// Encode and write an array of characters to the Output stream.
//
// If the internal QuickCPP encoding is identical to the output encoding,
// then the encoding step can be skipped.
//==============================================================================
void OutputStreamWriter::write(const CharType* pBuffer, size_t bufLen)
{
	if(!pBuffer) throw NullPointerException();
	
	QC_SYNCHRONIZED_PTR(m_rpLock)

	if(!m_rpOutputStream) throw IOException(QC_T("stream is closed"));

	//
	// If no encoding is required we are in the fortunate position
	// of being able to write bytes directly to the output stream
	//
	if(!m_bRequiresEncoding)
	{
		QC_DBG_ASSERT(0 == m_charSeqBufferUsed);
		m_rpOutputStream->write((const Byte*)pBuffer, bufLen*sizeof(CharType));
	}
	else if(bufLen)	// valid buffer and it does require encoding
	{
		//
		// Before jumping off to encode the supplied character buffer,
		// we first check that there isn't an unclosed character sequence
		// pending from the last write operation.
		//
		// If there is an unclosed sequence, we steal characters
		// from the input buffer and append them to the unclosed
		// sequence until the sequence is closed.
		//
		//
		if(m_charSeqBufferUsed)
		{
			QC_DBG_ASSERT(m_pCharSeqBuffer!=0);
			const size_t seqLen = SystemCodeConverter::GetCharSequenceLength(*m_pCharSeqBuffer);
			QC_DBG_ASSERT(seqLen > m_charSeqBufferUsed);
			QC_DBG_ASSERT(seqLen < CharSeqBufferSize);
			size_t charsToSteal = m_charSeqBufferUsed - seqLen;
			if(charsToSteal >= bufLen) charsToSteal = bufLen;

			::memcpy(m_pCharSeqBuffer+m_charSeqBufferUsed, pBuffer, charsToSteal*sizeof(CharType));
			
			m_charSeqBufferUsed+=charsToSteal;
			// adjust input buffer to reflect stolen characters
			bufLen-=charsToSteal;
			pBuffer+=charsToSteal;

			if(m_charSeqBufferUsed == seqLen)
			{
				if(doEncoding(m_pCharSeqBuffer, m_charSeqBufferUsed))
				{
					m_charSeqBufferUsed = 0;
				}
				else
				{
					QC_DBG_ASSERT(false); // should not reach here
				}
			}
		}

		//
		// If there is anything left to encode, do it
		//
		if(bufLen)
		{
			doEncoding(pBuffer, bufLen);
		}
	}
}

//==============================================================================
// OutputStreamWriter::doEncoding
//
// MT Note: m_rpLock must be held prior to calling
//==============================================================================
bool OutputStreamWriter::doEncoding(const CharType* pBuffer, size_t bufLen)
{
	QC_DBG_ASSERT(0 == m_charSeqBufferUsed || pBuffer == m_pCharSeqBuffer);
	QC_DBG_ASSERT(bufLen!=0);

	size_t charsRemaining = bufLen;
	const CharType* fromNext = pBuffer;

	while(charsRemaining)
	{
		Byte* toNext;
		const CharType* pCharStart = fromNext;

		//
		// Make room if necessary
		//
		if(m_byteBufferUsed == m_byteBufferSize)
		{
			writeByteBuffer();
		}

		//
		// Every time writeByteBuffer is called, we ask the encoder again
		// if conversion is really required.  Sometimes (UTF-16) conversion is 
		// only required until the initial BOM is written.
		//

		CodeConverter::Result result = 
			m_rpEncoder->encode(pCharStart, pBuffer+bufLen, fromNext, m_pByteBuffer+m_byteBufferUsed, m_pByteBuffer+m_byteBufferSize, toNext);

		m_byteBufferUsed = toNext-m_pByteBuffer;

		QC_DBG_ASSERT(m_byteBufferUsed <= m_byteBufferSize);
		QC_DBG_ASSERT(fromNext <= pBuffer+bufLen);

		charsRemaining = pBuffer+bufLen-fromNext;

		if((result == CodeConverter::outputExhausted) || 
		   (m_byteBufferUsed == m_byteBufferSize))
		{
			writeByteBuffer();
		}
		else if(result == CodeConverter::inputExhausted)
		{
			//
			// If the result from encode() is "inputExhausted" then we have a multi-byte
			// internal encoding (either a UTF-8 sequence or a UTF-16 surrogate pair) that
			// has not been completely written to the stream.
			//
			// In this case we must buffer the remaining characters until the next write
			// operation is performed.
			//
			QC_DBG_ASSERT(pBuffer != m_pCharSeqBuffer);

			if(pBuffer != m_pCharSeqBuffer)
			{
				if(!m_pCharSeqBuffer)
				{
					m_pCharSeqBuffer = new CharType [CharSeqBufferSize];
				}
				
				size_t charsToStore = pBuffer+bufLen - fromNext;
				QC_DBG_ASSERT(charsToStore <=  CharSeqBufferSize);
				::memcpy(m_pCharSeqBuffer, fromNext, charsToStore*sizeof(CharType));
				m_charSeqBufferUsed = charsToStore;
				break;
			}
			else
			{
				return false;
			}
		}
		else if(result != CodeConverter::ok)
		{
			String errMsg = StringUtils::FromLatin1(StringUtils::Format("unable to encode character 0x%04lX", (unsigned long)*fromNext));
			throw IOException(errMsg);
		}
		else
		{
			QC_DBG_ASSERT(charsRemaining == 0);
		}
	}

	//
	// After the very first write, we check again whether the encoder is
	// really required.  If it is no longer required we write out
	// any outstanding bytes, so that the next write can go directly to the
	// output stream.
	//
	if(m_bAtStart)
	{
		m_bAtStart = false;
		m_bRequiresEncoding = !(m_rpEncoder->alwaysNoConversion());
		if(!m_bRequiresEncoding)
		{
			writeByteBuffer(); 
			if(m_charSeqBufferUsed)
			{
				m_rpOutputStream->write((const Byte*)m_pCharSeqBuffer, m_charSeqBufferUsed*sizeof(CharType));
				m_charSeqBufferUsed=0;
			}
		}
	}

	return true;
}

//==============================================================================
// OutputStreamWriter::getEncoder
//
/**
   Returns a reference to the CodeConverter employed by this OutputStreamWriter.
   @synchronized
*/
//==============================================================================
AutoPtr<CodeConverter> OutputStreamWriter::getEncoder() const
{
	return m_rpEncoder;
}

QC_IO_NAMESPACE_END

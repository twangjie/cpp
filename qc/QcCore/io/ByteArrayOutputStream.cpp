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
/**
	@class qc::io::ByteArrayOutputStream
	
	@brief An output stream that stores the data in an internal buffer. 

    When the ByteArrayOutputStream is constructed, an internal buffer is created. The
	size of the internal buffer can be specified by using the overloaded
	constructor.  

    When bytes are written to this output stream, they are copied into
	the internal buffer.  The buffer is automatically expanded as additional
	bytes are written to it.

    The buffered data may be retrieved at any time using the data() and size()
	methods.

	@sa ByteArrayInputStream
*/
//=============================================================================

#include "ByteArrayOutputStream.h"
#include "MalformedInputException.h"
#include "UnsupportedEncodingException.h"
#include "IOException.h"

#include "QcCore/cvt/CodeConverterFactory.h"
#include "QcCore/base/NullPointerException.h"

QC_IO_NAMESPACE_BEGIN

using namespace cvt;

//==============================================================================
// ByteArrayOutputStream::ByteArrayOutputStream
//
/**
   Creates a ByteArrayOutputStream with a default initial buffer size.

   As bytes are written to the output stream the buffer is automatically
   extended as required.
*/
//==============================================================================
ByteArrayOutputStream::ByteArrayOutputStream() :
	m_bClosed(false)
{
}

//==============================================================================
// ByteArrayOutputStream::ByteArrayOutputStream
//
/**
   Creates a ByteArrayOutputStream with a specified initial buffer size.
   As bytes are written to the output stream the buffer is automatically
   extended as required.

   @param size the initial buffer size.
*/
//==============================================================================
ByteArrayOutputStream::ByteArrayOutputStream(size_t size) :
	m_buffer(size),
	m_bClosed(false)
{
}

//==============================================================================
// ByteArrayOutputStream::close
//
/**
   Closes the ByteArrayOutputStream.

   The internal buffer is intentionally not released so that it remains available
   for subsequent calls to getData() and toString().
*/
//==============================================================================
void ByteArrayOutputStream::close()
{
	m_bClosed = true;
}

//==============================================================================
// ByteArrayOutputStream::write
//
//==============================================================================
void ByteArrayOutputStream::write(const Byte* pBuffer, size_t bufLen)
{
	if(m_bClosed) throw IOException(QC_T("cannot write to a closed stream"));

	m_buffer.append(pBuffer, bufLen);
}

//==============================================================================
// ByteArrayOutputStream::reset
//
/**
   Resets the internal buffer to zero size.

   This does not change the capacity of the internal buffer or free the
   resources used by the buffer.
*/
//==============================================================================
void ByteArrayOutputStream::reset()
{
	m_buffer.clear();
}

//==============================================================================
// ByteArrayOutputStream::writeTo
//
/**
   Writes the contents of the internal buffer to the specified OutputStream.

   @param pOut the OutputStream to write the contents of the buffer to.
   @throws IOException if an error occurs writing to the OutputStream.
   @throws NullPointerException if @c pOut is null.
*/
//==============================================================================
void ByteArrayOutputStream::writeTo(OutputStream* pOut) const
{
	if(!pOut) throw NullPointerException();

	pOut->write(m_buffer.data(), m_buffer.size());
}

//==============================================================================
// ByteArrayOutputStream::size
//
/**
   Returns the number of bytes written to the internal byte buffer.
   @sa data()
*/
//==============================================================================
size_t ByteArrayOutputStream::size() const
{
	return m_buffer.size();
}

//==============================================================================
// ByteArrayOutputStream::data
//
/**
   Returns a constant pointer to the start of the internal byte buffer.
   @sa size()
*/
//==============================================================================
const Byte* ByteArrayOutputStream::data() const
{
	return m_buffer.data();
}

//==============================================================================
// ByteArrayOutputStream::toString
//
/**
   Converts the bytes in the internal buffer into a Unicode character string
   using the supplied encoding name.

   @param enc the name of the encoding.
   @throws UnsupportedEncodingException if @c enc is not a supported encoding
*/
//==============================================================================
String ByteArrayOutputStream::toString(const String& encoding) const
{
	AutoPtr<CodeConverter> rpDecoder;
	if(encoding.empty())
	{
		rpDecoder = CodeConverterFactory::GetInstance().getDefaultConverter();
	}
	else
	{
		rpDecoder = CodeConverterFactory::GetInstance().getConverter(encoding);
	}

	if(rpDecoder.isNull())
	{
		throw UnsupportedEncodingException(encoding);
	}

	return toString(rpDecoder.get());
}

//==============================================================================
// ByteArrayOutputStream::toString
//
/**
   Converts the bytes in the internal buffer into a Unicode character string
   using the default encoding.

   @throws UnsupportedEncodingException if @c enc is not a supported encoding
*/
//==============================================================================
String ByteArrayOutputStream::toString() const
{
	AutoPtr<CodeConverter> rpDecoder =
		CodeConverterFactory::GetInstance().getDefaultConverter();
	return toString(rpDecoder.get());
}

//==============================================================================
// ByteArrayOutputStream::toString
//
/**
   Converts the bytes in the internal buffer into a Unicode character string
   using the supplied CodeConverter.

   @param pDecoder the code converter to use for decoding bytes into
   Unicode characters
*/
//==============================================================================
String ByteArrayOutputStream::toString(CodeConverter* pDecoder) const
{
	//
	// If the byte buffer is encoded identically to
	// our internal character encoding (which is quite likely,
	// especially when using UTF-8), then we can optimize by 
	// bypassing some of the buffering and conversion work.
	// However, if an encoding error is detected, we must revert to
	// standard conversion.
	//
	if(pDecoder->alwaysNoConversion())
	{
		return String((const CharType*)m_buffer.data(), m_buffer.size());
	}

	//
	// Otherwise we have to do some work...
	//

	String ret;

	const size_t WorkBufferSize=256;
	const size_t OverflowSize=10;
	CharType workBuffer[WorkBufferSize];
	const Byte* pFromNext = m_buffer.data();
	const Byte* pFromEnd = pFromNext+m_buffer.size();

	while(pFromNext < pFromEnd)
	{
		CharType* pNextChar = workBuffer;
		CodeConverter::Result result;

		result = pDecoder->decode(pFromNext,
			                      pFromEnd,
			                      pFromNext,
			                      workBuffer,
			                      workBuffer+WorkBufferSize-OverflowSize,
			                      pNextChar);

		//
		// If the input buffer is exhausted then
		// we have a malformed input situation.  However, we do not
		// automatically throw an exception, we must use the policy
		// of the Decoder.
		//
		if(result == CodeConverter::inputExhausted)
		{
			if(pDecoder->getInvalidCharAction() == CodeConverter::abort)
			{
				throw MalformedInputException(QC_T("premature end of multi-byte sequence"), pDecoder);
			}
			else
			{
				CodeConverter::Result replRes = 
					SystemCodeConverter::ToInternalEncoding(
						pDecoder->getInvalidCharReplacement(),
						pNextChar,
						workBuffer+WorkBufferSize, // includes overflow area
						pNextChar);
				
				if(replRes == CodeConverter::outputExhausted)
				{
					// oh dear, what a pickle.  We have received an eof
					// part-way through a multi-character sequence and the
					// replacement character cnanot be encoded into the
					// output buffer.  This should not be possible
					// because our buffer has an overflow portion to deal with
					// this (unlikely) situation.
					throw MalformedInputException(QC_T("unable to encode byte sequence"), pDecoder);
				}

				// The replacement char being returned is a surrogate
				// for the remaining bytes in the buffer.
				pFromNext = pFromEnd;
			}
		}

		// make a note of how many characters were converted this time
		const size_t charsConverted = (pNextChar - workBuffer);

		QC_DBG_ASSERT(charsConverted!=0);
		QC_DBG_ASSERT(pFromNext == pFromEnd || result==CodeConverter::outputExhausted);

		//
		// Add the characters to the return string
		//
		ret.append(workBuffer, charsConverted);
	}

	return ret;
}

QC_IO_NAMESPACE_END

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
/**
	@class qc::io::InputStreamReader
	
	@brief An InputStreamReader reads raw bytes from an InputStream and translates
	them into Unicode characters using an instance of the CodeConverter class 
	to perform the translation.

    The encoding of the underlying byte input stream may be specified by
	name or by providing an instance of a CodeConverter.  If no encoding is
	specified the system will use a default encoding.
	
    A CodeConverter can be configured to behave in one of two specified
	ways when it encounters an encoding error.  By default,
	encoding errors are silently dealt with by skipping the invalid byte sequence
	and returning the replacement character U+FFFD to the application.  A stricter
	scheme is also available that treats encoding errors as non-recoverable 
	and throws a MalformedInputException.  Overloaded versions of the InputStreamReader
	constructors can be used to explicitly set the required policy.

    To improve efficiency, the InputStreamReader contains a byte buffer
	into which it reads bytes from the underlying input stream.  Therefore
	more bytes may be read ahead from the underlying stream than are necessary
	to satisfy the current read operation. 
	
    The following example demonstrates a simple transcoding function.  It
	decodes a UTF-8 encoded file into a stream of Unicode characters and 
	then writes the characters out encoded into bytes using the UTF-16 encoding:-

    @code
    File in(QC_T("utf8.txt"));
    File out(QC_T("utf16.txt"));

    AutoPtr<Reader> rpRdr  = new InputStreamReader(
                            new FileInputStream(in), QC_T("UTF-8") );

    AutoPtr<Writer> rpWtr = new OutputStreamWriter(
                           new FileOutputStream(out), QC_T("UTF-16") );

    CharType buffer[1024];
    long count;
    while( (count=rpRdr->read(buffer, sizeof(buffer))) != Reader::EndOfFile)
    {
        rpWtr->write(buffer, count);
    }
    rpWtr->flush();
    @endcode

	@mt
    As shown on the inheritance graph, InputStreamReader @a derives from 
	SynchronizedObject, which gives it the ability to protect its internal
	state from concurrent access from multiple threads.  All public methods 
	are synchronized for safe concurrent access.
    
	@sa OutputStreamWriter
*/
//==============================================================================

#include "InputStreamReader.h"
#include "AtomicReadException.h"
#include "CharacterCodingException.h"
#include "InputStream.h"
#include "MalformedInputException.h"
#include "UnsupportedEncodingException.h"

#include "QcCore/base/SystemUtils.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/base/IllegalCharacterException.h"
#include "QcCore/cvt/CodeConverterFactory.h"

#include <memory>

QC_IO_NAMESPACE_BEGIN

using namespace cvt;

const size_t ByteBufferSize = 2000 * sizeof(CharType);
const size_t OverflowBufferSize = 3 * sizeof(CharType);

//==============================================================================
// InputStreamReader::InputStreamReader
//
/**
   Constructs an InputStreamReader with @c pInputStream as the contained 
   InputStream.

   The InputStream will be decoded using the default CodeConverter
   returned from the CodeConverterFactory.  Malformed
   byte sequences will be silently converted into replacement characters.
   
   @param pInputStream the contained InputStream.
   @throws NullPointerException if @c pInputStream is null.
   @sa CodeConverterFactory::getDefaultConverter()
*/
//==============================================================================
InputStreamReader::InputStreamReader(InputStream* pInputStream) :
	m_rpInputStream(pInputStream),
	m_pByteBuffer(0), m_pNextByteAvailable(0), m_pNextByteFree(0),
	m_byteBufferSize(0),
	m_pCharSeqNext(0),
	m_charSeqLen(0),
	m_bRequiresDecoding(false),
	m_bAtEof(false)
{
	if(!pInputStream) throw NullPointerException();

	init(String(), false);
}

//==============================================================================
// InputStreamReader::InputStreamReader
//
/**
   Constructs an InputStreamReader with @c pInputStream as the contained 
   InputStream and @c encoding as the specified encoding name.

   The InputStream will be decoded using a CodeConverter obtained from the
   CodeConverterFactory.  Malformed
   byte sequences will be silently converted into replacement characters.

   @param pInputStream the contained InputStream.
   @param encoding the name of the encoding (e.g. "UTF-8")
   @throws NullPointerException if @c pInputStream is null.
   @throws UnsupportedEncodingException if the CodeConverterFactory is unable
           to create a CodeConverter for the specified @c encoding
*/
//==============================================================================
InputStreamReader::InputStreamReader(InputStream* pInputStream, const String& encoding) :
	m_rpInputStream(pInputStream),
	m_pByteBuffer(0), m_pNextByteAvailable(0), m_pNextByteFree(0),
	m_byteBufferSize(0),
	m_pCharSeqNext(0),
	m_charSeqLen(0),
	m_bRequiresDecoding(false),
	m_bAtEof(false)
{
	if(!pInputStream) throw NullPointerException();

	init(encoding, false);
}

//==============================================================================
// InputStreamReader::InputStreamReader
//
/**
   Constructs an InputStreamReader with @c pInputStream as the contained 
   InputStream and @c encoding as the specified encoding name.

   The InputStream will be decoded using a CodeConverter obtained from the
   CodeConverterFactory.  The policy for the treatment of malformed
   byte sequences is specified using the @c bStrict parameter.

   @param pInputStream the contained InputStream.
   @param encoding the name of the encoding (e.g. "UTF-8")
   @param bStrict when true, the CodeConverter is instructed to throw
          a MalformedInputException when it encounters an invalid byte sequence;
		  otherwise invalid byte sequences are silently converted into a replacement
		  character
   @throws NullPointerException if @c pInputStream is null.
   @throws UnsupportedEncodingException if the CodeConverterFactory is unable
           to create a CodeConverter for the specified @c encoding
*/
//==============================================================================
InputStreamReader::InputStreamReader(InputStream* pInputStream,
                                     const String& encoding,
                                     bool bStrict) :
	m_rpInputStream(pInputStream),
	m_pByteBuffer(0), m_pNextByteAvailable(0), m_pNextByteFree(0),
	m_byteBufferSize(0),
	m_pCharSeqNext(0),
	m_charSeqLen(0),
	m_bRequiresDecoding(false),
	m_bAtEof(false)
{
	if(!pInputStream) throw NullPointerException();

	init(encoding, bStrict);
}

//==============================================================================
// InputStreamReader::InputStreamReader
//
/**
   Constructs an InputStreamReader with @c pInputStream as the contained 
   InputStream and @c pDecoder as the CodeConverter which will translate
   bytes from the input stream into Unicode characters.

   The policy for dealing with malformed byte sequences can be 
   specified using the CodeConverter::setInvalidCharAction() method.

   @param pInputStream the contained InputStream.
   @param pDecoder the CodeConverter to use for decoding bytes into Unicode characters
   @throws NullPointerException if either @c pInputStream or @c pDecoder is null.
*/
//==============================================================================
InputStreamReader::InputStreamReader(InputStream* pInputStream, CodeConverter* pDecoder) :
	m_rpInputStream(pInputStream),
	m_pByteBuffer(0), m_pNextByteAvailable(0), m_pNextByteFree(0),
	m_byteBufferSize(0),
	m_pCharSeqNext(0),
	m_charSeqLen(0),
	m_bRequiresDecoding(false),
	m_bAtEof(false)
{
	if(!pInputStream) throw NullPointerException();

	initDecoder(pDecoder);
}

//==============================================================================
// InputStreamReader::~InputStreamReader
//
/**
   The destructor frees resources associated with this InputStreamReader.
   The underlying InputStream is not explicitly closed, but it will be
   automatically closed when no further references to it exist.
*/
//==============================================================================
InputStreamReader::~InputStreamReader()
{
	freeBuffers();
}

//==============================================================================
// InputStreamReader::init
//
// Common initialization (called from constructors)
//==============================================================================
void InputStreamReader::init(const String& encoding, bool bStrict)
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
	else
	{
		//
		// If we have been asked to be strict with any sloppy encoding
		// errors, we pass this on to the decoder.
		//
		if(bStrict)
		{
			rpDecoder->setInvalidCharAction(CodeConverter::abort);
		}
		initDecoder(rpDecoder.get());
	}
}

//==============================================================================
// InputStreamReader::initDecoder
//
// Common Initialization
//==============================================================================
void InputStreamReader::initDecoder(CodeConverter* pDecoder)
{
	if(!pDecoder) throw NullPointerException();

	m_rpDecoder = pDecoder;

	m_bRequiresDecoding = !(m_rpDecoder->alwaysNoConversion());
	
	// If decoding is required (ie the underlying byte stream
	// is not encoded in the same way as the internal quickcpp encoding)
	// then allocate a buffer for the efficient reading of Bytes.

	if(m_bRequiresDecoding)
	{
		m_byteBufferSize = ByteBufferSize;
		m_pByteBuffer = new Byte[m_byteBufferSize];
		m_pNextByteFree = m_pNextByteAvailable = m_pByteBuffer;
	}
}

//==============================================================================
// InputStreamReader::fillByteBuffer
//
// Private function called whenever there are insufficient unread bytes in the
// buffer for the read function to successfully create a character.
//
// This function will read at least one more byte into the buffer (unless the
// InputStream is at EOF).  If necessary the buffer will be re-organized
// to make room for extra bytes.
//
// MT Note: m_lock must be held prior to calling
//==============================================================================
void InputStreamReader::fillByteBuffer()
{
	QC_DBG_ASSERT(!m_bAtEof);
	QC_DBG_ASSERT(m_pByteBuffer!=0);

	//
	// If we have successfully used up all the bytes in the buffer
	// then we can expedite things by resetting the buffer pointers
	// back to the beginning
	//
	if(m_pNextByteAvailable > m_pByteBuffer && 
	   m_pNextByteAvailable == m_pNextByteFree)
	{
		m_pNextByteAvailable = m_pNextByteFree = m_pByteBuffer;
	}

	//
	// Calculate the free space in the buffer
	//
	size_t freeBytes = (m_pByteBuffer+m_byteBufferSize) - m_pNextByteFree;

	//
	// If there is room at the end of the buffer, we just read some bytes
	// into it and return in the expectation that we have done enough
	// for the caller to be able to perform some useful work.
	//
	if(freeBytes)
	{
		int numBytes = m_rpInputStream->read(m_pNextByteFree, freeBytes);

		if(numBytes == InputStream::EndOfFile)
		{
			m_bAtEof = true;
		}
		else
		{
			m_pNextByteFree+=numBytes;
		}
	}
	//
	// If there aren't any free bytes in the buffer, we need to re-organize it.
	// Re-organisation is only possible if at least one byte has actually
	// been read from the buffer.  Otherwise we have an unrecoverable error.
	//
	else if(m_pNextByteAvailable > m_pByteBuffer)
	{
		//
		// Some of the buffer has been read, the rest contains 
		// unconsumed data.  It is our job to re-organize the buffer
		// so that the unconsumed data is moved to the start of the 
		// buffer, thereby making room available for the next read operation.
		//
		size_t bytesLeft = m_pNextByteFree-m_pNextByteAvailable;
		::memmove(m_pByteBuffer, m_pNextByteAvailable, bytesLeft);
		m_pNextByteFree = m_pByteBuffer + bytesLeft;
		m_pNextByteAvailable = m_pByteBuffer;
	}
	else
	{
		// ARGGHH!
		throw IOException(QC_T("Input buffer too small to hold required sequence"));
	}
}

//==============================================================================
// InputStreamReader::freeBuffers
//
// Free the Byte buffer used to store characters from the input stream.
//==============================================================================
void InputStreamReader::freeBuffers()
{
	delete [] m_pByteBuffer;
	m_pNextByteFree = m_pNextByteAvailable = m_pByteBuffer = 0;
	m_byteBufferSize = 0;

	m_pCharSeqNext = m_charSeqBuffer;
	m_charSeqLen = 0;
}

//==============================================================================
// InputStreamReader::close
//
/**
   Closes the Reader and its associated InputStream.
   
   Once a Reader is closed, all system resources associated with the Reader are
   released, preventing any further read(), mark(), reset() or skip()
   operations.  However, further calls to close() have no effect.

   @throws IOException if an I/O error occurs.
   @synchronized
*/
//==============================================================================
void InputStreamReader::close()
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	if(m_rpInputStream)
	{
		m_rpInputStream->close();
		m_rpInputStream.release();
	}
	freeBuffers();
}

//==============================================================================
// InputStreamReader::getEncoding
//
/**
   Returns the canonical name of the encoding employed by the underlying
   byte stream.
   @synchronized
*/
//==============================================================================
String InputStreamReader::getEncoding() const
{
	return m_rpDecoder->getEncodingName();
}

//==============================================================================
// InputStreamReader::read
//
// Read and decode an array of characters from the input stream.
//
// This is the REAL man's reading method and must be made very efficient
// because it will be called all the time!
//
// At least one Character is read from the input stream.
//==============================================================================
long InputStreamReader::read(CharType* pBuffer, size_t bufLen)
{
	SystemUtils::TestBufferIsValid(pBuffer, bufLen);
	
	QC_SYNCHRONIZED_PTR(m_rpLock)
	
	int returnLen = 0;

	//
	// If we have some stored characters in our character sequence
	// buffer then use those first.  Then, if there is any room
	// left in the output buffer, fill that also.
	//
	if(m_charSeqLen)
	{
		QC_DBG_ASSERT(m_pCharSeqNext!=0);

		size_t charCount = bufLen;
		if(m_charSeqLen < charCount) charCount = m_charSeqLen;
		::memcpy(pBuffer, m_pCharSeqNext, charCount*sizeof(CharType));
		m_pCharSeqNext += charCount;
		m_charSeqLen -= charCount;
		bufLen -= charCount;
		pBuffer += charCount;
		returnLen += charCount;
	}

	//
	// If we still have room for more...
	//
	if(bufLen)
	{
		//
		// Don't make a blocking call if we have already read some
		// characters into the return buffer
		//
		returnLen += readAndDecode((returnLen==0), pBuffer, bufLen, false);
	}

	return returnLen ? returnLen : EndOfFile;
}

//==============================================================================
// InputStreamReader::readAtomic
//
// An Atomic read is similar to a buffered read, except that only complete
// character sequences are returned.  This means different things depending
// on what the internal character representation is:
// If character strings are UTF-8 strings, then only complete UTF-8 sequences
// are returned.
// If strings are UTF-16 strings, then surrogate pairs are returned together.
// If strings are USC-4 characters, this has exactly the same semantics as read()
//==============================================================================
long InputStreamReader::readAtomic(CharType* pBuffer, size_t bufLen)
{
	SystemUtils::TestBufferIsValid(pBuffer, bufLen);
	
	QC_SYNCHRONIZED_PTR(m_rpLock)

	//
	// It is not legal for us to have stored characters in our character
	// sequence buffer.  This would imply that the application has
	// performed an incomplete non-Atmomic read prior to calling this.
	//
	if(m_charSeqLen)
	{
		throw AtomicReadException(QC_T("not on multi-character sequence boundary"));
	}

	size_t charsRead = readAndDecode(true, pBuffer, bufLen, true);
	return charsRead ? charsRead : (m_bAtEof ? EndOfFile : 0);
}

//==============================================================================
// InputStreamReader::readAtomic
//
//==============================================================================
Character InputStreamReader::readAtomic()
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	//
	// It is not legal for us to have stored characters in our character
	// sequence buffer.  This would imply that the application has
	// performed an incomplete non-Atmomic read prior to calling this.
	//
	if(m_charSeqLen)
	{
		throw AtomicReadException(QC_T("not on character sequence boundary"));
	}

	//
	// We only want to read one unicode character, but we don't know how
	// many character positions the next unicode character will take.
	//
	// To overcome this problem, we perform a non-atomic read of a single character
	// followed by a read of the remaining characters from the sequence (if any).
	//
	const size_t maxSeqLen = 4;
	CharType buffer[maxSeqLen];
	long charsRead;

	if( (charsRead=read(buffer, 1)) == 1)
	{
		/*
		if(!SystemCodeConverter::IsSequenceStartChar(*buffer))
		{
			throw MalformedInputException((const Byte*)buffer, 1*sizeof(CharType), m_rpDecoder);
			throw AtomicReadException(QC_T("not on character sequence boundary"));
		}
		*/

		size_t seqLen = SystemCodeConverter::GetCharSequenceLength(*buffer);
		QC_DBG_ASSERT(seqLen <= maxSeqLen);

		while(charsRead < (long)seqLen)
		{
			long rc=read(buffer+charsRead, seqLen-charsRead);
			if(rc == EndOfFile)
				break;
			else
				charsRead += rc;
		}
		
		if(!SystemCodeConverter::IsValidCharSequence(buffer, charsRead))
		{
			// As soon as we find mal-formed input we suspend optimized
			// reading so the stream will be more carefully analysed
			// by susbsequent read requests.  This also facilitiates
			// a little trick to return read bytes to the byte buffer
			// when throwing exceptions.
			if(m_rpDecoder->getInvalidCharAction()==CodeConverter::abort)
			{
				suspendDecodingOptimization((Byte*)buffer, charsRead*sizeof(CharType));
				throw MalformedInputException((const Byte*)buffer, charsRead*sizeof(CharType), m_rpDecoder.get());
			}
			else
			{
				suspendDecodingOptimization(0, 0);
				return Character(m_rpDecoder->getInvalidCharReplacement());
			}
		}
		else
		{
			return Character(buffer, charsRead);
		}
	}
	else
	{
		QC_DBG_ASSERT(charsRead == EndOfFile);
		return Character::EndOfFileCharacter;
	}
}

//==============================================================================
// InputStreamReader::readAndDecode
//
// Reads bytes from the InputStream and decodes them into characters
// using the internal character encoding mechanism.
//
// For efficiency, bytes are read into an internal buffer before being
// fed to the decoder.
//
// Returns the number of characters written into the passed buffer.
//
// If a non-atomic read operation is requested and there is insufficient
// room in the destination buffer to accept the next unicode character,
// as many characters as are available are filled and the remainder is stored in
// an internal multi-character sequence buffer.
//
// It is an error for this method to be called while there are characters
// in the internal character sequence buffer.
//
// Returns: Number of character positions read into the supplied pBuffer.
//          A return value of zero indicates either EOF, a non-blocking
//          call where our internal buffer is exhausted or an atomic
//          read that passed an insufficiently large buffer to hold the
//          multi-character sequence for a single unicode character.
//
// MT Note: m_lock must be held prior to calling
//==============================================================================
size_t InputStreamReader::readAndDecode(bool bBlocking, CharType* pBuffer,
                                        size_t bufLen, bool bAtomicRead)
{
	QC_DBG_ASSERT(m_charSeqLen == 0);
	if(!m_rpInputStream) throw IOException(QC_T("stream is closed"));

	size_t charsRead = 0;

	//
	// If the underlying byte stream is encoded identically to
	// our internal character encoding (which is quite likely,
	// especially when using UTF-8), then we can optimize by 
	// bypassing some of the buffering and conversion work.
	// However, if an encoding error is detected, we must revert to
	// standard conversion.
	//
	if(!m_bRequiresDecoding)
	{
		if(readDirect(pBuffer, bufLen, bAtomicRead, charsRead))
		{
			return charsRead;
		}
		else
		{
			charsRead = 0;
			QC_DBG_ASSERT(m_bRequiresDecoding);
		}
	}

	CharType* myBuffer = pBuffer;
	size_t myBufLen = bufLen;

	do
	{
		if(m_pNextByteFree > m_pNextByteAvailable)
		{
			//
			// If we get this far then we know that we have read at least one
			// character and that decoding is required so...
			// we have some decoding to do
			//

			CharType* pIntNext = myBuffer;
			CodeConverter::Result result;

			try
			{
				result = m_rpDecoder->decode(m_pNextByteAvailable,
				                             m_pNextByteFree,
											 (const Byte*&)m_pNextByteAvailable,
											 myBuffer,
											 myBuffer+myBufLen,
											 pIntNext);
			}
			catch(CharacterCodingException& /*e*/)
			{
				//
				// If no characters have been converted then we can just rethrow the
				// exception.  Otherwise we stifle it so that the app doesn't get
				// the error until the last possible moment - when it will be in
				// a better position to report the file offset of the error.
				//
				if(pIntNext == myBuffer)
				{
					throw;
				}
				result = CodeConverter::inputExhausted;
			}
				
			// make a note of how many characters were converted this time
			charsRead += (pIntNext - myBuffer);

			// If there is an encoding error and no characters could
			// be read then we must throw an exception
			// Note: under normal circumstances the Decoder will throw the
			// exception, we only get this situation when the decoder
			// has been told not to throw exceptions.
			if(charsRead == 0 && result == CodeConverter::error)
			{
				throw MalformedInputException(QC_T("encoding error"), m_rpDecoder.get());
			}

			//
			// If no characters have been read, and the converter is saying that
			// there was insufficient room to write the characters to our buffer,
			// then we must have a multi-character sequence.
			//
			// When multi-character sequences are read into a too-small buffer,
			// it is our job to save the remaining characters until the next read
			// operation (unless we are specifically asked not to due to the
			// user performing an atomic read).
			//
			else if(charsRead == 0 && result == CodeConverter::outputExhausted)
			{
				if(bAtomicRead)
				{
					//throw AtomicReadException(QC_T("input buffer too short for character sequence"));
					// Atomic read with too small buffer gives zero rc
					break;
				}
				else
				{
					//
					// Point the buffer at our character sequence overflow buffer and
					// increment the length of the buffer to be made available.
					// Todo: Currently this is done one byte at a time, but it would be better
					// to determine the required size imemdiately
					//
					myBuffer = m_pCharSeqNext = m_charSeqBuffer;
					m_charSeqLen = ++myBufLen;
					if(myBufLen > CharSeqBufferSize)
					{
						String errMsg = StringUtils::FromLatin1(StringUtils::Format("unable to decode byte sequence starting 0x%X", (unsigned int)*m_pNextByteAvailable));
						throw MalformedInputException(errMsg, m_rpDecoder.get());
					}
					continue;
				}
			}
			//
			// If the input buffer is exhausted and we are at EOF then
			// we have a malformed input situation.  However, we do not
			// automatically throw an exception, we must use the policy
			// of the Decoder.
			//
			else if(result == CodeConverter::inputExhausted && m_bAtEof)
			{
				if(m_rpDecoder->getInvalidCharAction() == CodeConverter::abort)
				{
					throw MalformedInputException(QC_T("premature EOF within multi-byte sequence"), m_rpDecoder.get());
				}
				else
				{
					CharType* pCurrentPos = pIntNext;
					CodeConverter::Result replRes = SystemCodeConverter::ToInternalEncoding(
						m_rpDecoder->getInvalidCharReplacement(),
						pCurrentPos, myBuffer+myBufLen, pIntNext);
					
					if(replRes == CodeConverter::ok)
					{
						charsRead += (pIntNext-pCurrentPos);
					}
					else if(replRes == CodeConverter::outputExhausted)
					{
						// oh dear, what a pickle.  We have received an eof
						// part-way through a multi-character sequence and the
						// replacement character cnanot be encoded into the
						// output buffer.  For an atomic read, all we can
						// do is return 0 and wait for it to return with a 
						// larger buffer.  For non-atomic reads we can make
						// use of the character overflow buffer
						if(bAtomicRead)
						{
							break;
						}
						else
						{
							if( (SystemCodeConverter::ToInternalEncoding(
								m_rpDecoder->getInvalidCharReplacement(),
								m_charSeqBuffer, m_charSeqBuffer+CharSeqBufferSize, pIntNext)) == CodeConverter::ok)
							{
								charsRead = m_charSeqLen = (pIntNext - m_charSeqBuffer);
								m_pCharSeqNext = m_charSeqBuffer;
							}
							else
							{
								throw MalformedInputException(QC_T("premature EOF within multi-byte sequence"), m_rpDecoder.get());
							}
						}
					}

					// The replacement char being returned is a surrogate
					// for the bytes remaining in the buffer.
					m_pNextByteAvailable = m_pNextByteFree;
				}
			}
			//
			// Other errors from conversion can wait until next time round
			//
		}

		if(charsRead == 0 && bBlocking && !m_bAtEof)
		{
			fillByteBuffer();
		}
	}
	while(charsRead == 0 && bBlocking && !(m_bAtEof && m_pNextByteFree == m_pNextByteAvailable));

	//
	// If we have resorted to using our internal character sequence buffer
	// then the return value is equal to the size of the buffer supplied
	// and the passed buffer is filled from our character sequence buffer
	//
	if(m_charSeqLen)
	{
		QC_DBG_ASSERT(!bAtomicRead);
		QC_DBG_ASSERT(m_charSeqLen == charsRead);
		QC_DBG_ASSERT(m_charSeqLen > bufLen);
		QC_DBG_ASSERT(m_charSeqBuffer == m_pCharSeqNext);
		::memcpy(pBuffer, m_pCharSeqNext, bufLen*sizeof(CharType));
		m_pCharSeqNext += bufLen;
		m_charSeqLen -= bufLen;
	}

	//
	// The return value is the number of positions used in the passed
	// buffer.  It may be zero, in which case one of the following
	// conditions must be true:
	// - non-blocking call and interna; buffer exhausted
	// - atomic call and passed buffer too small
	// - EOF reached and byte buffer exhausted
	QC_DBG_ASSERT(charsRead!=0 ||
		(bAtomicRead && bufLen<SystemCodeConverter::GetMaximumCharSequenceLength()) ||
		!bBlocking ||
		(m_bAtEof && m_pNextByteFree == m_pNextByteAvailable));

	return m_charSeqLen ? bufLen : charsRead;
}

//==============================================================================
// InputStreamReader::readDirect
//
// Private function that implements reading into an application-supplied buffer
// when the byte stream is encoded in the same way as QuickCPP internal strings.
//==============================================================================
bool InputStreamReader::readDirect(CharType* pBuffer, size_t bufLen, bool bAtomicRead, size_t& charsRead)
{
	// If no decoding is required (meaning that the external
	// byte stream returns characters in the same encoding as our
	// internal encoding) then we can simply read bytes
	// and copy them straight into the output buffer
	//
	// There are a few complications however:-
	// - Atomic read requests must be honoured
	// - Only integral numbers of CharType characters can be read
	// - We must handle invalid character sequences
	//
	// In general, a read from the underlying input stream
	// will result in either just enough, too few or too many
	// bytes for the client request to be fully satisfied.
	//
	// Too few bytes will result when
	// a) fewer bytes are read than the size of CharType.  This can be
	//    relieved with more read requests until sufficient bytes have been read.
	// b) less than one character sequence for an Atomic Read request.  This too can
	//    be addressed by making further reads.
	//
	// Too many bytes will result when
	// a) A non-integral number of bytes are read, but more than enough for
	//    one character sequence
	// b) A trailing, incomplete character sequence is read for a atomic read.
	// In both of these situations, the excess bytes are moved to the byte
	// overflow buffer, from where they are retrieved on the next read
	// request.
	//
	// We must not block the caller unnecessarily.  This means that, for a non-
	// atomic read, we must not perform a blocking read once a full CharType 
	// has been read.  For an atomic read, we must not perform a blocking read
	// once a complete Unicode character has been read.
	// 
	// Invalid encoding sequence
	// -------------------------
	// The behaviour of a Reader when it encounters an invalid encoding sequence
	// is determined by the CodeConverter.  Either an exception is thrown or the
	// invalid sequence generates a special replacement character.  In either case
	// bytes are not usually lost from the stream (they remain in the Reader's byte
	// buffer until read), and we need to replicate that behaviour here.
	//
	// This behaviour is almost impossible to achieve when reading data directly
	// into a buffer supplied by the application, so we play a little trick.  As soon
	// as an encoding error is detected we revert to standard decoding (the
	// direct read optimisation is disabled), and the bytes that have been read are
	// moved into the standard byte buffer.  To activate this, this routine simply
	// needs to return false.

	bool bContinueOptimizing = true;

	//
	// To enable recovery to the Byte buffer, we must limit the size of read 
	// operations to the size of the byte buffer that we are prepared to allocate
	// minus the overflow byte buffer size.
	//
	const static size_t maxDirectBufferSize = (ByteBufferSize - OverflowBufferSize);
	
	size_t bufferLenBytes = (bufLen*sizeof(CharType));

	if(bufferLenBytes > maxDirectBufferSize)
		bufferLenBytes = maxDirectBufferSize;

	Byte* const pByteBuffer = (Byte*)pBuffer;
	const Byte* const pByteBufferEnd = pByteBuffer + bufferLenBytes;
	size_t bytesRead=0;

	//
	// 1. If there are any bytes sitting in the overflow buffer, use those
	//    first.
	//
	const size_t numStoredBytes = (m_pNextByteFree - m_pNextByteAvailable);

	if(numStoredBytes)
	{
		bytesRead = (numStoredBytes > bufferLenBytes)
		          ? bufferLenBytes
		          : numStoredBytes;

		::memcpy((void*)pByteBuffer, m_pNextByteAvailable, bytesRead);
		m_pNextByteAvailable += bytesRead;

		//
		// If we have exausted the overflow buffer, reset it.
		//
		if(m_pNextByteAvailable == m_pNextByteFree)
		{
			m_pNextByteAvailable = m_pNextByteFree = m_pByteBuffer;
		}
	}

	//
	// 2. Attempt to read sufficient bytes into the provided (Character) buffer
	//
	// Sufficient bytes have been read when we have a CharType or (for atomic
	// reads) a Unicode character.
	//
	while(((charsRead = bytesRead / sizeof(CharType)) == 0) || 
	      (bAtomicRead && charsRead < SystemCodeConverter::GetCharSequenceLength(*pBuffer)))
	{
		//
		// Apparently we need to read some more.
		//
		// Do we have space in the passed buffer?  As the passed bufLen is at
		// least one (CharType), the answer to this must always be positive
		// except when processing an atomic read.
		//
		size_t byteRoom = (bufferLenBytes - bytesRead);
		if(byteRoom)
		{
			int numBytes = m_rpInputStream->read(pByteBuffer+bytesRead, pByteBufferEnd-(pByteBuffer+bytesRead));

			//
			// If we have reached EOF, we could be in the somewhat strange
			// situation of having read some bytes, but not a whole character.
			// In this case we'll treat it as an error (and lose the errant
			// bytes)
			//
			if(numBytes == InputStream::EndOfFile)
			{
				if(bytesRead)
				{
					bContinueOptimizing = false;
					break;
				}
				else
				{
					// An EOF when no bytes have been read is nice and clean
					// so we can return immediately.
					m_bAtEof = true;
					return true;
				}
			}
			else
			{
				bytesRead += numBytes;
			}
		}
		else
		{
			// Insufficient buffer space must mean an atomic read of a multi-
			// character sequence.
			QC_DBG_ASSERT(bAtomicRead);
			//
			// To ensure that we don't consume any bytes, set charsRead
			// to zero, thereby ensuring that any consumed bytes are
			// returned to the overflow buffer.
			//
			charsRead=0;
			break;
		}
	}

	//
	// Now we have perhaps read sufficient bytes.  If not, it must be
	// because our buffer was not large enough for a character sequence
	// or we got a premature eof.
	//
	// Having read some bytes, we now need to determine how many
	// should be kept, with the remainder going to the overflow buffer.
	// 
	const CharType* pBufferEnd = pBuffer+charsRead;

	//
	// For an atomic read, we could jump to the end of the 
	// character buffer and search back for the first sequence start character
	// to see if the entire sequence is present.  If not, the sequence
	// is discarded.  However, we also need to check the entire sequence
	// for validity, so we can combine these two steps.
	//
	const CharType* pNextSequence;

	//
	// If this isn't an atomic read, the buffer may start with the
	// remnants of a sequence.  There's not much we can do about this
	// except step over it looking for the first sequence start character.
	//
	CharType* pStart = pBuffer;

	if(!bAtomicRead)
	{
		while (pStart < pBufferEnd && !SystemCodeConverter::IsSequenceStartChar(*pStart))
		{
			++pStart;
		}
	}

	//
	// Test the encoded buffer and return a pointer to the next
	// sequence.  If the returned pointer is less that pBufferEnd
	// then we know we have an incomplete trailing sequence - which must
	// be pruned when processing an atomic read.
	//
	if(SystemCodeConverter::TestEncodedSequence(pStart, pBufferEnd, pNextSequence)
		== SystemCodeConverter::error)
	{
		bContinueOptimizing = false;
	}

	//
	// If the preceding stages determined an encoding error in the input stream
	// the boolean value `bContinueOptimizing` is set to false.  This signifies that
	// we are to return all read bytes into the Byte Buffer and suspend direct read
	// optimization for this Reader.
	//
	if(!bContinueOptimizing)
	{
		suspendDecodingOptimization(pByteBuffer, bytesRead);
		return false;
	}

	//
	// Strip any trailing character sequences from an atomic read
	//
	if(bAtomicRead)
	{
		pBufferEnd = pNextSequence;
	}

	charsRead = (pBufferEnd - pBuffer);

	//
	// Okay, now we are in a position to know which characters are going
	// to be returned to the caller (unless we have a sequence error).
	// The buffered bytes from pBufferEnd thru to pByteBuffer+bytesRead
	// are not going to be returned to the caller, so they need to 
	// be stored in the byte overflow buffer until next time.
	//
	
	const size_t overflowBytes = pByteBuffer+bytesRead-(Byte*)pBufferEnd;

	//
	// It's possible the byte overflow buffer already has some bytes
	// in it, and those bytes must sit behind the tranche that are being
	// replaced.  Logically, there must be room to insert these
	// bytes back into the buffer from whence they came, because we
	// won't have read any more bytes while still leaving bytes in the
	// overflow buffer.
	//
	if(overflowBytes)
	{
		if(m_pNextByteAvailable > m_pByteBuffer)
		{
			//
			// Bytes still exist in the overflow buffer.  We cannot have
			// read any from the input stream.
			//
			QC_DBG_ASSERT(m_pNextByteAvailable-overflowBytes >= m_pByteBuffer);
			::memcpy(m_pNextByteAvailable-overflowBytes, pBufferEnd, overflowBytes);
		}
		else
		{
			//
			// The overflow buffer is empty (and possibly non-existent)
			//
			if(!m_pByteBuffer)
			{
				m_byteBufferSize = OverflowBufferSize;
				m_pByteBuffer = new Byte[m_byteBufferSize];
				m_pNextByteFree = m_pNextByteAvailable = m_pByteBuffer;
			}
			QC_DBG_ASSERT(overflowBytes <= m_byteBufferSize);
			::memcpy(m_pNextByteFree, pBufferEnd, overflowBytes);
			m_pNextByteFree += overflowBytes;
		}
	}

	return true;
}

//==============================================================================
// InputStreamReader::SenseEncoding
//
/**
   A static helper function that attempts to guess the encoding used by an
   InputStream by checking the initial byte sequence for a Byte Order Mark (BOM).

   This function uses mark() and reset() to re-position the input stream to its
   original location, so an InputStream that supports these operations must be
   used.

   @param pInputStream the byte input stream to test
   @param BOMSize a return parameter containing the size of the BOM detected
   @throws NullPointerException if @c pInputStream is null.
   @throws IOException if @c pInputStream does not support the @c mark operation.
   @returns a String containing the encoding name
   @sa InputStream::mark()
*/
//==============================================================================
String InputStreamReader::SenseEncoding(InputStream* pInputStream, size_t& BOMSize)
{
	if(!pInputStream) throw NullPointerException();

	String encoding;

	// mark the input stream so that we can reset to the beginning
	// (we only every read 4 characters)
	pInputStream->mark(4);
	
	//
	// Read the first 4 bytes from the stream
	// and test to see if we have a byte order mark
	//
	Byte bom[4];	// Byte Order Mark

	// Taken from Appendix F, XML1.0, Second Edition:-
	//
	// 00 00 FE FF UCS-4, big-endian machine (1234 order) 
	// FF FE 00 00 UCS-4, little-endian machine (4321 order) 
	// 00 00 FF FE UCS-4, unusual octet order (2143) 
	// FE FF 00 00 UCS-4, unusual octet order (3412) 
	// FE FF ## ## UTF-16, big-endian 
	// FF FE ## ## UTF-16, little-endian 
	// EF BB BF    UTF-8 

	//
	// The notation ## is used to denote any byte value except that
	// two consecutive ##s cannot both be 00.
	//

	//
	// Note, even if we have been given an external encoding (such as from
	// a MIME header), we still need to check for an pass over the BOM if it is
	// present.  We just disregard what it tells us!
	//
	size_t bomBytesRead=0;
	while(bomBytesRead < sizeof(bom))
	{
		long rc = pInputStream->read(bom+bomBytesRead, sizeof(bom)-bomBytesRead);
		if(rc == InputStream::EndOfFile)
			break;
		else
			bomBytesRead += rc;
	}

	if(bomBytesRead ==4)
	{
		BOMSize=4;
		if(bom[0] == 0 && bom[1] == 0 && bom[2] == 0xFE && bom[3] == 0xFF)
			encoding = QC_T("UCS-4BE");
		else if(bom[0] == 0xFF && bom[1] == 0xFE && bom[2] == 0 && bom[3] == 0)
			encoding = QC_T("UCS-4LE");
		else if(bom[0] == 0 && bom[1] == 0 && bom[2] == 0xFF && bom[3] == 0xFE)
			encoding = QC_T("UCS-4-2143");
		else if(bom[0] == 0xFE && bom[1] == 0xFF && bom[2] == 0 && bom[3] == 0)
			encoding = QC_T("UCS-4-3412");
		else if(bom[0] == 0xFE && bom[1] == 0xFF && (bom[2] != 0 || bom[3] != 0))
		{
			encoding = QC_T("UTF-16BE");
			BOMSize=2;
		}
		else if(bom[0] == 0xFF && bom[1] == 0xFE && (bom[2] != 0 || bom[3] != 0))
		{
			encoding = QC_T("UTF-16LE");
			BOMSize=2;
		}
		else if(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)
		{
			encoding = QC_T("UTF-8");
			BOMSize=3;
		}
		else
		{
			BOMSize=0;
		}
	}
	pInputStream->reset();

	return encoding;
}

//==============================================================================
// InputStreamReader::getDecoder
//
/**
   Returns the CodeConverter used by this InputStreamReader to decode
   bytes into Unicode characters.
   @synchronized
*/
//==============================================================================
AutoPtr<CodeConverter> InputStreamReader::getDecoder() const
{
	return m_rpDecoder;
}

//==============================================================================
// InputStreamReader::suspendDecodingOptimization
//
// Private function to suspend the optimization of reading bytes directly
// into the application-supplied buffer.
//==============================================================================
void InputStreamReader::suspendDecodingOptimization(const Byte* pExtraBytes,
                                                    size_t extraLen)
{
	if(!m_bRequiresDecoding)
	{
		m_bRequiresDecoding = true;

		Byte* pBuffer = new Byte[ByteBufferSize];
		const size_t storedBytes = m_pNextByteFree-m_pNextByteAvailable;
		QC_DBG_ASSERT(storedBytes <= m_byteBufferSize);
		QC_DBG_ASSERT(storedBytes + extraLen <= ByteBufferSize);
		if(pExtraBytes && extraLen)
		{
			::memcpy(pBuffer, pExtraBytes, extraLen);
		}
		if(storedBytes)
		{
			::memcpy(pBuffer+extraLen, m_pNextByteAvailable, storedBytes);
		}
		delete [] m_pByteBuffer;
		m_pNextByteAvailable = m_pByteBuffer = pBuffer;
		m_pNextByteFree = m_pNextByteAvailable + storedBytes + extraLen;
		m_byteBufferSize = ByteBufferSize;
	}
}

QC_IO_NAMESPACE_END

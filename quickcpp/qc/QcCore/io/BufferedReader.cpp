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
// Class: BufferedReader
// 
/**
	@class qc::io::BufferedReader
	
	@brief A BufferedReader wraps another Reader object and provides
	       buffering as well as support for the mark() and reset() methods and
		   a readLine() method.

    When the BufferedReader is constructed, an internal buffer is created. The
	size of the internal buffer can be specified by using the overloaded
	constructor.

    As characters are read, the internal buffer is refilled as
	necessary from the contained Reader.  In this way i/o efficiency
	can be improved by translating multiple small read operations into one
	large one.
	
	To improve efficiency and avoid the needless copying of data, if the application
	performs a read request that is larger than the internal buffer, the internal
	buffer is empty and there is not a mark() operation outstanding, the internal
	buffer is bypassed and characters are read directly into the application's buffer.

    mark() and reset() are supported by buffering all data after a mark() operation
	until the @c readLimit has been exceeded.  If mark() is called with a @c readLimit
	that is larger than the internal buffer, the buffer is re-allocated to the 
	size requested.

    @mt
    The contained Reader is used as the lock object for synchronized methods.
*/
//=============================================================================


#include "BufferedReader.h"
#include "AtomicReadException.h"
#include "IOException.h"

#include "QcCore/base/SystemUtils.h"

QC_IO_NAMESPACE_BEGIN

using namespace util;

const size_t DefaultBufferSize = 4096;
const size_t MinBufferSize = 10;

//==============================================================================
// BufferedReader::BufferedReader
//
/**
   Constructs a BufferedReader using a default buffer size and @c pReader
   as the contained Reader.
   
   @param pReader the contained Reader
   @throws NullPointerException if @c pReader is null
*/
//==============================================================================
BufferedReader::BufferedReader(Reader* pReader) : 
	m_rpReader(pReader)
{
	if(!pReader) throw NullPointerException();
	m_rpLock = pReader->getLock();

	init(DefaultBufferSize);
}

//==============================================================================
// BufferedReader::BufferedReader
//
/**
   Constructs a BufferedReader with the specified buffer size and @c pReader
   as the contained Reader.

   @param pReader the contained Reader
   @param size the size of the internal character buffer
   @throws NullPointerException if @c pReader is null
*/
//==============================================================================
BufferedReader::BufferedReader(Reader* pReader, size_t size) : 
	m_rpReader(pReader)
{
	if(!pReader) throw NullPointerException();
	m_rpLock = pReader->getLock();

	init(size);
}

//==============================================================================
// BufferedReader::~BufferedReader
//
/**
   Destroys the internal buffer.  Any buffered data that has not been read
   is lost.
*/
//==============================================================================
BufferedReader::~BufferedReader()
{
	delete [] m_pBuffer; m_pBuffer=0;
}

//==============================================================================
// BufferedReader::init
//
// Perform common initialization
//==============================================================================
void BufferedReader::init(size_t bufSize)
{
	m_bufSize = bufSize;
	m_pBuffer = new CharType[m_bufSize];
	m_markPos = -1;
	m_pos = 0;
	m_count = 0;
	m_eof = false;
	m_bCRSeen = false;
}

//==============================================================================
// BufferedReader::mark
//
/**
   Marks the current position in the character stream.
   
   Subsequent reset() operations will attempt to re-establish the stream's 
   position to the marked position.  This is guaranteed to succeed so long
   as the application does not read more than @c readLimit character positions.
   
   When the @c readLimit is exceeded, the marked position is automatically
   invalidated, with the result that subsequent reset() operations will fail
   with an IOException.

   Only one mark position is maintained by the InputStream.  Further calls to mark()
   will establish a new mark position; reset() can only reset the position
   to the most recently established mark position.

   If the current buffer has less than @c readLimit character positions available
   then the buffer is re-organized or reallocated so that is can hold at least
   @c readLimit character positions forward from the current position.

   Note:  remember that a single Unicode character may occupy several character
   positions.

   @sa markSupported()
   @sa reset()
   @param readLimit specifies the minimum number of character positions that the
          BufferedReader must return before making the marked position invalid.
   @throws IOException if the Reader has been closed
   @synchronized
*/
//==============================================================================
void BufferedReader::mark(size_t readLimit)
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	if(!m_rpReader) throw IOException(QC_T("stream is closed"));

	size_t bufferAvailable = m_bufSize - m_pos;

	if(readLimit <= bufferAvailable)
	{
		m_markPos = m_pos;
	}
	else
	{
		const size_t charsRemaining = m_count - m_pos;
		m_markPos = 0;
		m_count = charsRemaining;

		if(m_bufSize >= readLimit)
		{
			// overlapping copy operation requires ::memmove
			::memmove(m_pBuffer, m_pBuffer+m_pos, charsRemaining*sizeof(CharType));

			m_pos = 0;
		}
		else
		{
			CharType* pNewBuf = new CharType[readLimit];

			::memcpy(pNewBuf, m_pBuffer+m_pos, charsRemaining*sizeof(CharType));

			delete [] m_pBuffer;
			m_pos = 0;
			m_pBuffer = pNewBuf;
			m_bufSize = readLimit;
		}
	}
}

//==============================================================================
// BufferedReader::markSupported
//
/**
   Tests whether the Reader supports the mark() operation, which it does.
   
   @sa mark()
   @sa reset()
   @returns always returns true for BufferedReader
   @synchronized
*/
//==============================================================================
bool BufferedReader::markSupported() const
{
	return true;
}

//==============================================================================
// BufferedReader::reset
//
// Reset the position to the last mark() operation.  If there is no mark 
// (due to no mark() call or the buffer being overwritten (due to readLimit
// being exceeded) then threow an IOException.
//==============================================================================
void BufferedReader::reset()
{
	QC_SYNCHRONIZED_PTR(m_rpLock)
	
	if(!m_rpReader) throw IOException(QC_T("stream is closed"));

	if(m_markPos == -1)
	{
		throw IOException(QC_T("unable to reset input stream, either no mark or readLimit exceeded"));
	}
	else
	{
		m_pos = m_markPos;
	}
}
	
//==============================================================================
// BufferedReader::close
//
// Close the input stream and release any system resources.
//==============================================================================
void BufferedReader::close()
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	if(m_rpReader)
	{
		m_rpReader->close();
		m_rpReader.release(); // this is our indicator that the stream is closed
	}
	delete [] m_pBuffer;
	m_pBuffer = 0;
	m_pos = 0;
	m_count = 0;
	m_markPos = -1;
}

//==============================================================================
// BufferedReader::read
//
// If the buffer is empty, the mark is not valid, and the requested length is
// at least as large as the buffer, then this method will read characters 
// directly from the underlying stream into the given buffer.
// Thus redundant BufferedReaders will not copy data unnecessarily.
//==============================================================================
long BufferedReader::read(CharType* pBuffer, size_t bufLen)
{
	SystemUtils::TestBufferIsValid(pBuffer, bufLen);
	
	QC_SYNCHRONIZED_PTR(m_rpLock)
	
	if(!m_rpReader) throw IOException(QC_T("stream is closed"));

	//
	// Optimization: by-pass the buffer if the buffer is exhausted
	// and the read length is at least as long as our buffer size
	// and we don'e have a valid mark position
	//
	// Notice that we use readAtomic() so that when (and if)
	// we do revert to use the buffer, we can guarantee that
	// the underlying Reader will be at a suitable position
	//
	if(m_pos == m_count && !m_eof)
	{
		if(m_markPos == -1 && bufLen >= m_bufSize)
		{
			int ret = m_rpReader->readAtomic(pBuffer, bufLen);
			m_eof = (ret == EndOfFile);
			return ret;
		}
		else
		{
			fillBuffer();
		}
	}

	//
	// The check must be repeated here because m_eof may be set by fillBuffer()
	//
	if(m_pos == m_count && m_eof)
	{
		return EndOfFile;
	}
	else
	{
		size_t charactersRemaining = m_count - m_pos;
		size_t charactersToRead = (charactersRemaining > bufLen) ? bufLen : charactersRemaining;
		QC_DBG_ASSERT(charactersToRead!=0);
		QC_DBG_ASSERT(m_pBuffer!=0);
		::memcpy(pBuffer, m_pBuffer+m_pos, charactersToRead*sizeof(CharType));
		m_pos += charactersToRead;
		return charactersToRead;
	}
}

//==============================================================================
// BufferedReader::readAtomic
//
//==============================================================================
long BufferedReader::readAtomic(CharType* pBuffer, size_t bufLen)
{
	SystemUtils::TestBufferIsValid(pBuffer, bufLen);
	
	QC_SYNCHRONIZED_PTR(m_rpLock)

	if(!m_rpReader) throw IOException(QC_T("stream is closed"));

	//
	// Optimization: by-pass the buffer if the buffer is exhausted
	// and the read length is at least as long as our buffer size
	// and we don'e have a valid mark position
	//
	if(m_pos == m_count && !m_eof)
	{
		if(m_markPos == -1 && bufLen >= m_bufSize)
		{
			long ret = m_rpReader->readAtomic(pBuffer, bufLen);
			m_eof = (ret == EndOfFile);
			return ret;
		}
		else
		{
			//
			// fllBuffer performs an atomic read and may block
			//
			fillBuffer();
		}
	}

	//
	// The check must be repeated here because m_eof may be set by fillBuffer()
	//
	if(m_pos == m_count && m_eof)
	{
		return EndOfFile;
	}
	else
	{
		//
		// If we can slip our entire buffer into the passed buffer then 
		// that's great because our buffer always holds an integral number of
		// unicode characters.  But if the passed buffer is too small, we have
		// a choice: either
		// i)  scan the buffer counting characters; or
		// ii) search back for a sequence start character
		// All internal encodings are guaranteed to support the recognition of
		// sequence start characters, so (ii) will be more efficient for any buffer
		// of a significant size. 
		//
		size_t charCount = m_count - m_pos;
		if(charCount > bufLen)
		{
			charCount = bufLen;
			do {--charCount;}
			while(charCount && !SystemCodeConverter::IsSequenceStartChar(*(m_pBuffer+m_pos+charCount)));

			size_t charSeqLen = SystemCodeConverter::GetCharSequenceLength(*(m_pBuffer+m_pos+charCount));
			if((charCount + charSeqLen) <= bufLen)
			{
				charCount += charSeqLen;
			}
		}

		//
		// It's quite possible that we are unable to read any characters because
		// there is insufficient buffer space to read an integral number of 
		// characters.  This is legal.
		//

		if(charCount)
		{
			::memcpy(pBuffer, m_pBuffer+m_pos, charCount*sizeof(CharType));
			m_pos += charCount;
		}
		return charCount;
	}
}

//==============================================================================
// BufferedReader::readAtomic
//
//==============================================================================
Character BufferedReader::readAtomic()
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	// No need to check for closed stream, fillBuffer() does that
	if(m_pos == m_count)
	{
		fillBuffer();
	}
	
	if(m_pos == m_count && m_eof)
	{
		return Character::EndOfFileCharacter;
	}
	else
	{
		size_t oldPos = m_pos;
		const CharType nextChar = m_pBuffer[m_pos];
		
		if(!SystemCodeConverter::IsSequenceStartChar(nextChar))
		{
			throw AtomicReadException(QC_T("not on character sequence boundary"));
		}

		// The buffer must contain a whole sequence because it is filled
		// using an atomic read
		Character ret(m_pBuffer+oldPos, (m_count - m_pos));
		m_pos += ret.length();
		return ret;
	}
}

//==============================================================================
// BufferedReader::readLine
//
/**
   Reads a line of text into the return parameter @c ret.  A line is considered to be
   terminated by any one of a line feed ('\\n'), a carriage return ('\r'), 
   a carriage return followed immediately by a linefeed or when the end of the
   character stream is reached.

   @returns the number of ::CharType characters read or Reader::EndOfFile 
   if the end of the character stream is reached before any characters
   have been read.

   @throws IOException if an I/O error occurs.
   @synchronized
*/
//==============================================================================
long BufferedReader::readLine(String& ret)
{
	ret.erase();
	
	QC_SYNCHRONIZED_PTR(m_rpLock)

	int nextChar;
	while( (nextChar = read()) != EndOfFile)
	{
		if(nextChar == '\r')
		{
			m_bCRSeen = true;
			break;
		}
		else if(nextChar == '\n')
		{
			if(!m_bCRSeen)
			{
				break;
			}
			m_bCRSeen = false;
		}
		else
		{
			m_bCRSeen = false;
			ret += CharType(nextChar);
		}
	}
	
	if(!ret.empty())
	{
		return ret.size();
	}
	else
	{
		return (nextChar == EndOfFile) ? EndOfFile : 0;
	}
}

//==============================================================================
// BufferedReader::fillBuffer
//
// Called whenever the input buffer is exhausted and needs replenishing.
//
// If there is space available at the end of the buffer then we only read
// that many characters (this preserves the buffer for mark/reset operations),
// otherwise the buffer is completely overwritten (which splats any pending reset())
//
// Note: in order to support readAtomic(), we only fill the buffer with atomic
//       character sequences.
//
// MT Note: m_lock must be held prior to calling
//==============================================================================
void BufferedReader::fillBuffer()
{
	if(!m_rpReader) throw IOException(QC_T("stream is closed"));

	// if we have already seen the end of the stream then forget about it
	if(m_eof)
	{
		return;
	}

	// we should only be called when all the available characters have been read
	QC_DBG_ASSERT(m_pos == m_count);
	QC_DBG_ASSERT(((long)m_bufSize - (long)m_count) >= 0);

	size_t bufferAvailable = m_bufSize - m_count;

	//
	// If the buffer is exhausted, then reset everything
	//
	// Note: There must be at least N character positions available in the
	// input buffer to make the read worthwhile, otherwise we treat it as though
	// the buffer is exhausted.  This is necessary to support Atomic read operations
	// where the input buffer must be at least as large as any expected sequence
	// of characters.
	//
	// If there is no mark/reset operation pending, then we are free to make use
	// of the whole buffer - but optimization means that we are unlikely to be
	// called in that case!
	//
	if(bufferAvailable < MinBufferSize || m_markPos == -1)
	{
		m_markPos = -1;
		m_pos = 0;
		m_count = 0;
		bufferAvailable = m_bufSize;
	}

	long charactersRead = m_rpReader->readAtomic(m_pBuffer+m_count, bufferAvailable);

	if(charactersRead == EndOfFile)
	{
		m_eof = true;
	}
	else
	{
		QC_DBG_ASSERT(charactersRead > 0);
		m_count += charactersRead;
	}
}

QC_IO_NAMESPACE_END

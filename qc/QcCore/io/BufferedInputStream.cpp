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
// Class: BufferedInputStream
// 
/**
	@class qc::io::BufferedInputStream
	
	@brief A BufferedInputStream wraps another InputStream object and provides
	       buffering as well as support for the mark() and reset() methods. 

    When the BufferedInputStream is constructed, an internal buffer is created. The
	size of the internal buffer can be specified by using the overloaded
	constructor.

    As bytes from the stream are read, the internal buffer is refilled as
	necessary from the contained input stream.  In this way i/o efficiency
	can be improved by translating multiple small read operations into one
	large one.
	
	To improve efficiency and avoid the needless copying of data, if the application
	performs a read request that is larger than the internal buffer, the internal
	buffer is empty and there is not a mark() operation outstanding, the internal
	buffer is bypassed and bytes are read directly into the application's buffer.

    mark() and reset() are supported by buffering all data after a mark() operation
	until the @c readLimit has been exceeded.  If mark() is called with a @c readLimit
	that is larger than the internal buffer, the buffer is re-allocated to the 
	size requested.
*/
//=============================================================================

#include "BufferedInputStream.h"
#include "IOException.h"

#include "QcCore/base/SystemUtils.h"
#include "QcCore/base/NullPointerException.h"

QC_IO_NAMESPACE_BEGIN

const size_t DefaultBufferSize = 4096;

//==============================================================================
// BufferedInputStream::BufferedInputStream
//
/**
   Constructs a BufferedInputStream with a default buffer size and @c pInputStream
   as the contained InputStream.  
   @throws NullPointerException if @c pInputStream is null.
*/
//==============================================================================
BufferedInputStream::BufferedInputStream(InputStream* pInputStream) : 
	m_rpInputStream(pInputStream)
{
	if(!pInputStream) throw NullPointerException();

	init(DefaultBufferSize);
}

//==============================================================================
// BufferedInputStream::BufferedInputStream
//
/**
   Constructs a BufferedInputStream using @c pInputStream as the 
   contained input stream and a buffer size of @c bufSize.  if @c bufSize
   is zero, the default buffer size is used.
   @throws NullPointerException if @c pInputStream is null.
*/
//==============================================================================
BufferedInputStream::BufferedInputStream(InputStream* pInputStream,
	size_t bufSize) : 
	m_rpInputStream(pInputStream)
{
	if(!pInputStream) throw NullPointerException();

	init(bufSize ? bufSize : DefaultBufferSize);
}

//==============================================================================
// BufferedInputStream::~BufferedInputStream
//
/**
   Destroys the internal buffer.  Any buffered data that has not been read
   is lost.  The contained InputStream is not closed by this operation.
*/
//==============================================================================
BufferedInputStream::~BufferedInputStream()
{
	delete [] m_pBuffer;
	m_pBuffer = 0;
}

//==============================================================================
// BufferedInputStream::init
//
// Private initialization routine.
//==============================================================================
void BufferedInputStream::init(size_t bufSize)
{
	m_bufSize = bufSize;
	m_pBuffer = new Byte[m_bufSize];
	m_markPos = -1;
	m_pos = 0;
	m_count = 0;
	m_eof = false;
}

//==============================================================================
// BufferedInputStream::mark
//
/**
   Marks the current position in the byte stream.
   
   Subsequent reset() operations will attempt to re-establish the stream's 
   position to the marked position.  This is guaranteed to succeed so long
   as the application does not read more than @c readLimit bytes.
   
   When the @c readLimit is exceeded, the marked position is automatically
   invalidated, with the result that subsequent reset() operations will fail
   with an IOException.

   Only one mark position is maintained by the InputStream.  Further calls to mark()
   will establish a new mark position; reset() can only reset the position
   to the most recently established mark position.

   If the current buffer has less than @c readLimit bytes available
   then the buffer is re-organized or reallocated so that is can hold at least
   @c readLimit bytes forward from the current position.

   @sa markSupported()
   @sa reset()
   @param readLimit specifies the minimum number of bytes that the BufferedInputStream
          must return before making the marked position invalid.
   @throws IOException if the input stream is closed
*/
//==============================================================================
void BufferedInputStream::mark(size_t readLimit)
{
	if(!m_rpInputStream) throw IOException(QC_T("stream is closed"));

	QC_DBG_ASSERT(m_pBuffer!=0);

	size_t bufferAvailable = m_bufSize - m_pos;
	if(readLimit <= bufferAvailable)
	{
		m_markPos = m_pos;
	}
	else
	{
		const size_t bytesRemaining = m_count - m_pos;
		m_markPos = 0;
		m_count = bytesRemaining;

		if(m_bufSize >= readLimit)
		{
			// overlapping copy operation requires ::memmove
			::memmove(m_pBuffer, m_pBuffer+m_pos, bytesRemaining);

			m_pos = 0;
		}
		else
		{
			Byte* pNewBuf = new Byte[readLimit];

			::memcpy(pNewBuf, m_pBuffer+m_pos, bytesRemaining);

			delete [] m_pBuffer;
			m_pos = 0;
			m_pBuffer = pNewBuf;
			m_bufSize = readLimit;
		}
	}
}

//==============================================================================
// BufferedInputStream::markSupported
//
/**
   Tests whether the BufferedInputStream supports the mark() operation, which
   it does.

   @sa mark()
   @sa reset()
   @returns true.
*/
//==============================================================================
bool BufferedInputStream::markSupported() const
{
	return true;
}

//==============================================================================
// BufferedInputStream::reset
//
/**
   Resets the position in the byte stream to a previously marked position.

   It is permitted to call reset() multiple times.  Each time it is called, the
   position will be reset to the position established by the most recent mark()
   operation.

   @throws IOException if mark() was not successfully called or the internal buffer
           has been exhausted (i.e. the @c readLimit specified in the mark()
		   call has been exceeded)
   @throws IOException if the InputStream is closed
   @sa mark()
*/
//==============================================================================
void BufferedInputStream::reset()
{
	if(!m_rpInputStream) throw IOException(QC_T("stream is closed"));

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
// BufferedInputStream::available
//
// Our implementation must return the number of bytes left in our buffer
// plus the number that the underlying InputStream has available.
//==============================================================================
size_t BufferedInputStream::available()
{
	if(m_rpInputStream)
	{
		long bytesRemaining = m_count - m_pos;
		QC_DBG_ASSERT(bytesRemaining >= 0);
		return bytesRemaining + m_rpInputStream->available();
	}
	else
	{
		return 0;
	}
}

//==============================================================================
// BufferedInputStream::close
//
/**
   Closes the contained input stream and releases any system resources
   associated with it.
*/
//==============================================================================
void BufferedInputStream::close()
{
	//
	// Free our resources
	//
	delete [] m_pBuffer;
	m_pBuffer = 0;
	m_pos = 0;
	m_count = 0;
	//
	//  Pass the request to the contained stream (if any)
	// and then release its reference
	//
	if(m_rpInputStream)
	{
		m_rpInputStream->close();
		m_rpInputStream.release();
	}
}

//==============================================================================
// BufferedInputStream::read
//
// If the buffer is empty, the mark is not valid, and the requested length is
// at least as large as the buffer, then this method will read characters 
// directly from the underlying stream into the given buffer.
// Thus redundant BufferedReaders will not copy data unnecessarily.
//==============================================================================
long BufferedInputStream::read(Byte* pBuffer, size_t bufLen)
{
	SystemUtils::TestBufferIsValid(pBuffer, bufLen);
	if(!m_rpInputStream) throw IOException(QC_T("stream is closed"));

	//
	// Optimization: by-pass the buffer if the buffer is exhausted
	// and the read length is at least as long as our buffer size
	// and we don't have a valid mark position
	//
	if(m_pos == m_count && !m_eof)
	{
		//
		// The mark can be reset if we have reached the end of our buffer - 
		// but not before
		//
		if((m_markPos == -1 || m_bufSize == m_count) && bufLen >= m_bufSize)
		{
			m_markPos = -1;
			int bytesRead = m_rpInputStream->read(pBuffer, bufLen);
			m_eof = (bytesRead == EndOfFile);
			return bytesRead;
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
		size_t bytesRemaining = m_count - m_pos;
		size_t bytesToRead = (bytesRemaining > bufLen) ? bufLen : bytesRemaining;
		QC_DBG_ASSERT(bytesToRead!=0);
		QC_DBG_ASSERT(m_pBuffer!=0);
		::memcpy(pBuffer, m_pBuffer+m_pos, bytesToRead);
		m_pos += bytesToRead;
		return bytesToRead;
	}
}

//==============================================================================
// BufferedInputStream::fillBuffer
//
// Called whenever the buffers are exhausted and need replenishing.
//
// If there is space available at the end of the buffer then we only read
// that many bytes (this simplifies the mark/reset operations), otherwise
// the buffer is completely overwritten (which splats any pending reset())
//==============================================================================
void BufferedInputStream::fillBuffer()
{
	// if we have already seen the end of the stream then forget about it
	if(m_eof)
	{
		return;
	}

	// we should only be called when the stream is open
	QC_DBG_ASSERT(m_rpInputStream);
	// we should only be called when all the available bytes have been read
	QC_DBG_ASSERT(m_pos == m_count);
	// and there should be a buffer allocated 
	QC_DBG_ASSERT(m_pBuffer!=0 && m_bufSize!=0);

	long bufferAvailable = m_bufSize - m_count;
	QC_DBG_ASSERT(bufferAvailable >= 0);

	//
	// If the buffer is exhausted, or there is no pending reset then
	// we are free to re-use the entire buffer. Otherwise we must
	// preserve the buffer in case the app calls reset().
	//
	if(bufferAvailable == 0 || m_markPos == -1)
	{
		m_markPos = -1;
		m_pos = 0;
		m_count = 0;
		bufferAvailable = m_bufSize;
	}

	int bytesRead = m_rpInputStream->read(m_pBuffer+m_count, bufferAvailable);
	if(bytesRead == EndOfFile)
	{
		m_eof = true;
	}
	else
	{
		QC_DBG_ASSERT(bytesRead > 0);
		m_count += bytesRead;
	}
}

QC_IO_NAMESPACE_END

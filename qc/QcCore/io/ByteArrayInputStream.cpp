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
// Class: ByteArrayInputStream
/**
	@class qc::io::ByteArrayInputStream
	
	@brief An input stream which uses a byte array as its input source.

	@sa ByteArrayOutputStream
*/
//==============================================================================

#include "ByteArrayInputStream.h"
#include "IOException.h"

#include "QcCore/base/NullPointerException.h"
#include "QcCore/base/SystemUtils.h"

QC_IO_NAMESPACE_BEGIN

//==============================================================================
// ByteArrayInputStream::ByteArrayInputStream
//
/**
   Constructs a ByteArrayInputStream with a copy of the byte array starting at
   @c pData for a length of @c dataLen.
   @throws NullPointerException if @c pData is null.
*/
//==============================================================================
ByteArrayInputStream::ByteArrayInputStream(const Byte* pData, size_t dataLen) :
	m_apBuffer(new Byte[dataLen]),
	m_bufSize(dataLen),
	m_pos(0),
	m_markPos(-1),
	m_bClosed(false)
{
	if(!pData) throw NullPointerException();

	::memcpy(m_apBuffer.get(), pData, dataLen);
}

//==============================================================================
// ByteArrayInputStream::close
//
//==============================================================================
void ByteArrayInputStream::close()
{
	m_markPos = -1;
	m_pos = m_bufSize = 0;
	delete [] m_apBuffer.release();
	m_bClosed = true;
}

//==============================================================================
// ByteArrayInputStream::mark
//
// Marks the current position in this stream. A subsequent call to the 
// reset method re-positions the stream at the last marked position so that 
// subsequent reads re-read the same bytes. 
//
// The readLimit has no effect for ByteArrayInputStreams, as the entire data
// buffer is always available until the stream is closed. 
//==============================================================================
void ByteArrayInputStream::mark(size_t /*readLimit*/)
{
	if(m_bClosed) throw IOException(QC_T("stream is closed"));

	m_markPos = m_pos;
}

//==============================================================================
// ByteArrayInputStream::reset
//
// Reset the position to the last mark() operation.  If there is no mark 
// (due to no mark() call then throw an IOException.
//==============================================================================
void ByteArrayInputStream::reset()
{
	if(m_markPos == -1)
	{
		throw IOException(QC_T("unable to reset input stream, no marked position"));
	}
	else
	{
		m_pos = m_markPos;
	}
}

//==============================================================================
// ByteArrayInputStream::markSupported
//
/**
   Tests whether the ByteArrayInputStream supports the mark() operation,
   which it does.
   
   @sa mark()
   @sa reset()
   @returns @c true for ByteArrayInputStream.
*/
//==============================================================================
bool ByteArrayInputStream::markSupported() const
{
	return true;
}

//==============================================================================
// ByteArrayInputStream::read
//
//==============================================================================
long ByteArrayInputStream::read(Byte* pBuffer, size_t bufLen)
{
	SystemUtils::TestBufferIsValid(pBuffer, bufLen);
	
	if(m_bClosed) throw IOException(QC_T("stream is closed"));

	const size_t bytesAvailable = (m_bufSize - m_pos);

	if(bytesAvailable)
	{
		const size_t count = (bufLen < bytesAvailable)
		                   ? bufLen
		                   : bytesAvailable;

		::memcpy(pBuffer, m_apBuffer.get()+m_pos, count);
		m_pos+=count;
		return count;
	}
	else
	{
		return EndOfFile;
	}
}

//==============================================================================
// ByteArrayInputStream::available
//
//==============================================================================
size_t ByteArrayInputStream::available()
{
	return (m_bufSize - m_pos);
}

QC_IO_NAMESPACE_END

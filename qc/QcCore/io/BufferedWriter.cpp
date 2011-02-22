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
/**
	@class qc::io::BufferedWriter
	
	@brief A BufferedWriter wraps another Writer object and provides
	       buffering. 

    When the BufferedWriter is constructed, an internal buffer is created. The
	size of the internal buffer can be specified by using the overloaded
	constructor.

    When characters are written to this Writer, they are copied into
	the internal buffer instead of being written directly to the contained Writer.
	When the internal buffer becomes full it is automatically flushed to
	the contained Writer.  In this way i/o efficiency
	may be improved by translating multiple small write operations into one
	large one.
	
	To further improve efficiency and avoid the needless copying of data, if the
	application	performs a write request that is larger than the internal buffer,
	the internal buffer is bypassed and characters are written directly to the
	contained Writer.

    @mt
    The contained Writer is used as the lock object for synchronized methods.
*/
//=============================================================================


#include "BufferedWriter.h"

#include "QcCore/base/NullPointerException.h"

QC_IO_NAMESPACE_BEGIN

const size_t DefaultBufferSize = 1024;

//==============================================================================
// BufferedWriter::BufferedWriter
//
/**
   Constructs a BufferedWriter using the default buffer size and @c pWriter
   as the contained Writer.
   
   @throws NullPointerException if @c pWriter is null.
*/
//==============================================================================
BufferedWriter::BufferedWriter(Writer* pWriter) :
	m_rpWriter(pWriter)
{
	if(!pWriter) throw NullPointerException();
	m_rpLock = pWriter->getLock();

	init(DefaultBufferSize);
}

//==============================================================================
// BufferedWriter::BufferedWriter
//
/**
   Constructs a BufferedWriter using a buffer size of @c bufSize and 
   @c pWriter as the contained Writer.  If @c bufSize is zero, the
   default buffer size is used.

   @throws NullPointerException if @c pWriter is null.
*/
//==============================================================================
BufferedWriter::BufferedWriter(Writer* pWriter, size_t bufSize) :
	m_rpWriter(pWriter)
{
	if(!pWriter) throw NullPointerException();
	m_rpLock = pWriter->getLock();

	init(bufSize ? bufSize : DefaultBufferSize);
}

//==============================================================================
// BufferedWriter::~BufferedWriter
//
/**
   Flushes and destroys the character buffer before destroying this
   BufferedWriter.
*/
//==============================================================================
BufferedWriter::~BufferedWriter()
{
	try
	{
		flush();
	}
	catch(...)
	{
	}
	delete [] m_pBuffer;
}

//==============================================================================
// BufferedWriter::init
//
// Private initialization function.
//==============================================================================
void BufferedWriter::init(size_t bufferSize)
{
	m_bufferSize = bufferSize;
	m_pBuffer = new CharType[m_bufferSize];
	m_used=0;
}

//==============================================================================
// BufferedWriter::flush
//
//==============================================================================
void BufferedWriter::flush()
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	flushBuffersImpl();
	m_rpWriter->flush();
}

//==============================================================================
// BufferedWriter::flushBuffers
//
/**
   The internal character buffer is written to the contained Writer without
   requesting that Writer to flush().

   @sa flush()
   @synchronized
*/
//==============================================================================
void BufferedWriter::flushBuffers()
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	flushBuffersImpl();
}

//==============================================================================
// BufferedWriter::flushBuffersImpl
//
// MT Note: m_rpLock must be held before calling this method.
//==============================================================================
void BufferedWriter::flushBuffersImpl()
{
	m_rpWriter->write(m_pBuffer, m_used);
	m_used=0;
}

//==============================================================================
// BufferedWriter::write
//
//==============================================================================
void BufferedWriter::write(const CharType* pStr, size_t len)
{
	if(!pStr) throw NullPointerException();
	
	QC_SYNCHRONIZED_PTR(m_rpLock)
	
	if(m_used + len > m_bufferSize)
	{
		flushBuffersImpl();
	}

	if(len >= m_bufferSize)
	{
		QC_DBG_ASSERT(m_used == 0);
		m_rpWriter->write(pStr, len);
	}
	else
	{
		QC_DBG_ASSERT(len + m_used <= m_bufferSize);
		::memcpy(m_pBuffer+m_used, pStr, len*sizeof(CharType));
		m_used+=len;
	}
}

//==============================================================================
// BufferedWriter::close
//
//==============================================================================
void BufferedWriter::close()
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	flushBuffersImpl();
	m_rpWriter->close();
	// force writes through to closed stream
	delete [] m_pBuffer; m_pBuffer = 0;
	m_bufferSize = 0; 
}

QC_IO_NAMESPACE_END

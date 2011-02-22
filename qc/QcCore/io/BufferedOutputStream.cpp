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
// Class: BufferedOutputStream
// 
/**
	@class qc::io::BufferedOutputStream
	
	@brief A BufferedOutputStream wraps another OutputStream object and provides
	       buffering. 

    When the BufferedOutputStream is created, an internal buffer is created. The
	size of the internal buffer can be specified by using the overloaded
	constructor.

    When bytes are written to the stream, they are copied into the internal
	buffer instead of being written directly to the contained output stream.
	When the internal buffer becomes full it is automatically flushed to
	the contained input stream.  In this way i/o efficiency
	may be improved by translating multiple small write operations into one
	large one.
	
	To further improve efficiency and avoid the needless copying of data, if the
	application	performs a write request that is larger than the internal buffer,
	the internal buffer is bypassed and bytes are written directly to the
	contained output stream.
*/
//=============================================================================


#include "BufferedOutputStream.h"
#include "IOException.h"

#include "QcCore/base/NullPointerException.h"

QC_IO_NAMESPACE_BEGIN

const size_t DefaultBufferSize = 1024;

//==============================================================================
// BufferedOutputStream::BufferedOutputStream
//
/**
   Constructs a BufferedOutputStream using a default buffer size and 
   @c pOutputStream as the contained output stream.
   @throws NullPointerException if @c pOutputStream is null.
*/
//==============================================================================
BufferedOutputStream::BufferedOutputStream(OutputStream* pOutputStream) :
	m_rpOutputStream(pOutputStream)
{
	if(!pOutputStream) throw NullPointerException();

	init(DefaultBufferSize);
}

//==============================================================================
// BufferedOutputStream::BufferedOutputStream
//
/**
   Constructs a BufferedOutputStream using @c pOutputStream as the 
   contained output stream and a buffer size of @c bufSize.
   @throws NullPointerException if @c pOutputStream is null.
*/
//==============================================================================
BufferedOutputStream::BufferedOutputStream(OutputStream* pOutputStream, size_t bufSize) :
	m_rpOutputStream(pOutputStream)
{
	if(!pOutputStream) throw NullPointerException();

	init(bufSize ? bufSize : DefaultBufferSize);
}

//==============================================================================
// BufferedOutputStream::~BufferedOutputStream
//
/**
   Writes out and destroys the internal buffer before destroying this
   BufferedOutputStream.
*/
//==============================================================================
BufferedOutputStream::~BufferedOutputStream()
{
	if(m_rpOutputStream)
	{
		try
		{
			writeBuffer();
		}
		catch(IOException& /*e*/)
		{
		}
	}
	freeBuffers();
}

//==============================================================================
// BufferedOutputStream::close
//
/**
   Any buffered data is written to the output stream before it is closed and
   any system resources associated with the output stream are released.

   Once an OutputStream is closed further calls to write(), flush() or
   flushBuffers() will result in an IOException being thrown. Further calls
   to close() are legal but have no effect.
   
   @throws IOException if an I/O error occurs.
*/
//==============================================================================
void BufferedOutputStream::close()
{
	if(m_rpOutputStream)
	{
		flushBuffers();
		freeBuffers();
		m_rpOutputStream->close();
		m_rpOutputStream.release();
	}
}

//==============================================================================
// BufferedOutputStream::init
//
//==============================================================================
void BufferedOutputStream::init(size_t bufferSize)
{
	m_bufferSize = bufferSize;
	m_pBuffer = new Byte[m_bufferSize];
	m_used=0;
}

//==============================================================================
// BufferedOutputStream::freeBuffers
//
//==============================================================================
void BufferedOutputStream::freeBuffers()
{
	delete [] m_pBuffer; m_pBuffer=0;
	m_used = m_bufferSize = 0;
}

//==============================================================================
// BufferedOutputStream::flush
//
//==============================================================================
void BufferedOutputStream::flush()
{
	flushBuffers();
	m_rpOutputStream->flush();
}

//==============================================================================
// BufferedOutputStream::flushBuffers
//
//==============================================================================
void BufferedOutputStream::flushBuffers()
{
	if(!m_rpOutputStream) throw IOException(QC_T("stream closed"));

	writeBuffer();
	m_rpOutputStream->flushBuffers();
}

//==============================================================================
// BufferedOutputStream::write
//
/**
   Writes an array of bytes to this output stream.

   Normally the BufferedOutputStream simply copies the bytes to its
   internal buffer, flushing the buffer to the contained OutputStream
   when it becomes full.

   In the situation where @c bufLen is larger than the internal buffer size,
   the internal buffer will first be flushed before passing this write call
   directly to the contained OutputStream.  By using this technique, the unnecessary
   copying of data is avoided.

   @param pBuffer pointer to the start of an array of bytes to be written
   @param bufLen length of the byte array
   @throws NullPointerException if @c pBuffer is null.
   @throws IOException if an I/O error occurs.
*/
//==============================================================================
void BufferedOutputStream::write(const Byte* pBuffer, size_t bufLen)
{
	if(!pBuffer) throw NullPointerException();
	if(!m_rpOutputStream) throw IOException(QC_T("stream closed"));

	if(m_used + bufLen > m_bufferSize)
	{
		QC_DBG_ASSERT(m_pBuffer!=0);
		// Write our buffer without flushing out the stream
		writeBuffer();
		QC_DBG_ASSERT(0 == m_used);
	}

	if(bufLen > m_bufferSize)
	{
		QC_DBG_ASSERT(0 == m_used);
		m_rpOutputStream->write(pBuffer, bufLen);
	}
	else
	{
		QC_DBG_ASSERT(m_pBuffer!=0);
		QC_DBG_ASSERT(bufLen + m_used <= m_bufferSize);
		::memcpy(m_pBuffer+m_used, pBuffer, bufLen);
		m_used+=bufLen;
	}
}

//==============================================================================
// BufferedOutputStream::writeBuffer
//
// Helper function to write our internal buffer
//==============================================================================
void BufferedOutputStream::writeBuffer()
{
	if(m_pBuffer && m_used)
	{
		m_rpOutputStream->write(m_pBuffer, m_used);
		m_used=0;
	}
}

QC_IO_NAMESPACE_END

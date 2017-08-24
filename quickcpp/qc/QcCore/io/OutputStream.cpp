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
// Class OutputStream
/**
	@class qc::io::OutputStream
	
	@brief Abstract base class with the ability to write bytes to a data sink.

    Concrete classes derived from OutputStream may be used 
    to write bytes to a wide range of data sinks including the console, a file
    or a network socket connection.
    
    Unlike the standard C++ ostream, this class does not concern itself with
    formatting, rather it limits itself to the writing out of data.  In this respect
    it is closer to the standard streambuf class - but does not have the
    added complication of supporting a read capability.

    OutputStreams only concern themselves with the writing of bytes - not characters.
    If the application uses characters (as most do) then an OutputStream can be
    wrapped by an OutputStreamWriter that knows how the OutputStream should be
    encoded.
*/
//==============================================================================

#include "OutputStream.h"

QC_IO_NAMESPACE_BEGIN

//==============================================================================
// OutputStream::close
//
/**
   Closes the output stream and releases system resources associated with
   the stream.  

   Once an OutputStream is closed further calls to write(), flush() or
   flushBuffers() will result in an IOException being thrown. Further calls
   to close() are legal but have no effect.
   
   @throws IOException if an I/O error occurs.
*/
//==============================================================================
void OutputStream::close()
{
}

//==============================================================================
// OutputStream::flush
//
/**
   Forces all buffered data to be written out and sent to the final data sink.

   @sa flushBuffers()
   @throws IOException if an I/O error occurs.
*/
//==============================================================================
void OutputStream::flush()
{
}

//==============================================================================
// OutputStream::flushBuffers
//
/**
   Forces all data buffers associated with this output stream to be written out.
   This may not cause the data to be sent to the final sink if the underlying
   sink maintains its own buffering scheme.

   @sa flush()
   @throws IOException if an I/O error occurs.
*/
//==============================================================================
void OutputStream::flushBuffers()
{
}

//==============================================================================
// OutputStream::write
//
/**
   Writes the single byte @c x to this output stream.
   @param x the byte to write
   @throws IOException if an I/O error occurs.
*/
//==============================================================================
void OutputStream::write(Byte x)
{
	write(&x, 1);
}

#ifdef QC_DOCUMENTATION_ONLY
//=============================================================================
//
// Documentation for pure virtual methods follows:
//
//=============================================================================

//==============================================================================
// OutputStream::write
//
/**
   Writes an array of bytes to this output stream.

   @param pBuffer pointer to the start of an array of bytes to be written
   @param bufLen length of the byte array
   @throws NullPointerException if @c pBuffer is null.
   @throws IOException if an I/O error occurs.
*/
//==============================================================================
void OutputStream::write(const Byte* pBuffer, size_t bufLen);

#endif  //QC_DOCUMENTATION_ONLY

QC_IO_NAMESPACE_END

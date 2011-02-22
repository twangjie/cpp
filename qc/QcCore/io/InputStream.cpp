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
// Class InputStream
/**
	@class qc::io::InputStream
	
	@brief Abstract base class with the ability to read bytes from a data source.

	Concrete classes derived from InputStream may be used to read bytes from a wide range
	of data sources including the console, a file or a network socket connection.

	Unlike the standard C++ istream, this class does not concern itself with
	parsing or decoding, it just deals with the reading of bytes of data.
	In this respect it is closer to the C++ streambuf class - but avoids the complications
	of buffering or having an output capability.
	
	InputStreams only concern themselves with the reading of bytes - not characters.
	If the application requires characters then an InputStream can be wrapped by an
	InputStreamReader that knows how the InputStream is encoded.

	It is easy to construct your own class derived from InputStream.  There are
	a small number of pure virtual methods for which a derived class must provide an
	implementation: available(), close() and read().

	The following example opens an InputStream to read from a network resource
	and wraps it in an InputStreamReader to convert the byte stream into
	Unicode characters:-

    @code
    try {
        const URL url(QC_T("http://www.google.com"));
        AutoPtr<InputStream> rpIS = url.openStream();
        // wrap the InputStream by an InputStreamReader to convert the
        // byte stream into Unicode characters
        AutoPtr<Reader> rpReader(new InputStreamReader(rpIS.get());
        CharType ch;
        while( (ch = rpReader->read()) != Reader::EndOfFile) {
            Console::cout() << ch;
        }
    }
    catch(Exception& e) {
        Console::cerr() << e.toString() << endl;
    }
    @endcode
*/
//==============================================================================

#include "InputStream.h"
#include "IOException.h"

QC_IO_NAMESPACE_BEGIN

//==============================================================================
// InputStream::mark
//
/**
   Marks the current position in the byte stream.
   
   Subsequent reset() operations will attempt to re-establish the stream's 
   position to the marked position.
   
   Supporting mark() implies that the InputStream must maintain an internal 
   buffer containing all the bytes read from the point at which mark() was
   called.  The size of this buffer is implementation dependent, but is guaranteed
   to hold at least @c readLimit bytes before it becomes full.  When the buffer
   limit is exceeded, the marked position is automatically invalidated, with the 
   result that subsequent reset() operations will fail.

   Only one mark position is maintained by the InputStream.  Further calls to mark()
   will establish a new mark position; reset() can only reset the position
   to the most recently established mark position.

   @param readLimit specifies the minimum number of bytes that the InputStream must
          return before making the marked position invalid.
   @throws IOException if the InputStream does not support the mark() operation.
   @throws IOException if the InputStream is closed.
   @sa markSupported()
   @sa reset()
*/
//==============================================================================
void InputStream::mark(size_t /*readLimit*/) 
{
	static const String err = QC_T("mark operation is not supported");
	throw IOException(err);
}

//==============================================================================
// InputStream::markSupported
//
//
/**
   Tests whether the InputStream supports the mark() operation.

   The base class contains an implementation that always returns false.

   @sa mark()
   @sa reset()
   @returns true if the InputStream supports the mark() operation; false otherwise
*/
//==============================================================================
bool InputStream::markSupported() const
{
	return false;
}

//==============================================================================
// InputStream::reset
//
/**
   Resets the position in the byte stream to a previously marked position.

   It is permitted to call reset() multiple times.  Each time it is called, the
   position will be reset to the position established by the most recent mark()
   operation.

   @sa mark()
   @throws IOException if mark() is not supported
   @throws IOException if the InputStream is closed.
   @throws IOException if mark() was not successfully called or the internal buffer
           has been exhausted (i.e. the @c readLimit specified in the mark()
		   call has been exceeded)
*/
//==============================================================================
void InputStream::reset()
{
	throw IOException(QC_T("reset operation is not supported"));
}

//==============================================================================
// InputStream::skip
//
/**
   Reads and discards @c n bytes.

   This is the equivalent to calling read() @c n times or until InputStream::EndOfFile
   is returned.

   @param n The number of bytes to skip.
   @throws IOException if an error occurs while reading from the byte stream
   @returns the number of bytes skipped.
*/
//==============================================================================
size_t InputStream::skip(size_t n)
{
	size_t count=0;
	while(count < n)
	{
		if(read() == EndOfFile)
			break;
		count++;
	}
	return count;
}

//==============================================================================
// InputStream::read
//
/**
   Reads and returns a single ::Byte or InputStream::EndOfFile.

   The return value is a signed integer.  This is so that all byte values (0-255)
   as well as InputStream::EndOfFile (-1) can be returned.

   @throws IOException if an error occurs while reading from the byte stream
   @returns The ::Byte read or InputStream::EndOfFile if the end of the
            byte stream has been reached.
*/
//==============================================================================
int InputStream::read()
{
	Byte buffer;
	if(read(&buffer, 1) == EndOfFile)
		return EndOfFile;
	else
		return (int)buffer;
}

//==============================================================================
// InputStream::available
//
/**
   Returns the number of bytes that can be read without blocking.

   Some data sources (notably network socket and pipe streams) may make
   bytes available at a rate that is slower than the application can read them.
   In this case calls to read() may block until at least one byte becomes 
   available.  This method may be used to avoid making blocking calls.
   
   Note, however, that the utility of this function is severely limited.  Some
   sub-classes (e.g. FileInputStream) always return zero from available() and zero
   is also returned when the stream is at the end.  For these reasons, it 
   is rarely appropriate for an application to loop waiting for a positive 
   return value from available().

   @returns the number of bytes that can be read without blocking
   @throws IOException if an I/O error occurs
*/
//==============================================================================
size_t InputStream::available()
{
	return 0;
}

//=============================================================================
// InputStream::close
//
/**
   Closes the InputStream.
   
   Once an InputStream is closed, all system resources associated with the stream are
   released, preventing any further read(), mark(), reset() or skip()
   operations.  Further calls to close() have no effect.
   
   @throws IOException if an I/O error occurs.
*/
//=============================================================================
void InputStream::close()
{
}

#ifdef QC_DOCUMENTATION_ONLY
//=============================================================================
//
// Documentation for pure virtual methods follows:
//
//=============================================================================

//==============================================================================
// InputStream::read
//
/**
   Reads up to @c bufLen bytes into the supplied buffer.

   @param pBuffer A pointer to the buffer into which the bytes will be copied.
          This must be capable of holding at least @c bufLen bytes.
   @param bufLen The maximum number of bytes to read into the passed buffer.  If 
          this exceeds the maximum value that can be represented by a long integer,
		  it is reduced to a value that can be so represented.

   @throws IllegalArgumentException if @c bufLen is zero
   @throws NullPointerException if @c pBuffer is null
   @throws IOException if an error occurs while reading from the byte stream
   @returns The number of bytes read or InputStream::EndOfFile if the
            end of the stream has been reached.
*/
//==============================================================================
long InputStream::read(Byte* pBuffer, size_t bufLen);

#endif //QC_DOCUMENTATION_ONLY

QC_IO_NAMESPACE_END

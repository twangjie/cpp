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
// Class Reader
/**
	@class qc::io::Reader
	
	@brief Abstract class for reading a Unicode character stream.

	Concrete sub-classes of Reader may be used to translate byte sequences from an
	InputStream into sequences of Unicode characters.  In this case,
	an instance of CodeConverter may be used to translate between bytes and Unicode
	characters.

	Depending on the configuration chosen, Unicode characters may be represented
	internally using 32-bit UCS-4 characters, 16-bit UTF-16 characters or
	8-bit UTF-8 characters.  See @encoding for further information.

	The Reader interface mirrors InputStream, but deals in characters where
	InputStream deals in bytes.

    @mt
    As shown on the inheritance graph, Reader @a derives from 
	SynchronizedObject, which gives it the ability to protect its internal
	state from concurrent access from multiple threads.  All public methods 
	are synchronized for safe concurrent access.
*/
//==============================================================================

#include "Reader.h"
#include "IOException.h"

#include "QcCore/base/Character.h"
#include "QcCore/base/NullPointerException.h"

QC_IO_NAMESPACE_BEGIN

//==============================================================================
// Reader::Reader
//
/**
   Default constructor.
*/
//==============================================================================
Reader::Reader() :
	m_rpLock(this, this)
{
}

//==============================================================================
// Reader::Reader
//
/**
   Constructor taking a SynchronizedObject which will be used as the lock
   for synchronized functions.

   @throws NullPointerException if @c pLockObject is null
*/
//==============================================================================
Reader::Reader(SynchronizedObject* pLockObject) :
	m_rpLock(this, pLockObject)
{
	if(!pLockObject) throw NullPointerException();
}

//=============================================================================
// Reader::close
//
/**
   Closes the Reader.
   
   Once a Reader is closed, all system resources associated with the Reader are
   released, preventing any further read(), mark(), reset() or skip()
   operations.  However, further calls to close() have no effect.

   @throws IOException if an I/O error occurs.
   @synchronized
*/
//=============================================================================
void Reader::close()
{
}

//==============================================================================
// Reader::mark
//
/**
   Marks the current position in the character stream.
   
   Subsequent reset() operations will attempt to re-establish the character stream's 
   position to the marked position.
   
   Supporting mark() implies that the Reader must maintain an internal character 
   buffer containing all the characters read from the point at which mark() was
   called.  The size of this buffer is implementation dependent, but is guaranteed
   to hold at least @c readLimit ::CharType characters before it becomes full.
   When the buffer limit is exceeded, the marked position is automatically invalidated,
   with the result that subsequent reset() operations will fail.

   Only one mark position is maintained by the Reader.  Any subsequent calls to mark()
   will establish a new mark position; reset() can only reset the stream position
   to the most recently established mark position.

   @sa markSupported()
   @sa reset()
   @param readLimit specifies the minimum number of ::CharType characters that
          the Reader must return before making the marked position invalid.
   @throws IOException if the Reader does not support the mark() operation.
   @throws IOException if the Reader has been closed.
   @synchronized
*/
//==============================================================================
void Reader::mark(size_t /*readLimit*/) 
{
	static const String err = QC_T("mark operation is not supported");
	throw IOException(err);
}

//==============================================================================
// Reader::markSupported
//
/**
   Tests whether the Reader supports the mark() operation.
   
   The base class contains an implementation that always returns false.

   @sa mark()
   @sa reset()
   @returns true if the Reader supports the mark() operation; false otherwise
   @synchronized
*/
//==============================================================================
bool Reader::markSupported() const
{
	return false;
}

//==============================================================================
// Reader::read
//
/**
   Reads and returns a single ::CharType character.

   Depending on whether ::CharType is a 32-bit value, reading a single ::CharType
   character may not be the same as reading a single Unicode character.
   See CharEncoding for a detailed explanation of Unicode character encoding
   within @QuickCPP.

   @sa readAtomic()
   @throws IOException if an error occurs while reading from the character stream
   @returns The ::CharType read or Reader::EndOfFile if the end of the
            character stream has been reached.
   @synchronized
*/
//==============================================================================
IntType Reader::read()
{
	CharType x;
	if(read(&x, 1) == EndOfFile)
	{
		return EndOfFile;
	}
	else
	{
		return (IntType)x;
	}
}

//==============================================================================
// Reader::reset
//
/**
   Resets the position in the character stream to a previously marked position.

   It is permitted to call reset() multiple times.  Each time it is called, the
   position will be reset to the position established by the most recent mark()
   operation.

   @sa mark()
   @throws IOException if mark() is not supported
   @throws IOException if the Reader is closed
   @throws IOException if mark() was not successfully called or the internal
           character buffer has been exhausted (i.e. the @c readLimit specified
		   in the mark() call has been exceeded)
   @synchronized
*/
//==============================================================================
void Reader::reset()
{
	throw IOException(QC_T("reset operation is not supported"));
}

//==============================================================================
// Reader::skip
//
/**
   Reads and discards @c n characters.

   This is the equivalent to calling read() @c n times or until Reader::EndOfFile
   is returned, which ever comes first.

   Note: this method skips up to @c n occurrences of ::CharType from the
   character stream.  This is not necessarily the same as skipping @c n
   Unicode characters.  The skipAtomic() method may be used to skip @c n
   Unicode characters.

   @sa skipAtomic()
   @param n The number of ::CharType positions to skip
   @throws IOException if an error occurs while reading from the character stream
   @returns the number of ::CharType positions skipped.
   @synchronized
*/
//==============================================================================
size_t Reader::skip(size_t n)
{
	size_t count=0;

	for(; count<n; count++)
	{
		if(read() == EndOfFile) break;
	}

	return count;
}

//==============================================================================
// Reader::skipAtomic
//
/**
   Reads and discards @c n Unicode characters;

   This is the equivalent to calling readAtomic() @c n times or until
   Character::EndOfFileCharacter is returned.

   @sa skip()
   @param n The number of integral Unicode character positions to skip
   @throws IOException if an error occurs while reading from the character stream
   @throws AtomicReadException if the character stream is not positioned on a
           Unicode character boundary
   @returns the number of Unicode characters skipped.
   @synchronized
*/
//==============================================================================
size_t Reader::skipAtomic(size_t n)
{
	size_t count=0;

	for(; count<n; count++)
	{
		if(readAtomic() == Character::EndOfFileCharacter) break;
	}

	return count;
}

//==============================================================================
// Reader::getLock
//
/**
   Returns a reference to the SynchronizedObject used for controlling
   access to synchronized methods.
*/
//==============================================================================
AutoPtr<SynchronizedObject> Reader::getLock() const
{
	// Note: As we are returning a AutoPtr, it is imperative that
	// our reference count is not zero if the lock is pointing
	// at this object
	SynchronizedObject* pLock = m_rpLock;
	QC_DBG_ASSERT((Reader*) pLock != this || getRefCount());
	return pLock;
}

#ifdef QC_DOCUMENTATION_ONLY
//=============================================================================
//
// Documentation for pure virtual methods follows:
//
//=============================================================================

//==============================================================================
// Reader::read
//
/**
   Reads up to @c bufLen characters into the supplied buffer.

   The ::CharType characters read into the supplied buffer may not make up
   an integral number of Unicode characters.  For example, in the 
   case where the internal character encoding is UTF-16, if the
   passed buffer has room for just one ::CharType, and the next Unicode character
   is higher than U+FFFF, then only the first half of the UTF-16 surrogate pair
   will be returned. The second half of the pair will be returned on the next read
   operation.

   @sa readAtomic(CharType*, size_t)
   @param pBuffer A pointer to the buffer into which the characters will be copied.
          This must be capable of holding at least @c bufLen ::CharType positions.
   @param bufLen The maximum number of ::CharType characters to read into the
          passed buffer.  If this exceeds the maximum value that can be represented
		  by a long integer, it is reduced to a value that can be so represented.

   @throws IllegalArgumentException if @c bufLen is zero
   @throws NullPointerException if @c pBuffer is null
   @throws IOException if an error occurs while reading from the character stream
   @returns The number of ::CharType characters read or Reader::EndOfFile if the
            end of the stream has been reached.
   @synchronized
*/
//==============================================================================
long Reader::read(CharType* pBuffer, size_t bufLen);

//==============================================================================
// Reader::readAtomic
//
/**
   Reads an integral number of Unicode characters into the supplied ::CharType
   buffer.

   Reads as many characters that are available and that will fit into
   the supplied ::CharType buffer.  Unicode characters that are encoded internally
   into multi-character sequences are either read in their entirety or not at all.

   A return value of zero indicates that the supplied buffer was not large
   enough to hold the multi-character sequence for one Unicode character.

   @sa readAtomic()
   @param pBuffer A pointer to the buffer into which the characters will be copied.
          This must be capable of holding at least @c bufLen ::CharType positions.
   @param bufLen The maximum number of ::CharType characters to read into the
          passed buffer.  If this exceeds the maximum value that can be represented
		  by a long integer, it is reduced to a value that can be so represented.

   @throws IllegalArgumentException if @c bufLen is zero
   @throws NullPointerException if @c pBuffer is null
   @throws AtomicReadException if the next ::CharType is not on a character
           sequence boundary (i.e. a non-atomic read operation has been performed
		   previously which resulted in an incomplete multi-character sequence being read)
   @throws IOException if an error occurs while reading from the character stream
   @returns The number of ::CharType characters read or Reader::EndOfFile if the
            end of the stream has been reached.
   @synchronized
*/
//==============================================================================
long Reader::readAtomic(CharType* pBuffer, size_t bufLen);

//==============================================================================
// Reader::readAtomic
//
/**
   Reads a single Unicode Character.

   The Character class is capable of representing all Unicode characters up
   to U+10FFFF.

   @sa read()
   @throws AtomicReadException if the next ::CharType is not on a character
           sequence boundary (i.e. a non-atomic read operation has been performed
		   previously which resulted in an incomplete multi-character sequence being read)
   @throws IOException if an error occurs while reading from the character stream
   @returns The Character read or Character::EndOfFileCharacter if the
            end of the stream has been reached. 
   @synchronized
*/
//==============================================================================
Character Reader::readAtomic();

#endif //QC_DOCUMENTATION_ONLY

QC_IO_NAMESPACE_END

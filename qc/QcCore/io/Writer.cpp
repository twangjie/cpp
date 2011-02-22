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
// Class Writer
/**
	@class qc::io::Writer
	
	@brief Abstract base class representing an output stream with the ability
	to write Unicode characters to a sink.

 	Concrete sub-classes of Writer may be used to translate Unicode characters
	into byte sequences which are then written to an OutputStream.  In this case,
	an instance of CodeConverter may be used to translate between Unicode
	characters and bytes.

	Depending on the configuration chosen, Unicode characters may be represented
	internally using 32-bit UCS-4 characters, 16-bit UTF-16 characters or
	8-bit UTF-8 characters.  See @encoding for further information.

	The Writer interface mirrors OutputStream, but deals in characters where
	OutputStream deals in bytes.

    @mt
    As shown on the inheritance graph, Writer @a derives from 
	SynchronizedObject, which gives it the ability to protect its internal
	state from concurrent access from multiple threads.  All public methods 
	are synchronized for safe concurrent access.
*/
//==============================================================================

#include "Writer.h"

#include "QcCore/base/NullPointerException.h"

QC_IO_NAMESPACE_BEGIN

//==============================================================================
// Writer::Writer
//
/**
   Default constructor.
*/
//==============================================================================
Writer::Writer() :
	m_rpLock(this, this)
{
}

//==============================================================================
// Writer::Writer
//
/**
   Constructor taking a SynchronizedObject which will be used as the lock
   for synchronized functions.

   @throws NullPointerException if @c pLockObject is null
*/
//==============================================================================
Writer::Writer(SynchronizedObject* pLockObject) :
	m_rpLock(this, pLockObject)
{
	if(!pLockObject) throw NullPointerException();
}

//==============================================================================
// Writer::write
//
/**
   Writes the single ::CharType character @c c.

   Depending on the character encoding being employed, a single
   ::CharType character may not represent a complete Unicode character.  In this
   case the characters from subsequent write operations will be used to complete
   the multi-character sequence.

   @param c the character to write.
   @throws IOException if an I/O error occurs.
   @sa write(const Character& ch)
   @synchronized
*/
//==============================================================================
void Writer::write(CharType c)
{
	write(&c, 1);
}

//==============================================================================
// Writer::write
//
/**
   Writes the Unicode character @c ch to the output stream.

   Depending on the character encoding being employed, a single Unicode
   character may be represented internally using a sequence of more than one
   ::CharType characters.  In this case, as many ::CharType characters as are required
   to encode the Unicode character are written.

   @param ch the Unicode character to write.
   @throws IOException if an I/O error occurs.
   @sa write(CharType c)
   @synchronized
*/
//==============================================================================
void Writer::write(const Character& ch)
{
	write(ch.data(), ch.length());
}

//==============================================================================
// Writer::write
//
/**
   Writes the sequence of ::CharType characters contained in the String @c str.

   @param str the String to write.
   @throws IOException if an I/O error occurs.
   @synchronized
*/
//==============================================================================
void Writer::write(const String& str)
{
	if(!str.empty())
	{
		write(str.data(), str.length());
	}
}

//==============================================================================
// Writer::flush
//
/**
   Flushes any output buffers before forcing the output to its final
   destination.

   @throws IOException if an I/O error occurs.
   @sa flushBuffers()
   @synchronized
*/
//==============================================================================
void Writer::flush()
{
}

//==============================================================================
// Writer::flushBuffers
//
/**
   If this Writer maintains an output buffer, the buffer is emptied and written
   to the output destination without requesting the final destination
   to flush the output.

   The base class implementation does nothing.

   @sa flush()
   @synchronized
*/
//==============================================================================
void Writer::flushBuffers()
{
}

//==============================================================================
// Writer::getLock
//
/**
   Returns a reference to the SynchronizedObject used for controlling
   access to synchronized methods.
*/
//==============================================================================
AutoPtr<SynchronizedObject> Writer::getLock() const
{
	// Note: As we are returning a AutoPtr, it is imperative that
	// our reference count is not zero if the lock is pointing
	// at this object
	SynchronizedObject* pLock = m_rpLock;
	QC_DBG_ASSERT((Writer*) pLock != this || getRefCount());
	return pLock;
}

#ifdef QC_DOCUMENTATION_ONLY
//=============================================================================
//
// Documentation for pure virtual methods follows:
//
//=============================================================================

//==============================================================================
// Writer::close
//
/**
   Closes the character stream, flushing it first.
   @synchronized
*/
//==============================================================================
void Writer::close();

//==============================================================================
// Writer::write
//
/**
   Writes an array of ::CharType characters.

   @param pStr a pointer to the first ::CharType character in the array
   @param len the number of ::CharType characters to write
   @throws IOException if an I/O error occurs.
   @synchronized
*/
//==============================================================================
void Writer::write(const CharType* pStr, size_t len);

#endif //QC_DOCUMENTATION_ONLY

QC_IO_NAMESPACE_END

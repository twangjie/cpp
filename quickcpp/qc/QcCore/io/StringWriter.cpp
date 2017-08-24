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
// Class: StringWriter
/**
	@class qc::io::StringWriter
	
	@brief A Writer which collects output characters in a buffer, which can then
	be used to create a String.
	@sa StringReader
*/
//==============================================================================

#include "StringWriter.h"
#include "IOException.h"

QC_IO_NAMESPACE_BEGIN

//==============================================================================
// StringWriter::StringWriter
//
/**
   Constructs a StringWriter with no initial size.
*/
//==============================================================================
StringWriter::StringWriter() :
	m_bClosed(false)
{
}

//==============================================================================
// StringWriter::StringWriter
//
/**
   Constructs a StringWriter with an initial size of @c initialSize.
*/
//==============================================================================
StringWriter::StringWriter(size_t initialSize) :
	m_buffer(initialSize),
	m_bClosed(false)
{
}

//==============================================================================
// StringWriter::close
//
/**
   Closes the StringWriter.

   The internal string buffer is not released so that it remains available for
   subsequent calls to toString().

   @synchronized
*/
//==============================================================================
void StringWriter::close()
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	m_bClosed = true;
}

//==============================================================================
// StringWriter::write
//
/**
   Writes the character buffer @c pBuf with a length of @c len

   @throws IOException if the StringWriter has been closed
   @synchronized
*/
//==============================================================================
void StringWriter::write(const CharType* pBuf, size_t len)
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	if(m_bClosed) throw IOException(QC_T("cannot write to a closed stream"));

	m_buffer.append(pBuf, len);
}

//==============================================================================
// StringWriter::toString
//
/**
   Creates a string from the internal character buffer.

   @returns A String containing all the characters that have been written to the
            StringWriter so far.
   @synchronized
*/
//==============================================================================
String StringWriter::toString() const
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	return String(m_buffer.data(), m_buffer.size());
}

QC_IO_NAMESPACE_END

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
// Class: FilterReader
/**
	@class qc::io::FilterReader
	
	@brief A base class that @a derives from Reader but also
	@a contains another Reader which is used as the input source.
	
    FilterReader and its sister class FilterWriter provide
	an extensible framework for building chains of character processing.

    The FilterReader class overrides all methods of Reader with 
	versions that pass requests to the contained Reader.
	Derived classes of FilterReader are expected to further override
	some of these methods to perform some useful function before the characters
	are returned to the caller (which may be yet another FilterReader).

    @note Derived classes that override the any of the read() or readAtomic()
	methods would be advised to override all the read() and readAtomic()
	methods in addition to the skip() and skipAtomic() methods.  Failure to do
	so may produce unexpected results as
	these methods would otherwise be delegated to the contained Reader - 
	perhaps bypassing the required logic.

    @mt
    The contained Reader is used as the lock object for synchronized methods.
*/
//==============================================================================

#include "FilterReader.h"

#include "QcCore/base/NullPointerException.h"

QC_IO_NAMESPACE_BEGIN

//==============================================================================
// FilterReader::FilterReader
//
/**
   Constructs a FilterReader using @c pReader as the contained Reader.

   @throws NullPointerException if @c pReader is null.
*/
//==============================================================================
FilterReader::FilterReader(Reader* pReader) :
	m_rpReader(pReader)
{
	if(!pReader) throw NullPointerException();
	m_rpLock = pReader->getLock();
}

//==============================================================================
// FilterReader::close
//
//==============================================================================
void FilterReader::close()
{
	m_rpReader->close();
}

//==============================================================================
// FilterReader::mark
//
//==============================================================================
void FilterReader::mark(size_t readLimit)
{
	m_rpReader->mark(readLimit);
}
     
//==============================================================================
// FilterReader::markSupported
//
/**
   Tests whether the Reader supports the mark() operation.
   
   The FilterReader class contains an implementation that passes the
   request to the contained Reader.

   @sa mark()
   @sa reset()
   @returns true if the contained Reader supports the mark() operation;
            false otherwise
*/
//==============================================================================
bool FilterReader::markSupported() const
{
	return m_rpReader->markSupported();
}

//==============================================================================
// FilterReader::read
//
//==============================================================================
IntType FilterReader::read()
{
	return m_rpReader->read();
}

//==============================================================================
// FilterReader::read
//
//==============================================================================
long FilterReader::read(CharType* pBuffer, size_t bufLen)
{
	return m_rpReader->read(pBuffer, bufLen);
}

//==============================================================================
// FilterReader::readAtomic
//
//==============================================================================
long FilterReader::readAtomic(CharType* pBuffer, size_t bufLen)
{
	return m_rpReader->readAtomic(pBuffer, bufLen);
}

Character FilterReader::readAtomic()
{
	return m_rpReader->readAtomic();
}

//==============================================================================
// FilterReader::reset
//
//==============================================================================
void FilterReader::reset()
{
	m_rpReader->reset();
}

//==============================================================================
// FilterReader::skip
//
//==============================================================================
size_t FilterReader::skip(size_t n)
{
	return m_rpReader->skip(n);
}

//==============================================================================
// FilterReader::skipAtomic
//
//==============================================================================
size_t FilterReader::skipAtomic(size_t n)
{
	return m_rpReader->skipAtomic(n);
}

//==============================================================================
// FilterReader::getReader
//
/**
   Returns a reference to the contained Reader.
*/
//==============================================================================
AutoPtr<Reader> FilterReader::getReader() const
{
	return m_rpReader;
}

QC_IO_NAMESPACE_END

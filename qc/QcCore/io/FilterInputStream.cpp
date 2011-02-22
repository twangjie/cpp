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
// Class: FilterInputStream
/**
	@class qc::io::FilterInputStream
	
	@brief A base class that @a derives from InputStream but also
	@a contains another InputStream which is used as the input source.
	
    FilterInputStream and its sister class FilterOutputStream provide
	an extensible framework for building chains of processing.

    The FilterInputStream class overrides all methods of InputStream with 
	versions that pass requests to the contained input stream.
	Derived classes of FilterInputStream are expected to further override
	some of these methods to perform some useful function before the data
	is returned to the caller (which may be yet another FilterInputStream).

    @note Derived classes that override the any of the read() methods
	would be advised to override all the read() methods in addition to the
	skip() method.  Failure to do so may produce unexpected results as
	these methods would otherwise be delegated to the contained InputStream - 
	perhaps bypassing the required logic.
*/
//==============================================================================

#include "FilterInputStream.h"

#include "QcCore/base/NullPointerException.h"

QC_IO_NAMESPACE_BEGIN

//==============================================================================
// FilterInputStream::FilterInputStream
//
/**
   Constructs a FilterInputStream using @c pInputStream as the contained
   input stream.

   This constructor is protected to prevent the accidental
   creation of instances of FilterInputStream.  It should
   only be necessary to create instances of @a derived classes
   which have been customized to perform some useful behaviour.

   @throws NullPointerException if @c pInputStream is null.
*/
//==============================================================================
FilterInputStream::FilterInputStream(InputStream* pInputStream) :
	m_rpInputStream(pInputStream)
{
	if(!pInputStream) throw NullPointerException();
}

void FilterInputStream::mark(size_t readLimit)
{
	m_rpInputStream->mark(readLimit);
}

bool FilterInputStream::markSupported() const
{
	return m_rpInputStream->markSupported();
}

void FilterInputStream::reset()
{
	m_rpInputStream->reset();
}

size_t FilterInputStream::available()
{
	return m_rpInputStream->available();
}

void FilterInputStream::close()
{
	m_rpInputStream->close();
}

int FilterInputStream::read()
{
	return m_rpInputStream->read();
}

long FilterInputStream::read(Byte* pBuffer, size_t bufLen)
{
	return m_rpInputStream->read(pBuffer, bufLen);
}

size_t FilterInputStream::skip(size_t n)
{
	return m_rpInputStream->skip(n);
}

//==============================================================================
// FilterInputStream::getInputStream
//
/**
   Returns the contained input stream.
*/
//==============================================================================
AutoPtr<InputStream> FilterInputStream::getInputStream() const
{
	return m_rpInputStream;
}

QC_IO_NAMESPACE_END

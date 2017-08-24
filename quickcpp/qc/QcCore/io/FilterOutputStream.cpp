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
// Class: FilterOutputStream
/**
	@class qc::io::FilterOutputStream
	
	@brief A base class that @a derives from OutputStream but also
	@a contains another OutputStream which is used as the output sink.
	
    FilterOutputStream and its sister class FilterInputStream provide
	an extensible framework for building chains of processing.

    The FilterOutputStream class overrides all methods of OutputStream with 
	versions that pass requests to the contained output stream.
	Derived classes of FilterOutputStream are expected to further override
	some of these methods to perform some useful function before the data
	is written out to the sink (which may be yet another FilterOutputStream).
*/
//==============================================================================

#include "FilterOutputStream.h"

#include "QcCore/base/NullPointerException.h"

QC_IO_NAMESPACE_BEGIN

//==============================================================================
// FilterOutputStream::FilterOutputStream
//
/**
   Constructs a FilterOutputStream using @c pOutputStream as the contained
   output stream.

   @throws NullPointerException if @c pOutputStream is null.
*/
//==============================================================================
FilterOutputStream::FilterOutputStream(OutputStream* pOutputStream) :
	m_rpOutputStream(pOutputStream)
{
	if(!pOutputStream) throw NullPointerException();
}

void FilterOutputStream::close()
{
	m_rpOutputStream->close();
}

void FilterOutputStream::flush()
{
	m_rpOutputStream->flush();
}

void FilterOutputStream::flushBuffers()
{
	m_rpOutputStream->flushBuffers();
}

void FilterOutputStream::write(Byte x)
{
	m_rpOutputStream->write(x);
}

void FilterOutputStream::write(const Byte* pBuffer, size_t bufLen)
{
	m_rpOutputStream->write(pBuffer, bufLen);
}

//==============================================================================
// FilterOutputStream::getOutputStream
//
/**
   Returns the contained output stream.
*/
//==============================================================================
AutoPtr<OutputStream> FilterOutputStream::getOutputStream() const
{
	return m_rpOutputStream;
}

QC_IO_NAMESPACE_END

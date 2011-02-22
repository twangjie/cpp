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
// Class: FilterWriter
// 
/**
	@class qc::io::FilterWriter
	
	@brief A base class that @a derives from Writer but also
	@a contains another Writer which is used as the output sink.
	
    FilterWriter and its sister class FilterReader provide
	an extensible framework for building chains of character processing.

    The FilterWriter class overrides all methods of Writer with 
	versions that pass requests to the contained Writer.
	Derived classes of FilterWriter are expected to further override
	some of these methods to perform some useful function before the characters
	are written out to the sink (which may be yet another FilterWriter).

    @mt
    The contained Writer is used as the lock object for synchronized methods.
*/
//==============================================================================

#include "FilterWriter.h"

#include "QcCore/base/NullPointerException.h"

QC_IO_NAMESPACE_BEGIN

//==============================================================================
// FilterWriter::FilterWriter
//
/**
   Construct a FilterWriter with @c pWriter as the contained Writer.

   @throws NullPointerException if pWriter is null.
*/
//==============================================================================
FilterWriter::FilterWriter(Writer* pWriter) :
	m_rpWriter(pWriter)
{
	if(!pWriter) throw NullPointerException();
	m_rpLock = pWriter->getLock();
}
	
void FilterWriter::close()
{
	m_rpWriter->close();
}

void FilterWriter::flush()
{
	m_rpWriter->flush();
}

void FilterWriter::flushBuffers()
{
	m_rpWriter->flushBuffers();
}

void FilterWriter::write(const CharType* pStr, size_t len)
{
	m_rpWriter->write(pStr, len);
}

void FilterWriter::write(CharType c)
{
	m_rpWriter->write(c);
}

void FilterWriter::write(const Character& ch)
{
	m_rpWriter->write(ch);
}

void FilterWriter::write(const String& str)
{
	m_rpWriter->write(str);
}

//==============================================================================
// FilterWriter::getWriter
//
/**
   Returns the Writer contained by this FilterWriter.
*/
//==============================================================================
AutoPtr<Writer> FilterWriter::getWriter() const
{
	return m_rpWriter;
}

QC_IO_NAMESPACE_END

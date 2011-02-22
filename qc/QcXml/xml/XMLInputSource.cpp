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

#include "XMLInputSource.h"

#include "QcCore/base/NullPointerException.h"

QC_XML_NAMESPACE_BEGIN

XMLInputSource::XMLInputSource()
{
}

XMLInputSource::XMLInputSource(InputStream* pInputStream) :
	m_rpInputStream(pInputStream)
{
	if(!pInputStream) throw NullPointerException();
}

XMLInputSource::XMLInputSource(Reader* pReader) :
	m_rpReader(pReader)
{
	if(!pReader) throw NullPointerException();
}

XMLInputSource::XMLInputSource(const String& systemId) :
	m_systemId(systemId)
{
}

AutoPtr<InputStream> XMLInputSource::getInputStream() const
{
	return m_rpInputStream;
}

AutoPtr<Reader> XMLInputSource::getReader() const
{
	return m_rpReader;
}

String XMLInputSource::getSystemId() const
{
	return m_systemId;
}

String XMLInputSource::getEncoding() const
{
	return m_encoding;
}

void XMLInputSource::setEncoding(const String& encoding)
{
	m_encoding = encoding;
}

String XMLInputSource::getPublicId() const
{
	return m_publicId;
}

void XMLInputSource::setSystemId(const String& systemId)
{
	m_systemId = systemId;
}

void XMLInputSource::setPublicId(const String& publicId)
{
	m_publicId = publicId;
}

void XMLInputSource::setReader(Reader* pReader)
{
	m_rpReader = pReader;
}

void XMLInputSource::setInputStream(InputStream* pInputStream)
{
	m_rpInputStream = pInputStream;
}

QC_XML_NAMESPACE_END

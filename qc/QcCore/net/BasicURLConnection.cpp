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

#include "BasicURLConnection.h"

QC_NET_NAMESPACE_BEGIN

//==============================================================================
// BasicURLConnection::BasicURLConnection
//
//==============================================================================
BasicURLConnection::BasicURLConnection(const URL& url) :
	URLConnection(url),
	m_rpResponseHeaders(new MimeHeaderSequence)
{
}

//==============================================================================
// BasicURLConnection::setHeaderField
//
//==============================================================================
void BasicURLConnection::setHeaderField(const String& key, const String& value)
{
	m_rpResponseHeaders->setHeaderExclusive(key, value);
}

//==============================================================================
// BasicURLConnection::getHeaderField
//
//==============================================================================
String BasicURLConnection::getHeaderField(const String& name)
{
	getInputStream();
	return m_rpResponseHeaders->getHeader(name);
}

//==============================================================================
// BasicURLConnection::getHeaderField
//
//==============================================================================
String BasicURLConnection::getHeaderField(size_t index)
{
	getInputStream();
	return m_rpResponseHeaders->getHeader(index);
}

//==============================================================================
// BasicURLConnection::getHeaderFieldKey
//
//==============================================================================
String BasicURLConnection::getHeaderFieldKey(size_t index)
{
	getInputStream();
	return m_rpResponseHeaders->getHeaderKey(index);
}

//==============================================================================
// BasicURLConnection::getHeaderFieldCount
//
//==============================================================================
size_t BasicURLConnection::getHeaderFieldCount()
{
	getInputStream();
	return m_rpResponseHeaders->size();
}

QC_NET_NAMESPACE_END

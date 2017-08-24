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

#include "MimeHeaderSequence.h"

#include "QcCore/base/Tracer.h"
#include "QcCore/base/StringUtils.h"

#include <functional>
#include <algorithm>

QC_NET_NAMESPACE_BEGIN

struct KeyEquals : std::binary_function<std::pair<String, String>, String, bool>
{
	bool operator()(const std::pair<String, String>& lhs, const String& rhs) const
	{
		return (StringUtils::CompareNoCase(lhs.first, rhs) == 0);
	}
};

//==============================================================================
// MimeHeaderSequence::getHeader
//
//==============================================================================
String MimeHeaderSequence::getHeader(const String& key) const
{
	int i = findHeader(key);
	if(i == -1)
	{
		return String();
	}
	else
	{
		return m_headerFields[i].second;
	}
}

//==============================================================================
// MimeHeaderSequence::getHeader
//
//==============================================================================
String MimeHeaderSequence::getHeader(size_t index) const
{
	if(index<m_headerFields.size())
	{
		return m_headerFields[index].second;
	}
	else
	{
		return String();
	}
}

//==============================================================================
// MimeHeaderSequence::getHeaderKey
//
//==============================================================================
String MimeHeaderSequence::getHeaderKey(size_t index) const
{
	if(index<m_headerFields.size())
	{
		return m_headerFields[index].first;
	}
	else
	{
		return String();
	}
}

//==============================================================================
// MimeHeaderSequence::size
//
//==============================================================================
size_t MimeHeaderSequence::size() const
{
	return m_headerFields.size();
}

//==============================================================================
// MimeHeaderSequence::setHeader
//
// Sets a header field, overwriting any existing value
//==============================================================================
void MimeHeaderSequence::setHeaderExclusive(const String& key, const String& value)
{
	removeAllHeaders(key);
	insertHeader(key, value);
}

//==============================================================================
// MimeHeaderSequence::setHeaderIfAbsent
//
//==============================================================================
bool MimeHeaderSequence::setHeaderIfAbsent(const String& key, const String& value)
{
	if(findHeader(key) == -1)
	{
		insertHeader(key, value);
		return true;
	}
	else
	{
		return false;
	}
}

//==============================================================================
// MimeHeaderSequence::findHeader
//
//==============================================================================
int MimeHeaderSequence::findHeader(const String& key) const
{
	for(size_t i=0; i<m_headerFields.size(); i++)
	{
		if(StringUtils::CompareNoCase(m_headerFields[i].first, key)==0)
		{
			return i;
		}
	}
	return -1;
}

//==============================================================================
// MimeHeaderSequence::containsHeader
//
//==============================================================================
bool MimeHeaderSequence::containsHeader(const String& key) const
{
	return (findHeader(key) != -1);
}

//==============================================================================
// MimeHeaderSequence::insertHeader
//
//==============================================================================
void MimeHeaderSequence::insertHeader(const String& key, const String& value)
{
	m_headerFields.push_back(std::make_pair(key, value));
}

//==============================================================================
// MimeHeaderSequence::removeAllHeaders
//
//==============================================================================
void MimeHeaderSequence::removeAllHeaders(const String& key)
{
	HeaderFieldVector::iterator last = 
		std::remove_if(m_headerFields.begin(),
		               m_headerFields.end(), 
		               std::bind2nd(KeyEquals(), key));

	m_headerFields.erase(last, m_headerFields.end());
}

//==============================================================================
// MimeHeaderSequence::clear
//
//==============================================================================
void MimeHeaderSequence::clear()
{
	m_headerFields.clear();
}

//==============================================================================
// MimeHeaderSequence::writeHeaders
//
// Writes all the headers to the supplied Writer.
//==============================================================================
void MimeHeaderSequence::writeHeaders(Writer* pWriter)
{
	if(!pWriter) throw NullPointerException();
	const String sep = QC_T(": ");
	const String CRLF = QC_T("\r\n");

	Tracer::Trace(Tracer::Net, Tracer::Low, QC_T("writing headers:"));

	for(HeaderFieldVector::iterator i=m_headerFields.begin();
		i != m_headerFields.end(); ++i)
	{
		String header = (*i).first + sep + (*i).second; 

		Tracer::Trace(Tracer::Net, Tracer::Low, header);

		pWriter->write(header + CRLF);
	}
}

QC_NET_NAMESPACE_END

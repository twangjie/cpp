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

#include "MIMEType.h"

#include "QcCore/base/StringUtils.h"
#include "QcCore/base/UnicodeCharacterType.h"

QC_UTIL_NAMESPACE_BEGIN

MIMEType::MIMEType(const String& rawdata)
{
	parseContentTypeHeader(rawdata);
}

//
// Parse the raw string into the MIME Type consituents.
//
// The raw string is what you would expect to see in a "Content-Type"
// header field as described in RFC 2045.  It is not the intention
// (as of writing) to adhere religiously to this specification nor to 
// complain if some of the values fall outside of the range specified
// by the RFC.  We just need to extract the relevant tokens and let the
// calling application decide what should be done with it.
//
// Quoted from RFC 2045:-
//
//	In the Augmented BNF notation of RFC 822, a Content-Type header field
//   value is defined as follows:
//
//   content := "Content-Type" ":" type "/" subtype
//              *(";" parameter)
//              ; Matching of media type and subtype
//              ; is ALWAYS case-insensitive.
//
// [..snip]
//
//   parameter := attribute "=" value
//
//   attribute := token
//                ; Matching of attributes
//                ; is ALWAYS case-insensitive.
//
//   value := token / quoted-string
//
//   token := 1*<any (US-ASCII) CHAR except SPACE, CTLs,
//               or tspecials>
//
//   tspecials :=  "(" / ")" / "<" / ">" / "@" /
//                 "," / ";" / ":" / "\" / <">
//                 "/" / "[" / "]" / "?" / "="
//                 ; Must be in quoted-string,
//                 ; to use within parameter values
//

//
// So, to summarise, we have type "/" subtype (parameter*)
// E.g. text/plain; charset=us-ascii
//
// The BNF notation above is the opposite to what we normally expect
// but it is clear that multiple parameters are separated by ";".
//
bool MIMEType::parseContentTypeHeader(const String& contentType)
{
	// find the type/subtype separator
	size_t pos = contentType.find('/', 0);
	if(pos == String::npos)
	{
		return false;
	}

	m_type = StringUtils::StripWhiteSpace(contentType.substr(0, pos),
	                                      StringUtils::both);

	size_t startPos = pos+1;

	// find the end of subtype ";"
	pos = contentType.find(';', startPos);
	if(pos == String::npos)
	{
		// if there is no ";" then we have no parameters
		m_subType = StringUtils::StripWhiteSpace(contentType.substr(startPos),
	                                             StringUtils::both);
		return true;
	}
	else
	{
		m_subType = contentType.substr(startPos, pos-startPos);
		// skip over ";"
		startPos = pos+1;
	}

	while(true)
	{
		// locate "="
		pos = contentType.find('=', startPos);
		if(pos == String::npos || pos == contentType.length())
		{
			// could be an error, but we are not trying to police 
			// the validity of the MIME type, just trying to 
			// extract something useful.
			break;
		}

		String paramName = StringUtils::StripWhiteSpace(contentType.substr(startPos, pos-startPos),
		                                                StringUtils::both);

		startPos = pos+1;
		while(startPos < contentType.size() && UnicodeCharacterType::IsSpace(contentType[startPos]))
		{
			startPos++;
		}

		if(startPos == contentType.size())
		{
			//error again but...
			break;
		}

		//
		// The value may be a quoted string or sinple ascii text
		// quotes must be '"'
		//
		CharType x = contentType[startPos];
		if(x == '"')
		{
			if(startPos+1 == contentType.size())
				break; // yet another error!

			pos = contentType.find('"', startPos+1);
			if(pos != String::npos)
			{
				String value = contentType.substr(startPos+1, pos-(startPos+1));
				addParameter(paramName, value);
				startPos = pos+1;
			}
			else
			{
				break; // yet another error!
			}
		}
		else
		{
			// okay, single token is what we want (ie delimited by white-space)
			String value;
			while(startPos < contentType.size() && !UnicodeCharacterType::IsSpace(contentType[startPos]))
			{
				value.append(1, contentType[startPos]);
				startPos++;
			}
			addParameter(paramName, value);
		}
	}

	return true;
}

//==============================================================================
// MIMEType::getParameter
//
/**
   Brief

   Description
*/
//==============================================================================
String MIMEType::getParameter(const String& name) const
{
	ParamMap::const_iterator iter=m_paramMap.find(name);
	if(iter!=m_paramMap.end())
	{
		return (*iter).second;
	}
	else
	{
		return String();
	}
}

//==============================================================================
// MIMEType::getType
//
/**
   Brief

   Description
*/
//==============================================================================
const String& MIMEType::getType() const
{
	return m_type;
}

const String& MIMEType::getSubType() const
{
	return m_subType;
}

//==============================================================================
// MIMEType::addParameter
//
//==============================================================================
void MIMEType::addParameter(const String& name, const String& value)
{
	m_paramMap[name] = value;
}

QC_UTIL_NAMESPACE_END

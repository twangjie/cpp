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
/**
	@class qc::net::URLEncoder
	
	@brief A class module containing static methods for encoding strings
	into a form that can be transmitted as a URL.
*/
//==============================================================================

#include "URLEncoder.h"

#include "QcCore/base/ArrayAutoPtr.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/base/StringIterator.h"

#include <stdio.h>  
#include <ctype.h>  
#include <algorithm>  

QC_NET_NAMESPACE_BEGIN

//==============================================================================
// URLEncoder::Encode
//
/**
   Converts a Unicode string into the MIME @c x-www-form-urlencoded format.

   To convert a String, each Unicode character is examined in turn: 

   - The ASCII characters 'a' through 'z', 'A' through 'Z', '0' through '9', 
      and ".", "-", "*", "_" remain the same. 
   - The space character ' '(U+20) is converted into a plus sign '+'. 
   - All other characters are converted into their UTF-8 equivalent and the
     subsequent bytes are encoded as the 3-byte string "%xy", 
     where xy is the two-digit hexadecimal representation of the byte. 
*/
//==============================================================================
String URLEncoder::Encode(const String& uri)
{
	const char included[] = {'.', '-', '*', '_'};
	const char* pInclEnd = included + sizeof(included);

	String sRet;

	//
	// 1. Convert the URL to UTF-8.  Of course, it may already be encoded
	//    as UTF-8 if that is our internal string encoding.
	//
	ByteString utf8 = StringUtils::ToUTF8(uri);

	sRet.reserve(utf8.length());
	const Byte* pEnd =  (const Byte*)utf8.data() + utf8.length();

	for(const Byte* pByte=(const Byte*)utf8.data(); pByte<pEnd; ++pByte)
	{
		if((isalnum(*pByte)
			|| std::find(included, pInclEnd, *pByte) != pInclEnd))
		{
			sRet += CharType(*pByte);
		}
		else
		{
			//
			// 2. Escape any disallowed characters
			//
			char buffer[10];
			sprintf(buffer, "%%%02X", (unsigned)*pByte);
			sRet += StringUtils::FromLatin1(buffer);
		}
	}
	
	return sRet;
}

//==============================================================================
// URLEncoder::IdempotentEncode
//
/**
   Converts a Unicode string into an escaped form.

   This method performs a similar function to Encode(), but the algorithm
   has been modified to ignore certain escape characters thereby making
   it idempotent.

   The algorithm employed is described in the W3C
   <a href="http://www.w3.org/TR/REC-xml#sec-entity-decl">
   XML recommendation section 4.2.2, External Entities</a>.
*/
//
// URI references require encoding and escaping of certain characters.
// The disallowed characters include all non-ASCII characters, plus the
// excluded characters listed in Section 2.4 of [RFC 2396], except for
// the number sign (#) and percent sign (%) characters and the square bracket
// characters re-allowed in [RFC 2732].
//
// The excluded characters listed in Section 2.4 of IETF RFC 2396:
//
//  control: 0x00-0x1F and 0x7F
//  space: 0x20
//  delims: "<" | ">" | "#" | "%" | <">
//  unwise      = "{" | "}" | "|" | "\" | "^" | "[" | "]" | "`"
//
// 1) Each disallowed character is converted to UTF-8 as 1 or more bytes
// 2) Any bytes corresponding to a disallowed character are escaped
//    with the URI escaping mechanism (that is, converted to %HH, where
//    HH is the hexadecimal notation of the byte value)
// 3) The original character is replaced by the resulting character sequence
//
// Note that this normalization process is idempotent: repeated normalization
// does not change a normalized URI reference.
//==============================================================================
String URLEncoder::RawEncode(const String& uri)
{
	const char excluded[] = {'<', '>', '"', '{', '}', '|', '\\', '^', '\''};
	const char* pExclEnd = excluded + sizeof(excluded);

	String sRet;

	//
	// 1. Convert the URL to UTF-8.  Of course, it may already be encoded
	//    as UTF-8 if that is our internal string encoding.
	//
	ByteString utf8 = StringUtils::ToUTF8(uri);

	sRet.reserve(utf8.length());
	const Byte* pEnd =  (const Byte*)utf8.data() + utf8.length();

	for(const Byte* pByte=(const Byte*)utf8.data(); pByte<pEnd; ++pByte)
	{
		if((*pByte > 0x20 && *pByte < 0x7F)
			&& std::find(excluded, pExclEnd, *pByte) == pExclEnd)
		{
			sRet += CharType(*pByte);
		}
		else
		{
			//
			// 2. Escape any disallowed characters
			//
			char buffer[10];
			sprintf(buffer, "%%%02X", (unsigned)*pByte);
			sRet += StringUtils::FromLatin1(buffer);
		}
	}
	
	return sRet;
}

QC_NET_NAMESPACE_END

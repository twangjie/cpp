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
	@class qc::net::URLDecoder
	
	@brief A class module containing static methods for decoding strings
	that have been transmitted as an escaped URL.
*/
//==============================================================================

#include "URLDecoder.h"

#include "QcCore/base/ArrayAutoPtr.h"
#include "QcCore/base/AutoBuffer.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/cvt/UTF8Converter.h"

QC_NET_NAMESPACE_BEGIN

using cvt::UTF8Converter;

//==============================================================================
// URLDecoder::Decode
//
/**
   Converts an escaped string in the MIME @c x-www-form-urlencoded format into 
   a plain Unicode string.

   To convert a URL, each character is examined in turn: 

   - The ASCII characters 'a' through 'z', 'A' through 'Z', '0' through '9', 
      and ".", "-", "*", "_" remain the same. 
   - The plus sign '+' is converted into a space character (U+0020)
   - The remaining characters are represented by 3-character strings which begin
     with the percent sign, "%xy", where xy is the two-digit hexadecimal
	 representation of the UTF-8 encoded character. 

   @sa URLEncoder::Encode()
*/
//==============================================================================
String URLDecoder::Decode(const String& uri)
{
	if(uri.empty())
		return uri;

	ByteString utf8;
	utf8.reserve(uri.size());

	size_t pos = 0;
	size_t size = uri.size();

	// An encoded URL uses only US-ASCII characters.
	while(pos < size)
	{
		const CharType x = uri[pos];
		switch(x)
		{
			case '%':
				if(pos+2 < size)
				{
					ByteString hex = StringUtils::ToAscii(uri.substr(pos+1, 2));
					if(StringUtils::IsHexString(hex))
					{
						int n = (int)strtol(hex.c_str(), 0, 16);
						utf8 += (Byte)n;
						pos+=2;
					}
					else
					{
						utf8 += '%';
					}
				}
				break;
			case '+':
				utf8 += ' ';
				break;
			default:
				utf8 += (Byte)x;
		}
		pos++;
	}

	return StringUtils::FromUTF8(utf8);
}

//==============================================================================
// URLDecoder::RawDecode
//
/**
   Converts a raw-encoded URI string into a Unicode String.

   This method performs a similar function to Decode(), but the algorithm
   has been modified to complement the URLEncoder::RawEncode() function 
   which performs an idempotent transformation.

   The algorithm employed is described in the W3C
   <a href="http://www.w3.org/TR/REC-xml#sec-entity-decl">
   XML recommendation section 4.2.2, External Entities</a>.
*/
//
// The steps involved are a little simpler than encoding:
// 1) scan the string, replacing % followed by 2 hex digits with the byte 
//    equivalent
// 2) decode from utf-8
//==============================================================================
String URLDecoder::RawDecode(const String& uri)
{
	//
	// For efficiency and to prevent trying to instantiate
	// a null std string, return the empty string immediately
	//
	if(uri.empty())
		return uri;
	
	AutoBuffer<Byte> utf8Buffer(uri.size());

	size_t pos = 0;
	size_t size = uri.size();

	while(pos < size)
	{
		const UCharType x = uri[pos];
		switch(x)
		{
			case '%':
				if(pos+2 < size)
				{
					ByteString hex = StringUtils::ToAscii(uri.substr(pos+1,2));
					if(StringUtils::IsHexString(hex))
					{
						int n = (int)strtol(hex.c_str(), 0, 16);
						utf8Buffer.append(n);
						pos+=2;
					}
					else
					{
						utf8Buffer.append(Byte(x));
					}
				}
				break;
			default:
				if(x <= 0x7FU)
					utf8Buffer.append(Byte(x));
				else
					throw IllegalArgumentException(QC_T("uri string contains non-ascii characters"));
		}
		pos++;
	}

	//
	// We now have a UTF-8 buffer.  We just have to convert into into a
	// String in the internal encoding.
	//
	ByteString utf8((const char*)utf8Buffer.data(), utf8Buffer.size());
	return StringUtils::FromUTF8(utf8);
}

QC_NET_NAMESPACE_END

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
// Class: ISO88591Converter
// 
// ISO-8859-1 characters share the same bit representation as the first 256
// Unicode characters, therefore no conversion is necessary when decoding
// from ISO-8859-1 to Unicode.  Only characters up to 0xFF can be encoded
// anything higher is an error.
//
//==============================================================================

#include "ISO88591Converter.h"

#include "QcCore/base/SystemCodeConverter.h"

QC_CVT_NAMESPACE_BEGIN

//==============================================================================
// ISO88591Converter::decode
//
//==============================================================================
CodeConverter::Result ISO88591Converter::decode(const Byte *from, const Byte *from_end,
                                            const Byte *& from_next,
                                            CharType *to, CharType *to_limit,
                                            CharType *& to_next)
{
	Result ret = ok;
	from_next = from, to_next = to;
	
	//
	// decode Bytes into Characters until 
	// one of the buffers is exhausted
	//
	while(ret == ok && from_next < from_end && to_next < to_limit)
	{
		// For ultimate efficiency, test if we are in the ascii range,
		// if so, there is no internal encoding reqd
		if(*from_next < 0x80U)
		{
			*to_next++ = *from_next++;
		}
		else
		{
			if( (ret=SystemCodeConverter::ToInternalEncoding(*from_next, to_next, to_limit, to_next)) == ok)
			{
				from_next++;
			}
		}
	}

	return (ret); 
}

//==============================================================================
// ISO88591Converter::encode
//
//==============================================================================
CodeConverter::Result ISO88591Converter::encode(const CharType *from, const CharType *from_end,
                                                const CharType *& from_next,
                                                Byte *to, Byte *to_limit,
                                                Byte *& to_next)
{
	Result ret = ok;
	from_next = from, to_next = to;

	//
	// encode Characters into Bytes until 
	// one of the buffers is exhausted
	//
	while(from_next < from_end && to_next < to_limit && ret == ok)
	{
		UCS4Char ch;
		const CharType * from_next_copy;
		if( (ret=SystemCodeConverter::FromInternalEncoding(ch, from_next, from_end, from_next_copy))==ok)
		{
			if (ch < 0xFFU)	// needs just 1 byte
			{
				*to_next++ = Byte(ch);
				from_next = from_next_copy;
			}
			else // greater than 0xFE is an error for ISO-8859-1
			{
				if( (ret = handleUnmappableCharacter(ch, to_next, to_limit, to_next)) == ok)
				{
					from_next = from_next_copy;
				}
			}
		}
		else if(ret == error)
		{
			internalEncodingError(from_next, from_end-from_next);
		}
	}
	
	return ret;
}

//==============================================================================
// ISO88591Converter::getDecodedLength
//
// Returns the number of Unicode characters that an external array of bytes will
// generate once decoded.
//==============================================================================
size_t ISO88591Converter::getDecodedLength(const Byte *from, const Byte *from_end) const
{
	return (from_end - from);
}

bool ISO88591Converter::alwaysNoConversion() const
{
	// Even though ISO-8859-1 is Unicode encoded into a single octet,
	// This doesn't mean that ISO-8859-1 files don't need decoding
	// into the encoding of the application.  In fact, there is not a Latin1
	// internal encoding, so conversion is always required.
	return (false);
}

size_t ISO88591Converter::getMaxEncodedLength() const
{
	return 1;
}

//==============================================================================
// ISO88591Converter::getEncodingName
//
//==============================================================================
String ISO88591Converter::getEncodingName() const
{
	return QC_T("ISO-8859-1");
}

QC_CVT_NAMESPACE_END


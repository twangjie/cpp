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
// Class: ASCIIConverter
// 
// US-ASCII characters are 7-bit characters that share the same bit representation
// as the first 128 ISO10646 (UNICODE) characters, therefore no conversion is
// necessary when decoding from ASCII to Unicode.  Only characters up to 0x7F 
// can be encoded anything higher is an error.
//
//==============================================================================

#include "ASCIIConverter.h"

#include "QcCore/base/SystemCodeConverter.h"

QC_CVT_NAMESPACE_BEGIN

//==============================================================================
// ASCIIConverter::decode
//
//==============================================================================
CodeConverter::Result ASCIIConverter::decode(const Byte *from, const Byte *from_end,
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
	while(from_next < from_end && to_next < to_limit && ret == ok)
	{
		//
		// If the input byte is ASCII then we can convert it directly into
		// an internal character - all internal encodings are equal for the ASCII range
		//
		if(*from_next < 0x80U)
		{
			*to_next++ = *from_next++;
		}
		else
		{
			if(getInvalidCharAction() == abort)
			{
				handleInvalidByteSequence(from_next, 1);
				ret = error;
			}
			else
			{
				QC_DBG_ASSERT(getInvalidCharAction() == replace);
				if( (ret=SystemCodeConverter::ToInternalEncoding(
					getInvalidCharReplacement(), to_next, to_limit, to_next))==ok)
				{
					++from_next;
				}
			}
		}
	}

	return ret; 
}

//==============================================================================
// ASCIIConverter::encode
//
//==============================================================================
CodeConverter::Result ASCIIConverter::encode(const CharType *from, const CharType *from_end,
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
		if( (ret=SystemCodeConverter::FromInternalEncoding(ch, from_next, from_end, from_next_copy)) == ok)
		{
			if (ch < 0x80U)	// needs just 1 byte
			{
				*to_next++ = Byte(ch);
				from_next = from_next_copy;
			}
			else // greater than 0x7F is an error for US-ASCII
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
// ASCIIConverter::getDecodedLength
//
// Returns the number of Unicode character that an external array of bytes will
// generate once decoded.
//==============================================================================
size_t ASCIIConverter::getDecodedLength(const Byte *from, const Byte *from_end) const
{
	return (from_end - from);
}

bool ASCIIConverter::alwaysNoConversion() const
{
	// even though ASCII will not need converting, it does require
	// validation.  By returning false here, we ensure that the encode/decode
	// methods are always called.
	return (false);
}

size_t ASCIIConverter::getMaxEncodedLength() const
{
	return 1;
}

//==============================================================================
// ASCIIConverter::getEncodingName
//
//==============================================================================
String ASCIIConverter::getEncodingName() const
{
	return QC_T("US-ASCII");
}

QC_CVT_NAMESPACE_END


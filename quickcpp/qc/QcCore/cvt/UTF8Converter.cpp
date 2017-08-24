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
// Class: UTF8Converter
// 
// UTF-8 characters are encoded using sequences of 1 to 6 octets (bytes).  The
// only byte sequence of 1 has the high-order bit set to 0 and is used to
// represent the 7-bit US-ASCII character set.
//
// In a sequence of n bytes where n>1, the initial byte has the n high-order
// bits set to 1 followed by a bit set to 0.  The remaining bits of this byte
// contain bits from the value of the character being decoded.  The following
// bytes all have the high-order bit set to 1 and the following bit set to 0,
// leaving 6 bits in each to contain bits from the character being decoded.
//
// Example
// -------
// UCS-4 range (hex)    UTF-8 byte sequence
// 0000 0000 0000 007F  0xxxxxxx                                     (7  bits)
// 0000 0080 0000 07FF  110xxxxx 10xxxxxx                            (11 bits)
// 0000 0800 0000 FFFF  1110xxxx 10xxxxxx 10xxxxxx                   (16 bits)
// 0001 0000 001F FFFF  11110xxx 10xxxxxx 10xxxxxx 10xxxxxx          (21 bits)
// 0020 0000 03FF FFFF  111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx (26 bits)
// 0400 0000 7FFF FFFF  1111110x 10xxxxxx ...      10xxxxxx 10xxxxxx (31 bits)
//
// See RFC2279 For a full description of the algorithm.
//
// Note, all the ranges in the above table are mutually exclusive, therefore
// there is only ever one way to encode a character into UTF-8 and only one
// valid decoding.
//
// We test non-first bytes for validity by anding 0xC0 (11000000) with the
// byte and expecting an answer of 0x80 (10000000).  Anything else means
// that the first two bits aren't "10" which is an encoding error.
//==============================================================================

#include "UTF8Converter.h"

#include "QcCore/base/SystemCodeConverter.h"

QC_CVT_NAMESPACE_BEGIN

//==============================================================================
// UTF8Converter::decode
//
//==============================================================================
CodeConverter::Result UTF8Converter::decode(const Byte *from, const Byte *from_end,
                                            const Byte *& from_next,
                                            CharType *to, CharType *to_limit,
                                            CharType *& to_next)
{
	from_next = from, to_next = to;

	Result ret = ok;

	//
	// decode Bytes into Characters until an error
	// or one of the buffers is exhausted
	//
	while(ret == ok && from_next < from_end && to_next < to_limit)
	{
		// If the top bit is not on, then this is plain US-ASCII
		if ((*from_next & 0x80) == 0x00)
		{
			*to_next++ = *from_next++ & 0xff;
		}
		else
		{
			UCS4Char ch;
			const Byte* from_next_copy = from_next;
			ret = SystemCodeConverter::UTF8Decode(ch, from_next, from_end, from_next_copy);

			if(ret == error)
			{
				if(getInvalidCharAction() == abort)
				{
					handleInvalidByteSequence(from_next, s_TrailingBytesForUTF8[*from_next]+1);
					break;
				}
				else
				{
					QC_DBG_ASSERT(getInvalidCharAction() == replace);
					if( (ret=SystemCodeConverter::ToInternalEncoding(
						getInvalidCharReplacement(), to_next, to_limit, to_next))==ok)
					{
						//
						// When recovering from an invalid UTF-8 sequence
						// we have a choice as to how many bytes we deem to
						// be part of the errant sequence. 
						// 
						// For the exception msg (above) we use the official
						// trailing length.  However, as most UTF-8 errors
						// are caused by people editing UTF-8 with a Latin-1
						// (or other) editor, perhaps the most appropriate
						// recovery technique is to jump to the next
						// seqence start character.
						//
						while(++from_next < from_end && (*from_next) > 0x7FU && (*from_next) < 0xC0U)
						{
						}
					}
				}
			}
			else if(ret == ok)
			{
				if( (ret=SystemCodeConverter::ToInternalEncoding(ch, to_next, to_limit, to_next))==ok)
				{
					from_next = from_next_copy;
				}
			}
		}
	}

	return (ret); 
}

//==============================================================================
// UTF8Converter::encode
//
//==============================================================================
CodeConverter::Result UTF8Converter::encode(const CharType *from, const CharType *from_end,
                                            const CharType *& from_next,
                                            Byte *to, Byte *to_limit,
                                            Byte *& to_next)
{
	from_next = from, to_next = to;

	Result ret = ok;

	//
	// encode characters into bytes until an error
	// or one of the buffers is exhausted
	//
	while(ret == ok && from_next < from_end && to_next < to_limit)
	{
		if ((unsigned)*from_next <= 0x7F)	// needs just 1 byte
		{
			*to_next++ = Byte(*from_next++);
		}
		else
		{
			UCS4Char ch;
			const CharType * from_next_copy;
			if( (ret = SystemCodeConverter::FromInternalEncoding(ch, from_next, from_end, from_next_copy)) == ok)
			{
				if( (ret = SystemCodeConverter::UTF8Encode(ch, to_next, to_limit, to_next)) == ok)
				{
					from_next = from_next_copy;
				}
				else if(ret == error)
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
	}

	return ret;
}

//==============================================================================
// UTF8Converter::getDecodedLength
//
// Returns the number of Unicode characters that an external array of bytes will
// generate once decoded.
//==============================================================================
size_t UTF8Converter::getDecodedLength(const Byte *from, const Byte *from_end) const
{
	int ret = 0;

    for (; from < from_end; )
	{
		from += s_TrailingBytesForUTF8[*from]+1;
		ret++;
	}
	
	return ret;
}

//==============================================================================
// UTF8Converter::alwaysNoConversion
//
// Determine whether or not an external Byte stream will need to be converted
// before it can be used as an internal character sequence.
//
// When the pre-processor symbol QC_UTF8 is defined, our internal characters
// are stored as UTF-8, therefore we can take advantage of this situation
// and read UTF-8 encoded files directly into memory.
//==============================================================================
bool UTF8Converter::alwaysNoConversion() const
{
#if defined(QC_UTF8)
	return (true);
#else
	return (false);
#endif
}

//==============================================================================
// UTF8Converter::getMaxEncodedLength
//
// Return the maximum number of bytes required to encode a single character
// [sequence]
//==============================================================================
size_t UTF8Converter::getMaxEncodedLength() const
{
	return (6);
}

//==============================================================================
// UTF8Converter::getEncodingName
//
//==============================================================================
String UTF8Converter::getEncodingName() const
{
	return QC_T("UTF-8");
}

QC_CVT_NAMESPACE_END


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
// Class CodeConverterBase
/**
	@class qc::CodeConverterBase
	
	@brief Base class that contains enums, values and static methods used by
	all CodeConverter sub-classes.

	The primary motivation for the existence of this class is the 
	CodeConverterBase::Result enum which represents the result of a conversion
	operation.  Derived classes usually inherit from this class in order to gain
	visibility to this enum.

    For convenience, this class also contains some important static methods
	for the encoding and decoding of sequences of ::CharType
	into Unicode code-point values (and vice versa).
*/
//==============================================================================

#include "CodeConverterBase.h"
#include "debug.h"

QC_BASE_NAMESPACE_BEGIN 

//
// Index into the table below with the first byte of a UTF-8 sequence to
// get the number of trailing bytes that are supposed to follow it.
//
const char CodeConverterBase::s_TrailingBytesForUTF8[256] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0
};

//
// Magic values subtracted from a buffer value during UTF8 conversion.
// This table contains as many values as there might be trailing bytes
// in a UTF-8 sequence.
// 
// e.g. for a 2-byte UTF-8 sequence, the following bits are not part of the number
//      110xxxx 10xxxxxx
//      0xc0 << 6 = 0x0011 0000 0000 0000
//      0x80 << 0 = 0x0000 0000 1000 0000
//                  0x0011 0000 1000 0000 = 0x3080
//
static const UCS4Char OffsetsFromUTF8[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 
					 0x03C82080UL, 0xFA082080UL, 0x82082080UL };

//==============================================================================
// CodeConverterBase::Result 
//
/**
   Encodes the single Unicode character @c ch into a UTF-8 ::Byte array.

   @param ch the code-point value for the Unicode character to be encoded
   @param to pointer to the start of the output buffer
   @param to_limit pointer to the end of the output buffer.  In common with
          C++ standard library conventions, this must point at the next ::Byte 
		  position after the end of the output buffer
   @param to_next return parameter containing a pointer to the first unused
          ::Byte position in the passed output buffer
   
   @returns One of the CodeConverterBase::Result values indicating the success
            of the operation.
*/
//==============================================================================
CodeConverterBase::Result CodeConverterBase::UTF8Encode(UCS4Char ch,
	Byte* to, const Byte* to_limit, Byte*& to_next)
{
	static const int firstByteMark[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};
	static const int byteMask = 0x3F;
	static const int byteMark = 0x80; 

	if( (to_next = to) < to_limit)
	{
		int bytesToWrite;

		if(ch < 0x80)	// needs just 1 byte
		{
			*to_next++ = static_cast<Byte>(ch);
			return ok;
		}
		else if (ch < 0x800)    bytesToWrite = 2;
		else if (ch < 0x10000)  bytesToWrite = 3;
		else if (ch < 0x200000) bytesToWrite = 4;
		else if (ch < 0x400000) bytesToWrite = 5;
		else
		{
			bytesToWrite = 6;
		}

		if(to_next+bytesToWrite > to_limit)
		{
			return outputExhausted;
		}
		else
		{
			to_next += bytesToWrite;
			Byte* pTarget = to_next;
			switch (bytesToWrite)
			{
				/* note: everything falls through. */
				case 6:	*--pTarget = Byte((ch & byteMask) | byteMark); ch >>= 6;
				case 5:	*--pTarget = Byte((ch & byteMask) | byteMark); ch >>= 6;
				case 4:	*--pTarget = Byte((ch & byteMask) | byteMark); ch >>= 6;
				case 3:	*--pTarget = Byte((ch & byteMask) | byteMark); ch >>= 6;
				case 2:	*--pTarget = Byte((ch & byteMask) | byteMark); ch >>= 6;
				case 1:	*--pTarget = Byte( ch | firstByteMark[bytesToWrite]);
			}
		}
		return ok;
	}
	else
	{
		return error;
	}
}

//==============================================================================
// CodeConverterBase::Result 
//
/**
   Decodes a UTF-8 sequence of bytes into a single Unicode character.

   @param ch returned code-point value for the decoded Unicode character
   @param from pointer to the start of the UTF-8 encoded input buffer
   @param from_end pointer to the end of the input buffer.  In common with
          C++ standard library conventions, this must point at the next ::Byte 
		  position after the end of the input buffer
   @param from_next return parameter containing a pointer to the start of the 
          next multi-byte sequence in the input buffer
   
   @returns One of the CodeConverterBase::Result values indicating the success
            of the operation.
*/
//==============================================================================
CodeConverterBase::Result CodeConverterBase::UTF8Decode(UCS4Char& ch, 
                                              const Byte *from,
                                              const Byte *from_end,
                                              const Byte *& from_next)
{
	from_next = from;

	QC_DBG_ASSERT(from < from_end);

	// If the top bit is not on, then this is plain US-ASCII
	if ((*from_next & 0x80) == 0)
	{
		ch = *from_next++;
		return ok;
	}

	//
	// We don't support 5 or 6 byte sequences.  They are not part of the
	// Unicode standard for UTF-8
	//
	if((*from_next & 0xF8) == 0xF8)
		return error;

	unsigned short extraBytesToRead = s_TrailingBytesForUTF8[*from_next];

	if(from_next+extraBytesToRead >= from_end)
	{
		return inputExhausted;
	}

	if (!IsLegalUTF8(from_next, extraBytesToRead+1))
	{
		return error;
	}

	ch = 0;

	const Byte* from_next_copy = from_next;

	// 
	// For efficiency the cases all fall through.
	//
	switch (extraBytesToRead)
	{
		case 3:	ch += *from_next_copy++; ch <<= 6;
		case 2:	ch += *from_next_copy++; ch <<= 6;
		case 1:	ch += *from_next_copy++; ch <<= 6;
		case 0:	ch += *from_next_copy++;
	}

	/* The idea of subtracting a magic number comes from Mark E. Davis, 1994.*/
	ch -= OffsetsFromUTF8[extraBytesToRead];

	from_next = from_next_copy;

	return ok;
}

//==============================================================================
// CodeConverterBase::IsLegalUTF8
//
/**
   Tests if the multi-byte sequence starting at @c pStart 
   for @c length bytes is a valid UTF-8 sequence representing a single 
   Unicode character.

   @param pStart pointer to the first ::Byte of a multi-byte UTF-8 sequence
   @param length length of the multi-byte sequence to test
   @returns true if the sequence is valid UTF-8; false otherwise
*/
//
// Adapted from code found on the Unicode.org web site written by
// Mark E. Davis, 1994.
//==============================================================================
bool CodeConverterBase::IsLegalUTF8(const Byte* pStart, size_t length)
{
	Byte a;
	const Byte *srcptr = pStart+length;

	switch (length)
	{
	default: return false;
		/* Everything else falls through when "true"... */
	case 4: if (((a = (*--srcptr)) & 0xc0) != 0x80) return false;
	case 3: if (((a = (*--srcptr)) & 0xc0) != 0x80) return false;
	case 2: if (((a = (*--srcptr)) & 0xc0) != 0x80) return false;
		switch (*pStart)
		{
		    /* no fall-through in this inner switch */
		    case 0xE0: if (a < 0xA0) return false; break;
		    case 0xF0: if (a < 0x90) return false; break;
		    case 0xF4: if (a > 0x8F) return false; break;
		    default:   if (a < 0x80) return false;
		}
    case 1: if (*pStart>= 0x80 && *pStart < 0xC2) return false;
	        if (*pStart > 0xF4) return false;
	}

	return true;
}

//==============================================================================
// CodeConverterBase::IsLegalUTF16
//
/**
   Tests if the wchar_t sequence starting at @c pStart 
   for @c length characters is a valid UTF-16 sequence representing a single 
   Unicode character.  A UTF-16 sequence consists of either a single
   wchar_t value or a pair of values in the surrogate range.

   @param pStart pointer to the first wchar_t of a UTF-16 sequence
   @param length length of the sequence to test
   @returns true if the sequence is valid UTF-16; false otherwise
*/
//==============================================================================
bool CodeConverterBase::IsLegalUTF16(const wchar_t* pStart, size_t length)
{
	//static const unsigned surHighStart = 0xD800UL;
	//static const unsigned surHighEnd   = 0xDBFFUL;
	//static const unsigned surLowStart  = 0xDC00UL;
	//static const unsigned surLowEnd    = 0xDFFFUL;

	// Is the first character outside the surrogate range?
	if((*pStart & 0xF800UL) != 0xD800UL)
	{
		return (length == 1);
	}

	// For a surrogate pair, check the first surrogate is in the
	// high range and the second is in the low range
	if(length == 2)
	{
		return ((*pStart & 0xFC00UL) == 0xD800UL) &&
		       ((*(pStart+1) & 0xFC00UL) == 0xDC00UL);
	}
	else
	{
		return false;
	}
}

QC_BASE_NAMESPACE_END

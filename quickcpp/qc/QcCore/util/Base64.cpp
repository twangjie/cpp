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
// Class Base64
/**
	@class qc::util::Base64
	
	@brief Class module for encoding and decoding byte arrays to and from
	       Base64.

	The Base64 Content-Transfer-Encoding is described in 
	<a href="http://www.ietf.org/rfc/rfc2045.txt">RFC 2045</a> section 6.8.
	Briefly, the Base64 encoding transforms each 3 bytes (24 bits) of input into
	4 ASCII bytes, each one encoding 6 bits of the input.  The output 
	bytes only require 65 unique values (taken from 2^6 + 1 terminator)
	which means that a subset of printable ASCII can be used.

    Each 6-bit group is used as an index into an array of 64 printable
    characters.  The character referenced by the index is placed in the
    output string.  These characters, identified in Table 1, below, are
    selected so as to be universally representable, and the set excludes
    characters with particular significance to SMTP (e.g., ".", CR, LF)
    and to the multi-part boundary delimiters defined in RFC 2046 (e.g.,
    "-").

     @code
                    Table 1: The Base64 Alphabet

     Value Encoding  Value Encoding  Value Encoding  Value Encoding
         0 A            17 R            34 i            51 z
         1 B            18 S            35 j            52 0
         2 C            19 T            36 k            53 1
         3 D            20 U            37 l            54 2
         4 E            21 V            38 m            55 3
         5 F            22 W            39 n            56 4
         6 G            23 X            40 o            57 5
         7 H            24 Y            41 p            58 6
         8 I            25 Z            42 q            59 7
         9 J            26 a            43 r            60 8
        10 K            27 b            44 s            61 9
        11 L            28 c            45 t            62 +
        12 M            29 d            46 u            63 /
        13 N            30 e            47 v
        14 O            31 f            48 w         (pad) =
        15 P            32 g            49 x
        16 Q            33 h            50 y
     @endcode
*/
//==============================================================================

#include "Base64.h"

#include "QcCore/base/ArrayAutoPtr.h"
#include "QcCore/base/NullPointerException.h"
#include "QcCore/base/IllegalArgumentException.h"
#include "QcCore/base/debug.h"

QC_UTIL_NAMESPACE_BEGIN

//
// Table of characters used in the Base64 Alphabet
//
const Byte base64Chars[64] =
{
    'A','B','C','D','E','F','G','H',
    'I','J','K','L','M','N','O','P',
    'Q','R','S','T','U','V','W','X',
    'Y','Z','a','b','c','d','e','f',
    'g','h','i','j','k','l','m','n',
    'o','p','q','r','s','t','u','v',
    'w','x','y','z','0','1','2','3',
    '4','5','6','7','8','9','+','/',
};

//
// Reverse lookup table for the Base64 alphabet.
// reverseChars[byte] gives n for the nth Base64
// character or 0x80 if a character is not a Base64 character.
// 
const Byte reverseChars[256] = {
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x3E, 0x80, 0x80, 0x80, 0x3F,
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
	0x3C, 0x3D, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
	0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
	0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
	0x17, 0x18, 0x19, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
	0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
	0x31, 0x32, 0x33, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80};


//==============================================================================
// Base64::Encode
//
/**
   Encodes an array of bytes into base64.

   @param pFrom pointer to the start of an array of bytes to encode
   @param pFromEnd pointer to the next byte position after the end of the 
          array
   @param pTo pointer to the start of an array of bytes to receive the result
   @param pToLimit pointer to the next byte position after the end of the 
          result array
   @param pToNext return parameter which will contain a pointer
          to the first unused byte in the result array after encoding is complete.
   @throws NullPointerException if @c pFrom, @c pFromEnd, @c pTo or @c pToLimit
           are null.
   @throws IllegalArgumentException if @c pFromEnd is less than @c pFrom or
           @c pToLimit is less than @c pTo
*/
//==============================================================================
Base64::Result Base64::Encode(const Byte* pFrom,
                              const Byte* pFromEnd,
                              Byte* pTo,
                              const Byte* pToLimit,
                              Byte*& pToNext)
{
	if(!pFrom || !pFromEnd || !pTo || !pToLimit)
		throw NullPointerException();
	if(pFromEnd < pFrom || pToLimit < pTo)
		throw IllegalArgumentException();

	pToNext = pTo;

	Byte minibuf[3] = {0, 0, 0};

	while (pFrom < pFromEnd)
	{
		if(pToNext+4 > pToLimit)
			return outputExhausted;

        size_t n = pFromEnd - pFrom;

        if (n<3)
		{
			for (size_t j=0; j<n; ++j)
			{
				minibuf[j] = pFrom[j];
			}
			pFrom  = minibuf;
			pFromEnd = minibuf+3;
		}

		pToNext[0] = base64Chars[pFrom[0] >> 2];
		pToNext[1] = base64Chars[((pFrom[0] & 3) << 4) | (pFrom[1] >> 4)];
		pToNext[2] = base64Chars[((pFrom[1] & 0xF) << 2) | (pFrom[2] >> 6)];
		pToNext[3] = base64Chars[pFrom[2] & 0x3F];

		// Replace characters in output with "=" pad
		// characters if fewer than three characters were
		// read from the end of the input stream.

		if (n < 3)
		{
			pToNext[3] = '=';
			if (n < 2)
			{
				pToNext[2] = '=';
			}
		}

		pToNext+=4;
		pFrom+=3;
	}
	
	return ok;
}

//==============================================================================
// Base64::Decode
//
/**
   Decodes an array of base64-encoded bytes.

   @param pFrom pointer to the start of an array of bytes to decode
   @param pFromEnd pointer to the next byte position after the end of the 
          array
   @param pTo pointer to the start of an array of bytes to receive the result
   @param pToLimit pointer to the next byte position after the end of the 
          result array
   @param pToNext return parameter which will contain a pointer
          to the first unused byte in the result array after decoding is complete.
   @throws NullPointerException if @c pFrom, @c pFromEnd, @c pTo or @c pToLimit
           are null.
   @throws IllegalArgumentException if @c pFromEnd is less than @c pFrom or
           @c pToLimit is less than @c pTo
*/
//==============================================================================
Base64::Result Base64::Decode(const Byte* pFrom,
                              const Byte* pFromEnd,
                              Byte* pTo,
                              const Byte* pToLimit,
                              Byte*& pToNext)
{
	if(!pFrom || !pFromEnd || !pTo || !pToLimit)
		throw NullPointerException();
	if(pFromEnd < pFrom || pToLimit < pTo)
		throw IllegalArgumentException();

	pToNext = pTo;
	while(true)
	{
		//
		// Assemble a block of 4 encoded bytes.  Any invalid
		// byte are bypassed and silently ignored
		//

		Byte buffer[4];
		size_t numBytes = 0;
		while(numBytes < 4)
		{
			if(pFrom < pFromEnd)
			{
				if(*pFrom == '=')
				{
					if(numBytes < 2)
						return inputExhausted;
					// Once we see a "=" we can safely skip to the end
					pFrom = pFromEnd;
					break;
				}
				else if( (buffer[numBytes] = reverseChars[*pFrom]) != 0x80)
				{
					pFrom++;
					numBytes++;
				}
			}
			else
			{
				return (numBytes == 0) ? ok : inputExhausted;
			}
		}

		QC_DBG_ASSERT(numBytes>1);

		if(pToNext + (numBytes - 1) > pToLimit)
			return outputExhausted;
		
		pToNext += (numBytes-1);

		Byte* pWork = pToNext-1;

		switch(numBytes)
		{
		case 4:
			*pWork-- = (buffer[2] << 6) | buffer[3];
		case 3:
			*pWork-- = (buffer[1] << 4) | (buffer[2] >> 2);
		case 2:
			*pWork   = (buffer[0] << 2) | (buffer[1] >> 4);
			break;
		default:
			QC_DBG_ASSERT(false);
		}
	}
}

//==============================================================================
// Base64::GetMaxEncodedLength
//
/**
   Returns the maximum number of bytes that a byte sequence
   will occupy once it has been encoded into base64.

   @param pFrom pointer to the start of an array of bytes to encode
   @param pFromEnd pointer to the next byte position after the end of the 
          array
*/
//==============================================================================
size_t Base64::GetEncodedLength(const Byte* pFrom, const Byte* pFromEnd)
{
	const size_t inputLen = pFromEnd - pFrom;
	return ((inputLen % 3) == 0) ? (inputLen/3 * 4) : ((inputLen/3+1) * 4);
}

//==============================================================================
// Base64::GetMaxDecodedLength
//
/**
   Returns the maximum number of bytes that a base64-encoded sequence
   will occupy once it has been decoded.
   @param pFrom pointer to the start of an array of bytes to decode
   @param pFromEnd pointer to the next byte position after the end of the 
          array
*/
//==============================================================================
size_t Base64::GetMaxDecodedLength(const Byte* pFrom, const Byte* pFromEnd)
{
	const size_t inputLen = pFromEnd - pFrom;
	return (inputLen / 4 * 3);
}


QC_UTIL_NAMESPACE_END

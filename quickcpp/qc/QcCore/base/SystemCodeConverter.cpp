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
// Class SystemCodeConverter
/**
	@class qc::SystemCodeConverter

	@brief Class module for converting Unicode strings to and from
	the internal character encoding.

	The @QuickCPP library has been designed to offer a high degree of flexibility
	in the way that it represents Unicode characters.  It can be configured to
	use either @c char or @c wchar_t characters. 
	
	When configured to use @c char characters, @QuickCPP encodes Unicode characters
	into UTF-8.  The size of a @c wchar_t is not uniformly defined on different platforms, so
	@QuickCPP offers a choice of two encoding schemes when configured to use @c wchar_t
	characters: UCS-4 for 32-bit implementations and UTF-16 for 16-bit implementations.

    <table>
	 <tr><td>character type</td> <td>size (bits)</td> <td>encoding(s)</td> </tr>
	 <tr><td>char</td>           <td>8</td>           <td>UTF-8</td>       </tr>
	 <tr><td>wchar_t</td>        <td>16</td>          <td>UTF-16</td>      </tr>
	 <tr><td>wchar_t</td>        <td>32</td>          <td>UCS-4</td>        </tr>
	</table>

*/
//==============================================================================

#include "SystemCodeConverter.h"
#include "IllegalCharacterException.h"
#include "NullPointerException.h"
#include "debug.h"

QC_BASE_NAMESPACE_BEGIN 

static const size_t MaxEncodedSize = 4;
static const UCS4Char surHighStart = 0xD800UL;
static const UCS4Char surHighEnd   = 0xDBFFUL;
static const UCS4Char surLowStart  = 0xDC00UL;
static const UCS4Char surLowEnd    = 0xDFFFUL;

//==============================================================================
// SystemCodeConverter::toInternalEncoding
//
/**
   Converts a Unicode character value into a sequence of one or more ::CharType
   characters encoded according to the internal Unicode encoding scheme.
   
   The caller must provide a buffer of ::CharType characters that will be used
   to hold the result of the conversion.

   @param ch the Unicode character's code-point value in the range 0-0x10FFFF
   @param to a pointer to the first ::CharType character of a buffer to hold the
             result of the conversion
   @param to_limit a pointer to the next ::CharType character after the end of
             the output buffer
   @param to_next a return parameter, points to the first unused character in
             the passed ::CharType buffer
   
   @returns A CodeConverterBase::Result indicating the result of the conversion.
   @throws NullPointerException if @c to or @c to_limit are null
*/
//==============================================================================
SystemCodeConverter::Result SystemCodeConverter::ToInternalEncoding(UCS4Char ch,
	CharType *to, const CharType *to_limit, CharType *& to_next)
{
	if(!to || !to_limit) throw NullPointerException();
	if(ch > 0x10FFFFUL) return error;

	if( (to_next = to) < to_limit)
	{
	#if defined(QC_UTF8)

		return UTF8Encode(ch, (Byte*)to, (Byte*)to_limit, (Byte*&)to_next);

	#elif defined(QC_UTF16)

		//
		// It is an error to encode characters in the Surrogate Pair range
		// they are not real characters.
		//
		if(ch >= surHighStart && ch <= surLowEnd)
		{
			return error;
		}
		else if(ch < 0x10000)
		{
			*to_next++ = CharType(ch);
			return ok;
		}
		else if(to_next + 1 >= to_limit)
		{
			return outputExhausted;
		}

		ch -= 0x10000;
		*to_next++ = CharType((ch >> 10)   + surHighStart);
		*to_next++ = CharType((ch & 0x3FF) + surLowStart);
		return ok;

	#elif defined(QC_UCS4)

		*to_next++ = ch;
		return ok;

	#elif defined(QC_UCS2)

		if(ch > 0xFFFFUL)
			return error;

		*to_next++ = ch;
		return ok;

	#else

		#error Internal character encoding not correctly specified
		return error;

	#endif

	}
	else // no room in o/p buffer
	{
		return error;
	}
}

//==============================================================================
// SystemCodeConverter::fromInternalEncoding
//
/**
   Converts a sequence of 1 or more ::CharType characters, which are encoded
   according to the internal Unicode encoding scheme, into the code-point
   for a Unicode character.
   
   @param ch a return parameter giving the Unicode character's code-point
          value in the range 0-0x10FFFF
   @param from a pointer to the beginning of a ::CharType buffer that holds
          the encoded sequence
   @param from_end a pointer to the end of the passed ::CharType buffer
   @param from_next a return parameter, points to the beginning of the 
          next multi-character sequence in the passed ::CharType buffer
   
   @returns A CodeConverterBase::Result indicating the result of the conversion.
   @throws NullPointerException if @c from or @c from_end are null
*/
//==============================================================================
SystemCodeConverter::Result
SystemCodeConverter::FromInternalEncoding(UCS4Char& ch,
                                          const CharType* from,
                                          const CharType* from_end,
                                          const CharType*& from_next)
{
	if(!from || !from_end) throw NullPointerException();

	if( (from_next = from) < from_end)
	{
	#if defined(QC_UTF8)

		return UTF8Decode(ch, (const Byte*)from, (const Byte*)from_end, (const Byte*&)from_next);

	#elif defined(QC_UTF16)

		if(*from_next < surHighStart || *from_next > surLowEnd)
		{
			ch = *from_next++;
			return ok;
		}

		//
		// We are into surrigate pair territory.  We should guard against
		// an invalid internal character stream
		//
		if(*from_next > surHighEnd)
		{
			return error;
		}
		else if(from_next+1 >= from_end)
		{
			return inputExhausted;
		}
		else if(*(from_next+1) < surLowStart || *(from_next+1) > surLowEnd)
		{
			return inputExhausted;
		}

		UCS4Char high = *from_next++;
		UCS4Char low  = *from_next++;
		ch = ((high - surHighStart) << 10)
		   + (low - surLowStart) + 0x10000;;

		return ok;

	#elif defined(QC_UCS4) || defined(QC_UCS2)

		ch = *from_next++;
		return ok;

	#else

		#error Internal character encoding not correctly specified
		return error;

	#endif

	}
	else // no room in o/p buffer
	{
		return error;
	}
}

//==============================================================================
// SystemCodeConverter::GetInternalEncodingName
//
/**
   Returns the name of the encoding scheme used by @QuickCPP to encode Unicode
   characters.

   @returns a String containing the name of the internal character encoding
             in use.  e.g. "UTF-8"
*/
//==============================================================================
String SystemCodeConverter::GetInternalEncodingName()
{
#if defined(QC_UTF8)

	return QC_T("UTF-8");

#elif defined(QC_UTF16)

	return QC_T("UTF-16");

#elif defined(QC_UCS4)

	return QC_T("UCS-4");

#elif defined(QC_UCS2)

	return QC_T("UCS-2");

#else

	#error Internal character encoding not correctly specified
	return QC_T("unknown");

#endif
}

//==============================================================================
// SystemCodeConverter::ToInternalEncoding
//
/**
   Returns the Unicode character @c ch as a String containing a sequence
   of one or more ::CharType characters encoded using the @QuickCPP internal encoding
   scheme.

   @param ch the Unicode character to encode
   @returns a String containing a sequence of one or more CharType characters
   @throws IllegalCharacterException if @c ch cannot be encoded into the
           @QuickCPP internal encoding
*/
//==============================================================================
String SystemCodeConverter::ToInternalEncoding(UCS4Char ch)
{
	CharType buffer[MaxEncodedSize];
	CharType* pNext;
	if(ToInternalEncoding(ch, buffer, buffer+MaxEncodedSize, pNext) != ok)
	{
		throw IllegalCharacterException();
	}

	return String(buffer, pNext-buffer);
}

//==============================================================================
// SystemCodeConverter::GetMaximumCharSequenceLength
//
/**
   Returns the maximum number of ::CharType characters that may be used
   to encode a single Unicode character.

   The return value depends on whether @c char or @c wchar_t has been selected
   as the @QuickCPP character type as well as the operating system platform.
*/
//==============================================================================
size_t SystemCodeConverter::GetMaximumCharSequenceLength()
{
#if defined(QC_UTF8)

	return 4;

#elif defined(QC_UTF16)

	return 2;

#elif defined(QC_UCS4)

	return 1;

#elif defined(QC_UCS2)

	return 1;

#else

	#error Internal character encoding not correctly specified
	return 1;

#endif
}

//==============================================================================
// SystemCodeConverter::TestEncodedSequence
//
/**
   Tests a sequence of ::CharType characters to check that it is encoded
   according to the chosen @QuickCPP internal encoding scheme.

   @param from a pointer to the beginning of a ::CharType buffer that holds
          the encoded sequence
   @param from_end a pointer to the end of the passed ::CharType buffer
   @param from_next a return parameter, points to the beginning of the 
          next multi-character sequence in the passed ::CharType buffer

   @returns A CodeConverterBase::Result indicating the result of the test.
   @throws NullPointerException if @c from or @c from_end are null
*/
//==============================================================================
CodeConverterBase::Result
SystemCodeConverter::TestEncodedSequence(const CharType* from,
                                         const CharType* from_end,
                                         const CharType*& from_next)
{
	if(!from || !from_end) throw NullPointerException();
	QC_DBG_ASSERT(from <= from_end);

	while(from!=from_end)
	{
		if(!IsSequenceStartChar(*from))
			return error;

		size_t seqLen = GetCharSequenceLength(*from);

		if(from + seqLen > from_end)
		{
			from_next=from;
			return outputExhausted;
		}

#if defined(QC_UTF8)

		if(!IsLegalUTF8((Byte*)from, seqLen))
			return error;

#elif defined(QC_UTF16)

		if(!IsLegalUTF16((wchar_t*)from, seqLen))
			return error;

#endif
		from+=seqLen;
	}

	QC_DBG_ASSERT(from == from_end);
	from_next=from_end;
	return ok;
}

QC_BASE_NAMESPACE_END

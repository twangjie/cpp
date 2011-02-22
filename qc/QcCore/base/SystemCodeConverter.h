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
// Class: SystemCodeConverter
// 
// Overview
// --------
// The SystemCodeConverter class is a well-known and accessible class module
// that provides access to important routines for translating internal character
// sequences to unicode characters and visa versa.
//
//==============================================================================

#ifndef QC_BASE_SystemCodeConverter_h
#define QC_BASE_SystemCodeConverter_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include "CodeConverterBase.h"
#include "String.h"

QC_BASE_NAMESPACE_BEGIN

class QC_BASE_PKG SystemCodeConverter : public CodeConverterBase
{
public:

	static Result ToInternalEncoding(UCS4Char ch, CharType*to, const CharType*to_limit, CharType*& to_next);
	static String ToInternalEncoding(UCS4Char ch);
	static Result FromInternalEncoding(UCS4Char& ch, const CharType* from, const CharType* from_end, const CharType*& from_next);
	static String GetInternalEncodingName();
	static bool IsSequenceStartChar(UCharType ch);
	static size_t GetCharSequenceLength(UCharType ch);
	static size_t GetMaximumCharSequenceLength();
	static Result TestEncodedSequence(const CharType* from, const CharType* from_end, const CharType*& from_next);
	static bool IsValidCharSequence(const CharType* from, size_t len);
};

//==============================================================================
// SystemCodeConverter::GetCharSequenceLength
//
/**
Returns the number of ::CharType characters that make up a multi-character
sequence.

Unless the internal encoding used by the library is UCS-4 (where ::CharType
is at least 21-bits wide and can represent all Unicode characters from 
U+0000 - U+10FFFF), Unicode characters are represented internally using a sequence
of one or more ::CharType characters.

Each multi-character encoding supported (UTF-16 and UTF-8) allows the length 
of the sequence to be determined from the first character of the sequence.

In the case of UTF-16 this is 1 unless @c ch is a surrogate pair start character
(0xD800-0xDBFF) in which case the length is 2.

In the case of UTF-8 the sequence length can be established by looking at the
number of high-order  bits set to '1' in the passed char @c ch.  If no high-order 
bits are set, then the passed character is equivalent to an ASCII character and
the sequence has a length of 1.  In common with the rest of @QuickCPP, this method
does not recognize UTF-8 sequences greater than 4 bytes.  Lead bytes that indicate
sequences longer than 4 are treated as indicating a sequence of length 1.

@returns the length of the multi-character sequence denoted by the start
character @c ch
@param ch The first character in a sequence of one or more ::CharType
characters
*/
//==============================================================================
inline 
	size_t SystemCodeConverter::GetCharSequenceLength(UCharType ch)
{

#if defined(QC_UTF8)

	return s_TrailingBytesForUTF8[ch] + 1;

#elif defined(QC_UTF16)

	if((ch & 0xF800) == 0xD800) // first part of surrogate pair
		return 2;
	else
		return 1;

#elif defined(QC_UCS4) || defined(QC_UCS2)

	return 1;

#else
#error Internal character encoding not correctly specified
	return 1;

#endif

}

//==============================================================================
// SystemCodeConverter::IsSequenceStartChar
//
/**
Tests the passed character @c ch to see if it marks the start of
a multi-character sequence, a standalone character or a trailing character.

@param ch character to test
@returns true if @c ch is either a standalone character or marks the start
of a multi-character sequence; false otherwise

*/
//==============================================================================
inline
	bool SystemCodeConverter::IsSequenceStartChar(UCharType ch)
{
#if defined(QC_UTF8)

	// valid sequence start are ascii chars (<0x80) or
	// >= 0xC0

	return (ch < 0x80U || ch > 0xBFU);

#elif defined(QC_UTF16)

	//
	// Simply must not be in the 2nd half of a surrogate pair
	//
	// The 2nd half of a surrogate pair is in the range 0xDC00 - 0xDFFF
	// 0xDC00 = 1101 1100 0000 0000
	// 0xDFFF = 1101 1111 1111 1111
	// mask     1111 1100 0000 0000 = 0xFC00

	return ((ch & 0xFC00) != 0xDC00);

#elif defined(QC_UCS4) || defined(QC_UCS2)

	return true;

#else
#error Internal character encoding not correctly specified
	return false;

#endif
}

//==============================================================================
// SystemCodeConverter::IsValidCharSequence
//
/**
Tests the passed ::CharType sequence @c from, for a length of
@c len to see if it represents a properly encoded Unicode character.

@param from pointer to the first ::CharType character in the sequence
@param len the number of ::CharType characters in the encoded sequence
@returns true if the sequence represents a valid Unicode character; false otherwise
*/
//==============================================================================
inline
	bool SystemCodeConverter::IsValidCharSequence(const CharType* from, size_t len)
{
#if defined(QC_UTF8)
	return IsLegalUTF8((const Byte*)from, len);
#elif defined(QC_UTF16)
	return IsLegalUTF16((const wchar_t*)from, len);
#elif defined(QC_UCS4)
	return (*from <= 0x10FFFF);
#else
	return true;
#endif
}

QC_BASE_NAMESPACE_END

#endif //QC_BASE_SystemCodeConverter_h

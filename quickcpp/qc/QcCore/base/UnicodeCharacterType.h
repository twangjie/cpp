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
// Class: UnicodeCharacterType
//
// A locale-independant way of determining the Unicode characteriztics 
// of a character.
// 
//==============================================================================

#ifndef QC_BASE_UnicodeCharacterType_h
#define QC_BASE_UnicodeCharacterType_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

QC_BASE_NAMESPACE_BEGIN

class QC_BASE_PKG UnicodeCharacterType
{
public:

	static bool IsAscii(UCharType ch);
	static bool IsDigit(UCharType ch);
	static bool IsLetter(UCharType ch);
	static bool IsHexDigit(UCharType ch);
	static bool IsSpace(UCharType ch);
	static bool IsAlphaNumeric(UCharType ch);
};

//==============================================================================
// UnicodeCharacterType::IsAscii
//
/**
Tests if the Unicode character represented by the single ::CharType @c ch
is an ASCII character (0-0x7F).

@returns true if this Character is an ASCII character; false otherwise.
*/
//==============================================================================
inline
	bool UnicodeCharacterType::IsAscii(UCharType ch)
{
	return (ch <= 0x7F);
}


//==============================================================================
// UnicodeCharacterType::IsSpace
//
/**
Tests if the Unicode character represented by the single ::CharType @c ch
is white-space according to common Windows and Unix conventions.

Space characters are:-
- <tt>'\t' U+0009 HORIZONTAL TABULATION</tt>
- <tt>'\\n' U+000A NEW LINE</tt>
- <tt>'\f' U+000C FORM FEED</tt> 
- <tt>'\r' U+000D CARRIAGE RETURN</tt>  
- <tt>' '   U+0020 SPACE</tt> 

@returns true if @c ch is a space character; false otherwise.
*/
//==============================================================================
inline
	bool UnicodeCharacterType::IsSpace(UCharType ch)
{
	switch(ch)
	{
	case 0x0009:
	case 0x000A:
	case 0x000C:
	case 0x000D:
	case 0x0020:
		return true;
	default:
		return false;
	}
}

//==============================================================================
// UnicodeCharacterType::IsDigit
//
/**
Tests if the Unicode character represented by the single ::CharType @c ch
is an ASCII decimal digit 0-9.

@returns true if this Character is a decimal digit [0-9]; false otherwise.
*/
//==============================================================================
inline
	bool UnicodeCharacterType::IsDigit(UCharType ch)
{
	return (ch >= '0' && ch <= '9');
}


inline
	bool UnicodeCharacterType::IsLetter(UCharType ch)
{
	return ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'));
}
//==============================================================================
// UnicodeCharacterType::IsHexDigit
//
/**
Tests if the Unicode character represented by the single ::CharType @c ch
is an ASCII hexadecimal digit [0-9], [A-F], [a-f].

@returns true if this Character is a hexadecimal digit; false otherwise.
*/
//==============================================================================
inline
	bool UnicodeCharacterType::IsHexDigit(UCharType ch)
{
	return ((ch >= '0' && ch <= '9') ||
		(ch >= 'a' && ch <= 'f') ||
		(ch >= 'A' && ch <= 'F'));
}

//==============================================================================
// UnicodeCharacterType::IsAlphaNumeric
//
/**
Tests if the Unicode character represented by the single ::CharType @c ch
is an alpha-numeric character[a-z], [A-Z], [0-9].

@returns true if this Character is an alpha-numeric character; false otherwise.
*/
//==============================================================================
inline
	bool UnicodeCharacterType::IsAlphaNumeric(UCharType ch)
{
	return ((ch >= '0' && ch <= '9') ||
		(ch >= 'a' && ch <= 'z') ||
		(ch >= 'A' && ch <= 'Z'));
}

QC_BASE_NAMESPACE_END

#endif //QC_BASE_UnicodeCharacterType_h

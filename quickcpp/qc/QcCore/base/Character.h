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
// Class: Character
// 
//==============================================================================

#ifndef QC_BASE_Character_h
#define QC_BASE_Character_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include "String.h"

#include "IllegalArgumentException.h"
#include "IllegalCharacterException.h"
#include "SystemCodeConverter.h"
#include "NullPointerException.h"
#include "UnicodeCharacterType.h"
#include "debug.h"

QC_BASE_NAMESPACE_BEGIN

class QC_BASE_PKG Character
{
public:

	Character();
	Character(const Character& rhs);
	Character(const CharType* pSeqStart, size_t len);
	//explicit Character(const String& str);
	explicit Character(UCS4Char ch);

	Character& operator=(const Character& rhs);

	bool operator==(const Character& rhs) const;
	bool operator!=(const Character& rhs) const;

	bool operator==(CharType c) const;
	bool operator!=(CharType c) const;

	const CharType* data() const;
	const size_t length() const;
	const CharType first() const;
	UCS4Char toUnicode() const;
	String toString() const;
	void appendToString(String&) const;

	bool isAscii() const;
	bool isDigit() const;
	bool isLetter() const;
	bool isEOF() const;
	bool isHexDigit() const;
	bool isSpace() const;

	static bool isAscii(CharType c);
	static bool isDigit(CharType c);
	static bool isLetter(CharType c);
	static bool isHexDigit(CharType c);
	static bool isSpace(CharType c);

#if defined(QC_UTF8)
	enum {MaxSeqLen=4};
#elif defined(QC_UTF16)
	enum {MaxSeqLen=2};
#elif defined(QC_UCS4) || defined(QC_UCS2)
	enum {MaxSeqLen=1};
#else
#error Internal character encoding not correctly specified
#endif

	static Character EndOfFileCharacter;

private:
	void copyData(const CharType* pData);

private:
	CharType m_data[MaxSeqLen];
	int m_length;	
};

//==============================================================================
// Character::Character
//
/**
Default constructor.

Creates a Character that is equivalent to the EndOfFile character.
*/
//==============================================================================
inline
	Character::Character() : 
m_length(-1)
{}

//==============================================================================
// Character::Character
//
/**
Constructs a Character given a pointer to the first member of a multi-character
sequence and its maximum length.

A multi-character sequence consists of one or more ::CharType
characters that, taken together, represent a single Unicode character.

The sequence, including the first ::CharType character and any 
trailing characters are copied into the internal multi-character sequence.

@param pSeqStart a pointer to the first character of a multi-character sequence
that represents a single Unicode character.
@param len the number of ::CharType characters that are legally
addressable within the array starting at @c pSeqStart

@throws NullPointerException if @c pSeqStart is null.
@throws IllegalCharacterException if the array starting at @c pSeqStart
does not represent a valid Unicode character in the internal encoding
*/
//==============================================================================
inline 
	Character::Character(const CharType* pSeqStart, size_t len)
{
	if(!pSeqStart) throw NullPointerException();

	m_length = SystemCodeConverter::GetCharSequenceLength(*pSeqStart);
	QC_DBG_ASSERT(m_length!=0);

	if((int)len < m_length) throw IllegalCharacterException();

	if(!SystemCodeConverter::IsValidCharSequence(pSeqStart, m_length))
		throw IllegalCharacterException();

	copyData(pSeqStart);
}

//==============================================================================
// Character::Character
//
/**
Constructs a Character from the first Unicode character in @c str.

The String @c str must contain one or more ::CharType characters.  The first
character represents the first character in a multi-character sequence.

A multi-character sequence consists of one or more ::CharType
characters that, taken together, represent a single Unicode character.

The entire sequence, including the first ::CharType character and any 
trailing characters are copied into the internal multi-character sequence.

@throws IllegalArgumentException if @c str is empty
@throws IllegalCharacterException if @c str does not contain sufficient
::CharType characters to complete a single multi-character sequence
or does not represent a properly encoded Unicode character
*/
//==============================================================================
/*
inline 
Character::Character(const String& str)
{
QC_DBG_ASSERT(!str.empty());
if(str.empty()) throw IllegalArgumentException();
const CharType* pSeqStart=str.data();
QC_DBG_ASSERT(pSeqStart);
m_length = SystemCodeConverter::GetCharSequenceLength(*pSeqStart);
QC_DBG_ASSERT(m_length);

if((int)str.size() < m_length) throw IllegalCharacterException();

if(!SystemCodeConverter::IsValidCharSequence(pSeqStart, m_length))
throw IllegalCharacterException();

copyData(pSeqStart);
}*/

//==============================================================================
// Character::Character
//
/**
Copy constructor.  Constructs a Character with the same value as @c rhs.
@param rhs the Character to copy
*/
//==============================================================================
inline
	Character::Character(const Character& rhs) :
m_length(rhs.m_length)
{
	if(m_length != -1)
		copyData(rhs.m_data);
}

//==============================================================================
// Character::operator=
//
/**
Assignment operator.  Sets this Character equal to @c rhs.

@returns a reference to this Character.
*/
//==============================================================================
inline
	Character& Character::operator=(const Character& rhs)
{
	m_length = rhs.m_length;
	if(m_length != -1)
		copyData(rhs.m_data);
	return *this;
}

//==============================================================================
// Character::operator==
//
/**
Equality operator.

Tests if the internal multi-character sequence has a length of 1 and the
first member is equal to @c c.

@returns true if the Unicode character represented by this Character is
equal to the ::CharType character @c c; false otherwise
*/
//==============================================================================
inline 
	bool Character::operator==(CharType c) const
{
	return (m_length == 1 && m_data[0] == c);
}

//==============================================================================
// Character::operator!=
//
/**
Inequality operator.

Tests if the internal multi-character sequence has a length other than 1 or the
first member is @em not equal to @c c.

@returns true if the Unicode character represented by this Character is
equal to the single ::CharType character @c c; false otherwise
*/
//==============================================================================
inline
	bool Character::operator!=(CharType c) const
{
	return (m_length != 1 || m_data[0] != c);
}

//==============================================================================
// Character::data
//
/**
Returns a pointer to the controlled ::CharType character sequence buffer.

@returns a pointer to the controlled character sequence.
@sa length()
*/
//==============================================================================
inline 
	const CharType* Character::data() const
{
	return m_data;
}

//==============================================================================
// Character::length
//
/**
Returns the number of ::CharType characters in the controlled
character sequence.

@returns the length of the controlled character sequence.
@sa data()
*/
//==============================================================================
inline 
	const size_t Character::length() const
{
	return (m_length > 0) ? m_length : 0;
}

//==============================================================================
// Character::first
//
/**
Returns the first ::CharType character in the controlled sequence.

@returns the first ::CharType character in the controlled sequence.

@throws IllegalCharacterException if this Character does not represent
a valid Unicode character in the range U+0000-U+10FFFF.
*/
//==============================================================================
inline 
	const CharType Character::first() const
{
	QC_DBG_ASSERT(m_length!=0);
	if(m_length < 1) throw IllegalCharacterException();
	return m_data[0];
}

//==============================================================================
// Character::toUnicode
//
/**
Converts the controlled multi-character sequence into a 32-bit Unicode 
code-point value.

@returns the Unicode character represented by this Character as a 32-bit
value.
@throws IllegalCharacterException if this Character does not represent
a valid Unicode character in the range U+0000-U+10FFFF.
*/
//==============================================================================
inline 
	UCS4Char Character::toUnicode() const
{
	if(1 == m_length)
		return m_data[0];
	else if(-1 == m_length)
		return 0xFFFFUL;

	UCS4Char ch(0);
	const CharType* pNext;
	const CharType* pEnd = m_data+MaxSeqLen;
	SystemCodeConverter::Result res = 
		SystemCodeConverter::FromInternalEncoding(ch, m_data, pEnd, pNext);
	if(res != SystemCodeConverter::ok) throw IllegalCharacterException();
	return ch;
}

//==============================================================================
// Character::isAscii
//
/**
Tests if the Unicode character represented by this Character is in the ASCII
range U+0000-U+007F.

@returns true if this Character is in the ASCII range; false otherwise.
@sa UnicodeCharacterType::IsAscii()
*/
//==============================================================================
inline
	bool Character::isAscii() const
{
	return (m_length == 1 && UnicodeCharacterType::IsAscii(m_data[0]));
}

//==============================================================================
// Character::isDigit
//
/**
Tests if the Unicode character represented by this Character represents
an ASCII decimal digit 0-9.

@returns true if this Character is a decimal digit [0-9]; false otherwise.
@sa UnicodeCharacterType::IsDigit()
*/
//==============================================================================
inline
	bool Character::isDigit() const
{
	return (m_length == 1 && UnicodeCharacterType::IsDigit(m_data[0]));
}

inline 
	bool Character::isLetter() const
{
	return (m_length == 1 && UnicodeCharacterType::IsLetter(m_data[0]));
}

//==============================================================================
// Character::isSpace
//
/**
Tests if the Unicode character represented by this Character represents
white-space according to common Windows and Unix conventions.

Space characters are:-
- <tt>'\t' U+0009 HORIZONTAL TABULATION</tt>
- <tt>'\\n' U+000A NEW LINE</tt>
- <tt>'\f' U+000C FORM FEED</tt> 
- <tt>'\r' U+000D CARRIAGE RETURN</tt>  
- <tt>' '   U+0020 SPACE</tt> 

@returns true if this Character is a space character; false otherwise.
@sa UnicodeCharacterType::IsSpace()
*/
//==============================================================================
inline
	bool Character::isSpace() const
{
	return (m_length == 1 && UnicodeCharacterType::IsSpace(m_data[0]));
}

//==============================================================================
// Character::isHexDigit
//
/**
Tests if the Unicode character represented by this Character represents
an ASCII hexadecimal digit [0-9], [A-F], [a-f].

@returns true if this Character is a hexadecimal digit; false otherwise.
@sa UnicodeCharacterType::IsHexDigit()
*/
//==============================================================================
inline
	bool Character::isHexDigit() const
{
	return (m_length == 1 && UnicodeCharacterType::IsHexDigit(m_data[0]));
}

//==============================================================================
// Character::isEOF
//
/**
Tests if this Character is equal to the special Character: 
Character::EndOfFileCharacter.

Functions that read a character stream and return a Character need a method
to indicate that the end of stream has been reached.  To achieve this they
return a special Character with a unique value that is different from all valid 
Unicode characters.

@returns true if this Character is equal to the Character::EndOfFileCharacter;
false otherwise.
*/
//==============================================================================
inline
	bool Character::isEOF() const
{
	return (-1 == m_length);
}

inline
	bool Character::isAscii(CharType c)
{
	return (UnicodeCharacterType::IsAscii(c));
}

inline
	bool Character::isDigit(CharType c)
{
	return (UnicodeCharacterType::IsDigit(c));
}

inline
	bool Character::isLetter(CharType c)
{
	return (UnicodeCharacterType::IsLetter(c));
}

inline
	bool Character::isHexDigit(CharType c)
{
	return (UnicodeCharacterType::IsHexDigit(c));
}

inline
	bool Character::isSpace(CharType c)
{
	return (UnicodeCharacterType::IsSpace(c));
}

//==============================================================================
// Character::appendToString
//
/**
Appends the multi-character sequence controlled by this Character to the 
passed String @c str.
@param str the String which will have this Character appended
*/
//==============================================================================
inline
	void Character::appendToString(String& str) const
{
	str.append(m_data, m_length);
}

//==============================================================================
// Character::toString
//
/**
Returns the multi-character sequence controlled by this Character as a
String.

@returns a String with the same sequence of ::CharType characters.
*/
//==============================================================================
inline
	String Character::toString() const
{
	return String(m_data, m_length);
}

//==============================================================================
// Character::operator==
//
/**
Equality operator.

Tests if the Unicode character represented by this is the same 
Unicode character as @c rhs;

@returns true if the Unicode character represented by this Character is
equal to the Unicode character @c rhs; false otherwise
*/
//==============================================================================
inline
	bool Character::operator==(const Character& rhs) const
{
	if(m_length!=rhs.m_length) 
		return false;

	// eof is eof regardless of data contents
	if(m_length == -1) 
		return true;

#if defined(QC_UCS4) || defined(QC_UCS2)

	return m_data[0] == rhs.m_data[0];

#else // !(QC_UCS4 || QC_UCS2)

	switch(m_length)
	{

		// UTF-8 is the only internal encoding that can take > 2 character positions
#if defined(QC_UTF8) 

	case 4: if(m_data[3] != rhs.m_data[3]) return false;
	case 3: if(m_data[2] != rhs.m_data[2]) return false;

#endif //QC_UTF8

	case 2: if(m_data[1] != rhs.m_data[1]) return false;
	default:
	case 1: if(m_data[0] != rhs.m_data[0]) return false;
	}

	return true;

#endif //QC_UCS4

}

//==============================================================================
// Character::operator!=
//
/**
Inequality operator.

Tests if the Unicode character represented by this is @em not the same 
Unicode character as @c rhs;

@returns false if the Unicode character represented by this Character is
equal to the Unicode character @c rhs; true otherwise
*/
//==============================================================================
inline
	bool Character::operator!=(const Character& rhs) const
{
	return !(*this == rhs);
}

//==============================================================================
// Character::copyData
//
// Private implementation to copy a sequence to the internal buffer.  Optimized
// for the various supported encoding schemes.
//==============================================================================
inline
	void Character::copyData(const CharType* pData)
{
	QC_DBG_ASSERT(pData!=0 && m_length > 0);
	if(!pData) throw NullPointerException();

#if defined(QC_UCS4) || defined(QC_UCS2)

	m_data[0] = *pData;

#else // !(QC_UCS4 || QC_UCS2)

	pData += m_length;
	switch(m_length)
	{

		// UTF-8 is the only internal encoding that can take > 2 character positions
#if defined(QC_UTF8) 

	case 4: m_data[3] = *--pData;
	case 3: m_data[2] = *--pData;

#endif //QC_UTF8

	case 2: m_data[1] = *--pData;
	default:
	case 1: m_data[0] = *--pData;
	}

#endif //QC_UCS4

}

QC_BASE_NAMESPACE_END

#endif //QC_BASE_Character_h

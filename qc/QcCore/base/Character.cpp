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
// Class Character
/**
	@class qc::Character
	
	@brief Represents a Unicode character using an internal sequence of one or more 
	::CharType characters.

	It provides optimized routines for converting Unicode characters into
	a sequence of one or more ::CharType characters and for decoding a multi-character
	sequence into a Unicode code-point ::UCS4Char value.

    The Character class also contains a number of convenient methods for
	querying the characteristics of the encoded Unicode character.  These routines
	such as isHexDigit() and isSpace() are simply wrappers for functions in the 
	Unicode class.  They have counterparts in the standard C++ library,
	but the standard library routines rely on the capabilities of a locale
	which may not be available for Unicode.  The Unicode class does
	not suffer from this drawback.
*/
//==============================================================================

#include "Character.h"
#include "SystemCodeConverter.h"
#include "IllegalCharacterException.h"
#include "UnicodeCharacterType.h"

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// Character::EndOfFileCharacter
//
/**
   Character representing the 'end of file' condition.

   This is a special Character that can be returned from functions that read
   a single Character when the end of file condition has been reached.
*/
//==============================================================================
Character Character::EndOfFileCharacter;

//==============================================================================
// Character::Character
//
/**
   Constructs a Character with an internal ::CharType sequence equivalent to 
   the Unicode character represented by the value of @c ch.

   @throws IllegalCharacterException if @c ch is not a legal Unicode character
           in the range U+0000-U+10FFFF.
*/
//==============================================================================
Character::Character(UCS4Char ch)
{
	if(ch > 0x10FFFFUL) throw IllegalCharacterException();

	CharType* pNext;
	const CharType* pEnd = m_data+MaxSeqLen;
	SystemCodeConverter::Result res = 
		SystemCodeConverter::ToInternalEncoding(ch, m_data, pEnd, pNext);
	if(res == SystemCodeConverter::ok)
	{
		m_length = (pNext-m_data);
	}
	else
	{
		throw IllegalCharacterException();
	}
}

QC_BASE_NAMESPACE_END

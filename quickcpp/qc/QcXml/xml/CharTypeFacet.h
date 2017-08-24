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
// Class: CharTypeFacet
// 
// Overview
// --------
// The xml specification provides character classifications that are much
// richer than the classifications provided by the standard c++ library.
//
// This class provides an efficient table-driven mechanism to determine
// the classification of a single character.
//
//==============================================================================

#ifndef QC_XML_CharTypeFacet_h
#define QC_XML_CharTypeFacet_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

QC_XML_NAMESPACE_BEGIN


class QC_XML_PKG CharTypeFacet
{
public:
	
	typedef unsigned char Mask;

	enum mask {None=0x00,
	           NameStartChar=0x01,
	           NameChar=0x02,
			   Whitespace=0x04,
			   ValidChar=0x08,
			   NonDataChar=0x10,
			   TagStartChar=0x20,
			   PubidChar=0x40,
			   PEStartChar=0x80,
			   Any=0xFF};

	static bool IsWhiteSpace(const Character& ch);
	static bool IsNameStartChar(const Character& ch);
	static bool IsNameChar(const Character& ch);
	static bool IsValidChar(const Character& ch);
	static bool IsCharType(const Character& ch, Mask mask);
	static bool IsCharType(const Character& ch, Mask includeMask, Mask excludeMask);
	static bool IsCharType(const String& str, Mask mask);
	static bool IsValidName(const String& str, bool bName);

	static unsigned char s_XMLTable[];
	static const UCS4Char s_MaxChar;
	static const size_t s_CharTabSize;
};

inline bool CharTypeFacet::IsCharType(const Character& ch, Mask mask)
{
	UCS4Char inChar = ch.toUnicode();
	if(inChar < s_CharTabSize)
		return (s_XMLTable[inChar] & mask) != 0; 
	else
		if(inChar <= s_MaxChar)
			return ((mask & ValidChar) != 0);
		else
			return false;
}

inline bool CharTypeFacet::IsCharType(const Character& ch, Mask includeMask, Mask excludeMask)
{
	UCS4Char inChar = ch.toUnicode();
	if(inChar < s_CharTabSize)
	{
		unsigned char entry = s_XMLTable[inChar];
		return ((entry & includeMask) != 0 && (entry & excludeMask) == 0); 
	}
	else
		if(inChar <= s_MaxChar)
			return (includeMask & ValidChar) != 0;
		else
			return false;
}

inline bool CharTypeFacet::IsNameChar(const Character& ch)
{
	UCS4Char inChar = ch.toUnicode();
	if(inChar < s_CharTabSize)
		return (s_XMLTable[inChar] & NameChar) != 0;
	else
		return false;
}

inline bool CharTypeFacet::IsValidChar(const Character& ch)
{
	const UCS4Char x = ch.toUnicode();
	if(x < s_CharTabSize)
		return (s_XMLTable[x] & ValidChar) != 0;
	else
		return (x <= s_MaxChar); 
}

inline bool CharTypeFacet::IsNameStartChar(const Character& ch)
{
	const UCS4Char x = ch.toUnicode();
	if(x < s_CharTabSize)
		return (s_XMLTable[x] & NameStartChar) != 0;
	else
		return false;
}

inline bool CharTypeFacet::IsWhiteSpace(const Character& ch)
{
	const UCS4Char x = ch.toUnicode();
	if(x < s_CharTabSize)
		return (s_XMLTable[x] & Whitespace) !=0;
	else
		return false;
}

inline bool CharTypeFacet::IsCharType(const String& str, Mask mask)
{
	const CharType* pStr = str.data();
	const CharType* pEnd = pStr+str.length();

	while(pStr < pEnd)
	{
		Character ch(pStr, (pEnd-pStr));
		if(!IsCharType(ch, mask))
		{
			return false;
		}
		pStr+=ch.length();
	}
	return (!str.empty());
}

inline bool CharTypeFacet::IsValidName(const String& str, bool bName)
{
	return (IsCharType(str, CharTypeFacet::NameChar) 
		&& (!bName || IsNameStartChar(Character(str.data(), str.size())) ));
}

QC_XML_NAMESPACE_END

#endif //QC_XML_CharTypeFacet_h


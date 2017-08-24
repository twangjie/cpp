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
// Class StringTokenizer
/**
	@class qc::util::StringTokenizer
	
	@brief Splits a String into a sequence of delimited tokens.

	A StringTokenizer is constructed using a String together with a set
	of delimiter characters contained in another String.  The StringTokenizer will
	split the controlled String into a sequence of tokens, each token separated
	by one or more characters from the set of delimiter characters.

	The StringTokenizer can behave in one of three ways, depending on
	the values of the @c bReturnDelims and @c bReturnContiguousDelims 
	constructor flags:

	- If the @c bReturnDelims flag is @c false, delimiter characters
	  are used to separate tokens but they are not returned as tokens themselves.
	- If the @c bReturnDelims flag is @c true and @c bReturnContiguousDelims is
	  @c false then individual delimiter characters are considered to be tokens in 
	  their own right. 
	- If the @c bReturnDelims flag is @c true and @c bReturnContiguousDelims is
	  also @c true then contiguous sequences of delimiter characters are returned
	  as a single token. 

	A token is thus either a sequence of consecutive characters that are not
	delimiters, a single delimiter character or a sequence of characters that
	are delimiters.

	The following example splits a string containing words separated by
	space characters into a number of word tokens.  Delimiters are not returned
	in this case:

    @code
    StringTokenizer tokenizer(QC_T("this is a test"));
    while (tokenizer.hasMoreTokens())
    {
        Console::cout() << tokenizer.nextToken() << endl;
    }
    @endcode


	In common with all String handling in @QuickCPP, the controlled
	String and the set of delimiter characters may contain Unicode characters that
	have been encoded into a multi-character sequence.  The StringTokenizer
	class correctly treats multi-character sequences as single Unicode
	characters for the purposes of comparison between characters in the
	controlled String and the set of delimiter characters.
*/
//==============================================================================

#include "StringTokenizer.h"
#include "stlutils.h"

#include "QcCore/base/StringUtils.h"
#include "QcCore/base/StringIterator.h"

#include <algorithm>
#include <functional>

QC_UTIL_NAMESPACE_BEGIN

const String strWhitespace = QC_T(" \t\r\n");

//==============================================================================
// StringTokenizer::StringTokenizer
//
/**
   Constructs a StringTokenizer with the controlled String @c str using a
   default white-space string as the delimiter.
   
   The default delimiter contains the white-space characters making white-space
   according to the description of UnicodeCharacterType::IsSpace().
   
   @param str The String to tokenize
*/
//==============================================================================
StringTokenizer::StringTokenizer(const String& str) :
	m_bReturnDelims(false),
	m_bReturnContiguousDelims(false),
	m_bContainsMultiCharSequence(false),
	m_delim(strWhitespace),
	m_nextPos(0),
	m_str(str)
{
	m_nextPos = locateNextToken(false, 0);
}

//==============================================================================
// StringTokenizer::StringTokenizer
//
/**
   Constructs a StringTokenizer with the controlled String @c str using all the
   Unicode characters from @c delim as delimiters.

   @param str The String to tokenize
   @param delim A String containing a set of Unicode characters to be used as
          token delimiters
   @param bReturnDelims when set to @c true, calls to nextToken() will return
          the delimiter characters as tokens in their own right.
          See the @c bReturnContiguousDelims parameter for how the delimiter
          tokens can be further controlled.
   @param bReturnContiguousDelims controls how delimiter characters are
          grouped into tokens.  When set to @c true
          calls to nextToken() will return a maximal sequence of delimiter
          characters as a single token.  When set to @c false, each delimeter
          character will be returned as an individual token.  This parameter
          has no effect when the @c bReturnDelims parameter is set to @c false.
*/
//==============================================================================
StringTokenizer::StringTokenizer(const String& str,
                                 const String& delim,
                                 bool bReturnDelims,
                                 bool bReturnContiguousDelims) :
	m_bReturnDelims(bReturnDelims),
	m_bReturnContiguousDelims(bReturnContiguousDelims),
	m_bContainsMultiCharSequence(StringUtils::ContainsMultiCharSequence(delim)),
	m_delim(delim),
	m_nextPos(0),
	m_str(str)
{
	if(bReturnDelims)
	{
		if(str.empty())
		{
			m_nextPos = String::npos;
		}
	}
	else
	{
		m_nextPos = locateNextToken(false, 0);
	}
}

//==============================================================================
// StringTokenizer::peekNextToken
//
/**
   Returns the next token without advancing the position of the StringTokenizer.

   This method returns the same value as nextToken() but without advancing the
   position past the token.

   @returns A String containing the next token or delimiter.  An empty string
            indicates the end of the String has been reached.

   @sa nextToken.
*/
//==============================================================================
String StringTokenizer::peekNextToken() const
{
	size_t oldPos = m_nextPos;
	StringTokenizer* ncThis = const_cast<StringTokenizer*>(this);
	String ret = ncThis->nextToken();
	ncThis->m_nextPos = oldPos;
	return ret;
}

//==============================================================================
// StringTokenizer::nextToken
//
/**
   Returns the next token from the controlled String.

   See the class description for details about how delimiters may
   be returned as tokens.

   When all the tokens have been exhausted, an empty String is returned.

   @returns A String containing the next token or delimiter.  An empty string
            indicates that the end of the String has been reached.
   
   @sa peekNextToken()
*/
//==============================================================================
String StringTokenizer::nextToken()
{
	String::size_type tokenPos = locateNextToken(false, m_nextPos);

	if(m_bReturnDelims && tokenPos != m_nextPos)
	{
		String::size_type oldPos = m_nextPos;

		if(m_bReturnContiguousDelims)
		{
			m_nextPos = tokenPos;
		}
		else
		{
			m_nextPos++;
			if(m_nextPos >= m_str.size())
				m_nextPos = String::npos;
		}
		
		return m_str.substr(oldPos, m_nextPos==String::npos ? m_nextPos : m_nextPos-oldPos);
	}
	else if(tokenPos != String::npos)
	{
		String::size_type delimPos = locateNextToken(true, tokenPos);
		
		m_nextPos = m_bReturnDelims 
		          ? delimPos
		          : locateNextToken(false, delimPos);

		return m_str.substr(tokenPos, delimPos==String::npos ? delimPos : delimPos-tokenPos);
	}
	else
	{
		m_nextPos = String::npos;
		return String();
	}
}

//==============================================================================
// StringTokenizer::hasMoreTokens
//
/**
   Tests to see if more tokens exist in the controlled String.

   @returns @c true if a call to nextToken() will yield a non-empty String;
            @c false otherwise
*/
//==============================================================================
bool StringTokenizer::hasMoreTokens() const
{
	return (m_nextPos != String::npos);
}

//==============================================================================
// StringTokenizer::toVector
//
/**
   Returns the remaining tokens as elements of a std::vector<String>.  
   On return from this function all the tokens from the controlled String will
   have been processed.

   This function is implemented as if nextToken() is called until it returns
   an empty String, adding each returned token to the vector.  Therefore,
   if the StringTokenizer is set to return delimiters as tokens, the returned
   vector will contain the delimiters also.

   @returns a vector containing the remaining tokens from the controlled String.
*/
//==============================================================================
std::vector<String> StringTokenizer::toVector()
{
	std::vector<String> vec;
	while(hasMoreTokens())
	{
		vec.push_back(nextToken());
	}
	return vec;
}

//==============================================================================
// StringTokenizer::locateNextToken
//
// Private helper function used to locate the next token in the controlled
// String.
//
// This is optimized for delimiter strings that do not contain multi-character
// sequences (they can use the ByteString find methods).
//==============================================================================
String::size_type StringTokenizer::locateNextToken(bool bDelim, String::size_type pos) const
{
	if(pos != String::npos)
	{
		if(m_bContainsMultiCharSequence)
		{
			StringIterator delimStart(m_delim.data());
			StringIterator delimEnd(m_delim.data()+m_delim.size());
			StringIterator seqStart(m_str.data()+pos);
			StringIterator seqEnd(m_str.data()+m_str.size());
			StringIterator next = (bDelim)
				? std::find_first_of(seqStart, seqEnd, delimStart, delimEnd)
				: find_first_not_of(seqStart, seqEnd, delimStart, delimEnd);

			if(next == seqEnd)
				return String::npos;
			else
				return (next.data() - m_str.data());
		}
		else
		{
			return (bDelim)
				? m_str.find_first_of(m_delim, pos)
				: m_str.find_first_not_of(m_delim, pos);
		}
	}
	else
	{
		return pos;
	}
}

QC_UTIL_NAMESPACE_END

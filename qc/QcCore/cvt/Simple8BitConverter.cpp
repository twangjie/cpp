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
// Class: Simple8BitConverter
// 
// There are many character encodings that make use of 8-bit "octets"
// mapping the 256 values to specific Unicode characters.
//
// This class implements a general-purpose, table-driven approach
// to encoding and decoding these values.
//
//==============================================================================

#include "Simple8BitConverter.h"

#include "QcCore/base/SystemCodeConverter.h"

QC_CVT_NAMESPACE_BEGIN

const Simple8BitConverter::CodedChar UNDEFINED = 0xFFFF;

Simple8BitConverter::Simple8BitConverter(const String& name,
                                         const CodedChar* decodingTable) :
	m_pDecodingTable(decodingTable),
	m_name(name)
{
	generateEncodingMap();
}

//==============================================================================
// Simple8BitConverter::generateEncodingMap
//
//==============================================================================
void Simple8BitConverter::generateEncodingMap()
{
	for(size_t i=0; i<256; ++i)
	{
		const CodedChar theChar = m_pDecodingTable[i];
		if(theChar != UNDEFINED)
		{
			m_encodingMap[theChar] = Byte(i);
		}
	}
}

//==============================================================================
// Simple8BitConverter::decode
//
//==============================================================================
CodeConverter::Result Simple8BitConverter::decode(const Byte *from, const Byte *from_end,
                                                  const Byte *& from_next,
                                                  CharType *to, CharType *to_limit,
                                                  CharType *& to_next)
{
	from_next = from, to_next = to;
	Result ret = ok;
	//
	// decode Bytes into Characters until 
	// one of the buffers is exhausted
	//
	while(ret == ok && from_next < from_end && to_next < to_limit)
	{
		const CodedChar nextChar = m_pDecodingTable[*from_next];
		if(nextChar == UNDEFINED)
		{
			if(getInvalidCharAction() == abort)
			{
				handleInvalidByteSequence(from_next, 1);
				ret = error;
			}
			else
			{
				QC_DBG_ASSERT(getInvalidCharAction() == replace);
				if( (ret=SystemCodeConverter::ToInternalEncoding(
					getInvalidCharReplacement(), to_next, to_limit, to_next))==ok)
				{
					++from_next;
				}
			}
		}
		else
		{
			if( (ret=SystemCodeConverter::ToInternalEncoding(nextChar, to_next, to_limit, to_next))==ok)
			{
				++from_next;
			}
		}
	}

	return (ret); 
}

//==============================================================================
// Simple8BitConverter::encode
//
//==============================================================================
CodeConverter::Result Simple8BitConverter::encode(const CharType *from, const CharType *from_end,
                                                  const CharType *& from_next,
                                                  Byte *to, Byte *to_limit,
                                                  Byte *& to_next)
{
	Result ret = ok;
	from_next = from, to_next = to;

	//
	// encode Characters into Bytes until 
	// one of the buffers is exhausted
	//
	while(from_next < from_end && to_next < to_limit && ret == ok)
	{
		UCS4Char ch;
		const CharType * from_next_copy;
		if( (ret=SystemCodeConverter::FromInternalEncoding(ch, from_next, from_end, from_next_copy)) == ok)
		{
			EncodingMap::const_iterator iter = m_encodingMap.find(CodedChar(ch));
			if(iter != m_encodingMap.end())
			{
				*to_next++ = (*iter).second;
				from_next = from_next_copy;
			}
			else // if the character is not in the map then we have an error
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
	
	return ret;
}

//==============================================================================
// Simple8BitConverter::getDecodedLength
//
// Returns the number of Unicode characters that an external array of bytes will
// generate once decoded.
//==============================================================================
size_t Simple8BitConverter::getDecodedLength(const Byte *from, const Byte *from_end) const
{
	return (from_end - from);
}

bool Simple8BitConverter::alwaysNoConversion() const
{
	return (false);
}

size_t Simple8BitConverter::getMaxEncodedLength() const
{
	return 1;
}

//==============================================================================
// Simple8BitConverter::getEncodingName
//
//==============================================================================
String Simple8BitConverter::getEncodingName() const
{
	return m_name;
}

QC_CVT_NAMESPACE_END


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
// Class: ASCII8BitConverter
// 
// There are many character encodings that make use of 8-bit "octets"
// mapping the 256 values to specific Unicode characters.
//
// This class implements a general-purpose, table-driven approach
// to encoding and decoding these values.
//
//==============================================================================

#include "ASCII8BitConverter.h"

#include "QcCore/base/SystemCodeConverter.h"

QC_CVT_NAMESPACE_BEGIN

const ASCII8BitConverter::CodedChar UNDEFINED = 0xFFFF;

ASCII8BitConverter::ASCII8BitConverter(const String& name,
                                       const CodedChar decodingTable[128]) :
	m_pDecodingTable(decodingTable),
	m_name(name)
{
	generateEncodingMap();
}

//==============================================================================
// ASCII8BitConverter::generateEncodingMap
//
//==============================================================================
void ASCII8BitConverter::generateEncodingMap()
{
	for(size_t i=0; i<128; ++i)
	{
		const CodedChar theChar = m_pDecodingTable[i];
		if(theChar != UNDEFINED)
		{
			m_encodingMap[theChar] = Byte(i+128);
		}
	}
}

//==============================================================================
// ASCII8BitConverter::decode
//
//==============================================================================
CodeConverter::Result ASCII8BitConverter::decode(const Byte *from, const Byte *from_end,
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
		if(*from_next & 0x80)
		{
			const CodedChar nextChar = m_pDecodingTable[(*from_next) & 0x7F];

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
		else
		{
			//
			// No need to convert to internal because all internal encodings
			// are the same in the ascii range
			//
			*to_next++ = *from_next++;
		}
	}

	return (ret); 
}

//==============================================================================
// ASCII8BitConverter::encode
//
//==============================================================================
CodeConverter::Result ASCII8BitConverter::encode(const CharType *from, const CharType *from_end,
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
			if(ch > 0x7F)
			{
				EncodingMap::const_iterator iter;

				if((ch <= 0xFFFFU) && ( (iter = m_encodingMap.find(CodedChar(ch))) != m_encodingMap.end()))
				{
					*to_next++ = (*iter).second;
					from_next = from_next_copy;
				}
				else // the character is not in the map
				{
					if( (ret = handleUnmappableCharacter(ch, to_next, to_limit, to_next)) == ok)
					{
						from_next = from_next_copy;
					}
				}
			}
			else
			{
				*to_next++ = Byte(*from_next++);
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
// ASCII8BitConverter::getDecodedLength
//
// Returns the number of Unicode characters that an external array of bytes will
// generate once decoded.
//==============================================================================
size_t ASCII8BitConverter::getDecodedLength(const Byte *from, const Byte *from_end) const
{
	return (from_end - from);
}

bool ASCII8BitConverter::alwaysNoConversion() const
{
	return (false);
}

size_t ASCII8BitConverter::getMaxEncodedLength() const
{
	return 1;
}

//==============================================================================
// ASCII8BitConverter::getEncodingName
//
//==============================================================================
String ASCII8BitConverter::getEncodingName() const
{
	return m_name;
}

QC_CVT_NAMESPACE_END


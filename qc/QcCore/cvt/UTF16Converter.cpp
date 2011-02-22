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
// Class: UTF16Converter
//
// See RFC2781 For a fuller description.
// 
// UTF-16 is described in the Unicode Standard, version 3.0.
//
// UTF-16 can encode characters in the range 0x00 - 0x10FFFF.  Characters
// up to 0xFFFF are encoded into two 8-bit octets (bytes).  Characters
// above 0xFFFF are written into 4 bytes, effectively piggy-backing on top
// of a reserved range of characters between 0xD800 - 0xDFFF.  This range
// provides 10 bits of information.  Therefore two such ranges gives 20 bits
// which is more than sufficient to encode the characters between 0x10000 and
// 0x10FFFF.
//
// Depending on the architecture of the machine that created the file,
// the bytes may be written in little-endian or big-endian order.
//
//==============================================================================

#include "UTF16Converter.h"

#include "QcCore/io/IOException.h"
#include "QcCore/base/SystemCodeConverter.h"

QC_CVT_NAMESPACE_BEGIN

using io::IOException;

const unsigned long maxUTF16 = 0x10FFFFUL;

UTF16Converter::UTF16Converter(Endianness endianness) : 
	m_endianness(endianness),
	m_bBOMRequired(true)
{
}

UTF16Converter::UTF16Converter() : 
	m_endianness(unspecified),
	m_bBOMRequired(true)
{
}

//==============================================================================
// UTF16Converter::decode
//
//==============================================================================
CodeConverter::Result UTF16Converter::decode(const Byte *from, const Byte *from_end,
                                             const Byte *& from_next,
                                             CharType *to, CharType *to_limit,
                                             CharType *& to_next)
{
	from_next = from, to_next = to;

	Result ret = ok;
	//
	// decode Bytes into Characters until an error
	// or one of the buffers is exhausted
	//
	while(ret == ok && from_next < from_end && to_next < to_limit)
	{
		if(from_next+1 == from_end)
		{
			ret = inputExhausted;
		}
		else
		{
			//
			// If we haven't been told whether the byte stream is big- or little-
			// endian then we must be given a Byte Order Mark (BOM).  If so
			// we eat it - it does not generate a character to be returned to the
			// application.
			//
			if(m_endianness==unspecified)
			{
				readBOM(from_next, from_end, from_next);
				continue;
			}

			//
			// decode the first 2 bytes
			//
			unsigned short W1 = (m_endianness==big_endian)
			                  ? *from_next << 8 | *(from_next+1) 
							  : *(from_next+1) << 8 | *from_next;

			//
			// If the character is in the so-called surrogate area
			// (0xD800 - 0xDFFF) then we need another 2 bytes...
			// A mask can be used to test for this
			// 0xD800 = 1101 1000 0000 0000
			// 0xDFFF = 1101 1111 1111 1111
			// mask     1111 1000 0000 0000 = 0xF800
			//
			if((W1 & 0xF800) != 0xD800) // ie not a surrogate
			{
				if( (ret=SystemCodeConverter::ToInternalEncoding(W1, to_next, to_limit, to_next)) == ok)
				{
					from_next+=2;
				}
			}
			else if(from_next+3 >= from_end)
			{
				ret = inputExhausted;
			}
			//
			// The first character of a surrogate pair must be in the range
			// 0xD800 - 0xDBFF
			// A mask can be used to test for this
			// 0xD800 = 1101 1000 0000 0000
			// 0xDBFF = 1101 1011 1111 1111
			// mask     1111 1100 0000 0000 = 0xFC00
			//
			else if((W1 & 0xFC00) != 0xD800)
			{
				if(getInvalidCharAction() == abort)
				{
					handleInvalidByteSequence(from_next, 4);
					ret = error;
				}
				else
				{
					QC_DBG_ASSERT(getInvalidCharAction() == replace);
					if( (ret=SystemCodeConverter::ToInternalEncoding(
						getInvalidCharReplacement(), to_next, to_limit, to_next))==ok)
					{
						from_next+=4;
					}
				}
			}
			else
			{
				//
				// decode the second 2 bytes
				//
				unsigned short W2 = (m_endianness==big_endian)
								  ? *(from_next+2) << 8 | *(from_next+3) 
								  : *(from_next+3) << 8 | *(from_next+2);

				//
				// The second character must be in the low surrogate area
				// 0xDC00 - 0xDFFF
				// A mask can be used to test for this
				// 0xDC00 = 1101 1100 0000 0000
				// 0xDFFF = 1101 1111 1111 1111
				// mask     1111 1100 0000 0000 = 0xFC00

				// otherwise we have a bad encoding
				//
				if((W2 & 0xFC00) != 0xDC00)
				{
					if(getInvalidCharAction() == abort)
					{
						handleInvalidByteSequence(from_next, 4);
						ret = error;
					}
					else
					{
						QC_DBG_ASSERT(getInvalidCharAction() == replace);
						if( (ret=SystemCodeConverter::ToInternalEncoding(
							getInvalidCharReplacement(), to_next, to_limit, to_next))==ok)
						{
							from_next+=4;
						}
					}
				}
				else
				{
					//
					// okay - we have the two surrogate pairs.
					// Each pair contains 10 bits of information, these are
					// combined into a 20-bit number which is then added
					// to 0x10000 to get the USC4 character
					//
					UCS4Char ucsChar = ((W1 & 0x03FF) << 10) | (W2 & 0x03FF);
					ucsChar += 0x10000;

					if( (ret=SystemCodeConverter::ToInternalEncoding(ucsChar, to_next, to_limit, to_next))==ok)
					{
						from_next+=4;
					}
				}
			}
		}
	}

	return (ret); 
}

//==============================================================================
// UTF16Converter::readBOM
//
//==============================================================================
void UTF16Converter::readBOM(const Byte *from, const Byte *from_end,
                             const Byte *& from_next)
{
	// readBOM() will not be called unless there are at least 2 bytes
	// available
	QC_DBG_ASSERT(from_end-from >= 2);

	if(from[0] == 0xFE && from[1] == 0xFF)
	{
		m_endianness = big_endian;
	}
	else if(from[0] == 0xFF && from[1] == 0xFE)
	{
		m_endianness = little_endian;
	}
	else
	{
		throw IOException(QC_T("Missing UTF-16 byte-order mark"));
	}

	//
	// If we found a suitable BOM, then jog the returned Byte pointer past the BOM
	//
	from_next = from+2;
}

//==============================================================================
// UTF16Converter::encode
//
// From RFC2781:
// 1) If U < 0x10000, encode U as a 16-bit unsigned integer and terminate.
// 2) Let U' = U - 0x10000.
// 3) Initialize two 16-bit unsigned integers to 0xD800 and 0xDC00, repectively.
//    These integers each have 10 bits free to encode the character value, for
//    a total of 20 bits.
// 4) Assign the 10 high-order bits of the 20-bit U' to the 10 low-order
//    bits of W1 and the 10 low-order bits of U' to the 10 low-order bits of W2.
// Graphically, steps 2 thru 4 look like:
// U' = yyyyyyyyyyxxxxxxxxxx
// W1 = 110110yyyyyyyyyy
// W2 = 110111xxxxxxxxxx
//
// Note: unless instructed to the contrary, we write using the big-endian 
//       format.
//==============================================================================
CodeConverter::Result UTF16Converter::encode(const CharType *from, const CharType *from_end,
                                             const CharType *& from_next,
                                             Byte *to, Byte *to_limit,
                                             Byte *& to_next)
{
	from_next = from, to_next = to;

	Result ret = ok;

	//
	// UTF-16 encodings require a byte-order mark (BOM) at the start.
	//
	// In early releases of QuickCPP, this function was performed by the
	// OutputStreamWriter in collaboration with the Converter.  This has now
	// been simplified so that the Converter has sole responsibility - and Writers
	// have no knowledge of BOMs whatsoever.
	//

	if(m_bBOMRequired)
	{
		if(to_next+1 < to_limit)
		{
			m_bBOMRequired = false;

			//
			// Use the architecture of the machine to decide if o/p will
			// be in big-endian or little-endian format
			//
			if(m_endianness == unspecified)
			{
#if defined(WORDS_BIGENDIAN)
				m_endianness = big_endian;
#else
				m_endianness = little_endian;
#endif //WORDS_BIGENDIAN
			}

			if(m_endianness == little_endian)
			{
				*to_next++ = 0xFF;
				*to_next++ = 0xFE;
			}
			else
			{
				*to_next++ = 0xFE;
				*to_next++ = 0xFF;
			}
		}
		else
		{
			return outputExhausted;
		}
	}

	//
	// encode Characters into Bytes until an error
	// or one of the buffers is exhausted
	//
	while(ret == ok && from_next < from_end && (to_next+1) < to_limit)
	{
		const CharType* from_next_copy = from_next;
		UCS4Char ch;
		ret = SystemCodeConverter::FromInternalEncoding(ch, from_next, from_end, from_next_copy);
		if(ret == ok)
		{
			if(ch > 0xFFFFU)
			{
				if(to_next+4 > to_limit)
				{
					ret = outputExhausted;
				}
				else if(ch > maxUTF16)
				{
					if( (ret = handleUnmappableCharacter(ch, to_next, to_limit, to_next)) == ok)
					{
						from_next = from_next_copy;
					}
				}
				else
				{
					//
					// encode the UCS-4 value into 2 16-bit surrogate pairs
					//
					unsigned long U1 = ch - 0x10000UL;
					unsigned short W1 = UShort(0xD800 | (U1 >> 10));
					unsigned short W2 = UShort(0xDC00 | (U1 & 0x03FF));

					*to_next++ = (m_endianness != little_endian)
							? W1 >> 8
							: W1 & 0x00FF;
					*to_next++ = (m_endianness != little_endian)
							? W1 & 0x00FF
							: W1 >> 8;
					*to_next++ = (m_endianness != little_endian)
							? W2 >> 8
							: W2 & 0x00FF;
					*to_next++ = (m_endianness != little_endian)
							? W2 & 0x00FF
							: W2 >> 8;
					
					from_next = from_next_copy;
				}
			}
			else if((ch & 0xF800) == 0xD800) // ie a surrogate
			{
				if( (ret = handleUnmappableCharacter(ch, to_next, to_limit, to_next)) == ok)
				{
					from_next = from_next_copy;
				}
			}
			else
			{
				//
				// encode the value into 2-bytes
				//
				*to_next++ = (m_endianness != little_endian)
						? Byte(ch >> 8)
						: Byte(ch & 0x00FF);
				*to_next++ = (m_endianness != little_endian)
						? Byte(ch & 0x00FF)
						: Byte(ch >> 8);

				from_next = from_next_copy;
			}
		}
		else if(ret == error)
		{
			internalEncodingError(from_next, from_end-from_next);
		}
	}

	return (ret);
}

//==============================================================================
// UTF16Converter::getDecodedLength
//
// Returns the number of Unicode characters that an external array of bytes will
// generate once decoded.
//==============================================================================
size_t UTF16Converter::getDecodedLength(const Byte *from, const Byte *from_end) const
{
	int ret = 0;

    for (; from < from_end; )
	{
		unsigned short W1 = (m_endianness != little_endian)
		                  ? *from << 8 | *(from+1) 
		                  : *(from+1) << 8 | *from;

		if ((W1 & 0xF800) == 0xD800) // ie a surrogate
		{
			from += 4;
		}
		else
		{
			from += 2;
		}

		ret++;
	}
    
	return (ret);
}

//==============================================================================
// UTF16Converter::alwaysNoConversion
//
// Is this a no-op conversion? 
//==============================================================================
bool UTF16Converter::alwaysNoConversion() const
{
	// Reading UTF-16 files when the internal encoding is UTF-16 may
	// not require conversion - depending on whether the endianness of the
	// file matches the endianness of the machine.

	//
	// Note: A Reader opened with an undefined endianness (pending a BOM)
	// will not be able to answer this question affirmatively.
	//

#if defined(QC_UTF16)

	#if defined(WORDS_BIGENDIAN)
		
		return (m_endianness == big_endian);

	#else

		return (m_endianness == little_endian);

	#endif

#else // !QC_UTF16

	return false;

#endif
}

//==============================================================================
// UTF16Converter::getMaxEncodedLength
//
// Return the maximum number of Bytes required to encod a character
//==============================================================================
size_t UTF16Converter::getMaxEncodedLength() const
{
	return (4);
}

//==============================================================================
// UTF16Converter::getEncodingName
//
// Return the "canonical" name of the encoding
//==============================================================================
String UTF16Converter::getEncodingName() const
{
	switch(m_endianness)
	{
	case big_endian:
		return QC_T("UTF-16BE");
	case little_endian:
		return QC_T("UTF-16LE");
	case unspecified:
		return QC_T("UTF-16");
	}

	return String();
}


QC_CVT_NAMESPACE_END


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

#include "XMLFilterReader.h"

QC_XML_NAMESPACE_BEGIN

XMLFilterReader::XMLFilterReader(Reader* pReader) :
	FilterReader(pReader),
	m_bStandaloneCR(false)
{
}

XMLFilterReader::~XMLFilterReader()
{
}

//==============================================================================
// XMLFilterReader::read
//
// Call the underlying Reader's read method, and then normalize newlines.
//==============================================================================
IntType XMLFilterReader::read()
{
	IntType ret = FilterReader::read();

	if(ret == 0x0D)
	{
		ret = 0x0A;
		m_bStandaloneCR = true;
	}
	else if(ret == 0x0A && m_bStandaloneCR)
	{
		m_bStandaloneCR = false;
		ret = read();
	}

	return ret;
}

//==============================================================================
// XMLFilterReader::read
//
// Call the underlying Reader's read method, and then normalize newlines.
//==============================================================================
long XMLFilterReader::read(CharType* pBuffer, size_t bufLen)
{
	long charsRead=0;

	//
	// loop to guard against the situation where just one
	// character is returned and normalized away - resulting in
	// a potential invalid zero return.
	//
	while(charsRead==0)
	{
		charsRead = FilterReader::read(pBuffer, bufLen);
		
		if(charsRead > 0)
		{
			charsRead = normalizeNewLines(pBuffer, charsRead);
		}
		else
		{
			break;
		}
	}
	return charsRead;
}

//==============================================================================
// XMLFilterReader::readAtomic
//
//==============================================================================
long XMLFilterReader::readAtomic(CharType* pBuffer, size_t bufLen)
{
	long charsRead=0;

	//
	// loop to guard against the situation where just one
	// character is returned and normalized away - resulting in
	// a potential invalid zero return.
	//
	while(charsRead==0)
	{
		charsRead = FilterReader::readAtomic(pBuffer, bufLen);
		if(charsRead > 0)
		{
			charsRead = normalizeNewLines(pBuffer, charsRead);
		}
		else
		{
			break;
		}
	}
	return charsRead;
}

Character XMLFilterReader::readAtomic()
{
	static Character newLine('\n');
	Character ret = FilterReader::readAtomic();
	UCS4Char ch = ret.toUnicode();

	if(ch == 0x0D)
	{
		ret = newLine;
		m_bStandaloneCR = true;
	}
	else if(ch == 0x0A && m_bStandaloneCR)
	{
		m_bStandaloneCR = false;
		ret = readAtomic();
	}

	return ret;
}

//==============================================================================
// XMLFilterReader::normalizeNewLines
//
// Normalize end-of-line characters as per the W3 XML 1.0 recommendation
// section 2.11
//
// Replace the 2-character sequence "0x0D" "0x0A" with "0x0A" and
// standalone "0x0D" with "0x0A"
//
//==============================================================================
long XMLFilterReader::normalizeNewLines(CharType* pBuffer, size_t bufLen)
{
	CharType* pEnd = pBuffer + bufLen;
	CharType* pNextOut = pBuffer;

	for(CharType* pNextIn = pBuffer; pNextIn < pEnd; ++pNextIn)
	{
		const CharType nextChar = *pNextIn;

		if(nextChar == 0x0D)
		{
			*pNextOut++ = 0x0A;
			//
			// We have found a 0x0D (carriage return to you and me)
			// which we have silently converted to a newline
			// If a newline follows immediately afterwards then we want to
			// delete it.  If we are before the last character then we can
			// simply check the buffer, otherwise we need to get another.
			//
			if(pNextIn+1 < pEnd)
			{
				if(*(pNextIn+1) == 0x0A)
				{
					// next character is effectively deleted simply by incrementing ptr
					// so that it is not copied like all the rest
					pNextIn++;
				}
			}
			else
			{
				//
				// We are in the situation where the last character in the 
				// buffer was a 0x0D (cr).  We have changed it into a
				// newline (0x0A) but we need to know what comes next to see
				// if the next character is a newline which should be silently
				// dropped
				//
				// To do this we just set a marker which is tested next time
				// we do a normalization
				
				m_bStandaloneCR = true;
				break;
			}
		}
		else if(!(nextChar == 0x0A && m_bStandaloneCR))
		{
			*pNextOut++ = nextChar;
		}

		m_bStandaloneCR = false;
	}
	
	return (pNextOut - pBuffer);
}
	

QC_XML_NAMESPACE_END

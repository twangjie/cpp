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

#include "Scanner.h"
#include "Buffer.h"
#include "BufferRange.h"
#include "CharTypeFacet.h"
#include "Entity.h"
#include "ExternalEntity.h"

#include "QcCore/base/debug.h"
#include "QcCore/base/SystemCodeConverter.h"

#include <algorithm>

QC_XML_NAMESPACE_BEGIN

const size_t defaultBufferSize = 4096;  //to do: test various sizes

const Character chSpace(' ');

ScannerPosition Scanner::GetPosition(const Entity& entity, const StreamPosition& location, size_t bufferSize)
{
	if(bufferSize == 0)
	{
		bufferSize = defaultBufferSize;
	}

	AutoPtr<Buffer> mpBuffer(new Buffer(bufferSize, entity));
	return ScannerPosition(mpBuffer.get(), location);
}

//=============================================================================
// Scanner::PeekNextCharacterImpl
// 
// Full implementation of PeekNextCharacter(), where the common part has
// been moved inline.
//
// Return the next character pointed at by position, without advancing position.
//
//=============================================================================
Character Scanner::PeekNextCharacterImpl(const ScannerPosition& position)
{
	//
	// Because position.m_rpBuffer is dereferenced several times, it aids 
	// performance to take a local copy.
	//
	Buffer* pBuffer = position.m_rpBuffer.get();

	// 
	// If we have reached the end of the available portion of the buffer
	// then try and read some more.
	// 
	// The result of this is that there should be at least one character
	// more to read or the buffer is at the end of the Entity, or the
	// burffer is full and we need to move to the next one.
	//
	if (position.m_bufferOffset == pBuffer->m_used)
	{
		pBuffer->read();
	}

	//
	// If there is unread data in the buffer, then that's easy
	//
	const size_t charsAvailable = (pBuffer->m_used - position.m_bufferOffset);
	if(charsAvailable)
	{
		return Character(pBuffer->m_pData+position.m_bufferOffset, charsAvailable);
	}
	else if (!pBuffer->m_bEOF)
	{
		Buffer* pNextBuffer = pBuffer->m_rpNext.get();

		if(pNextBuffer && pNextBuffer->m_used)
		{
			return Character(pNextBuffer->m_pData, pNextBuffer->m_used);
		}
	}

	//
	// We get here at EOF for an Entity.  If we have a next ptr then
	// we can move back up to the parent entity and try again.
	// 
	QC_DBG_ASSERT(pBuffer->m_bEOF);
	if(position.m_pNext)
	{
		switch (position.m_eofAction)
		{
		case ScannerPosition::space:
			return chSpace;
		case ScannerPosition::skip:
			return PeekNextCharacter(*position.m_pNext);
		case ScannerPosition::softEOF:
			break;
		default:
			QC_DBG_ASSERT(false);
		}
	}

	return Character::EndOfFileCharacter;
}


//=============================================================================
// Scanner::PeekNextString
// 
// Return the next string of characters pointed at by position, where each
// character comforms to the classification determined by mask.
// This is a "peek" function - it does not advance position.
//
// Note: As this function takes a copy of the ScannerPosition it is 
// possibly sub-optimal.  However, the parser is not likely to need this
// very often so this might not be too important.
//=============================================================================
String Scanner::PeekNextContigString(ScannerPosition& position,
                                     CharTypeFacet::Mask includeMask,
                                     CharTypeFacet::Mask excludeMask)
{
	ScannerPosition myPosition = position;
	Character nextChar;
	String delim;
	return GetNextContigString(myPosition, includeMask, excludeMask, delim, nextChar).asString();
}

//=============================================================================
// Scanner::PeekNextStringConstant
// 
// Simply test if the next n characters in the input stream match exactly
// the characters of the string str.
//=============================================================================
bool Scanner::PeekNextStringConstant(ScannerPosition& position, const String& str)
{
	QC_DBG_ASSERT(!str.empty());
	ScannerPosition myPosition = position;
	
	for(size_t i=0; i<str.size(); i++)
	{
		if(GetNextCharacter(myPosition) != str[i])
			return false;
	}

	// all the string's characters matched the input stream, so 
	// return true.
	return true;
}

//=============================================================================
// Scanner::PeekNextStringConstant
// 
// Overloaded version that takes a char* parameter.
//=============================================================================
bool Scanner::PeekNextStringConstant(ScannerPosition& position, const char* pStr)
{
	QC_DBG_ASSERT(pStr!=0);
	ScannerPosition myPosition = position;
	size_t stringLen = strlen(pStr);
	for(size_t i=0; i<stringLen; i++)
	{
		if(GetNextCharacter(myPosition) != pStr[i])
		{
			return false;
		}
	}

	// all the string's characters matched the input stream, so 
	// return true.
	return true;
}

//=============================================================================
// Scanner::GetNextCharacterImpl
// 
// Return the next character pointed at by position and advance position.
//
// Note: Notice the similarity to PeekNextChar - basically doing the same
//       job without advancing the position.  Would like to use common 
//       routine - but without the expense of run-time performance.
//=============================================================================
Character Scanner::GetNextCharacterImpl(ScannerPosition& position)
{
	Character ret = Character::EndOfFileCharacter;

	//
	// Because position.m_rpBuffer is dereferenced several times, it aids 
	// performance to take a local copy.
	//
	Buffer* pBuffer = position.m_rpBuffer.get();

	// 
	// If we have reached the end of the available portion of the buffer
	// then try and read some more.
	// 
	// The result of this is that there should be at least one character
	// more to read or the buffer is at the end of the Entity, or the
	// burrfer is full and we need to move to the next one.
	//
	if (position.m_bufferOffset == pBuffer->m_used)
	{
		pBuffer->read();
	}

	//
	// If there is unread data in the buffer, then that's easy
	// take the Char sequence and increment position by its length
	//
	const size_t charAvailable = (pBuffer->m_used - position.m_bufferOffset);
	if(charAvailable)
	{
		ret = Character(pBuffer->m_pData+position.m_bufferOffset, charAvailable);
		position.m_bufferOffset+=ret.length();
	}
	else if(!pBuffer->m_bEOF)
	{
		// Otherwise, if the current buffer is not the last in line (eof)
		// we want to Get the next buffer.  It is likely that this involves
		// creating (and chaining) a new buffer - but it might not if we have
		// been here before courtesy of a different streamPosition object.
		// Whatever, the result will be that we can chain forward using the
		// rcpNext member.

		Buffer* pNextBuffer = pBuffer->m_rpNext.get();
		
		// Advance position to the Next buffer
		position.m_rpBuffer = pNextBuffer;
		position.m_bufferOffset = 0;

		if(pNextBuffer->m_used)
		{
			// Take the available Char sequence and increment position by its length
			ret = Character(pNextBuffer->m_pData, pNextBuffer->m_used);
			position.m_bufferOffset+=ret.length();
		}
	}


	//
	// If the ScannerPosition allows us to process the next position in the
	// chain then we will do so.
	//
	if(ret.isEOF() && position.m_pNext)
	{
		switch (position.m_eofAction)
		{
		case ScannerPosition::space:
			position.m_eofAction = ScannerPosition::skip;
			return chSpace;
		case ScannerPosition::skip:
			position = ScannerPosition(*position.m_pNext);
			return GetNextCharacter(position);
		case ScannerPosition::softEOF:
			break;
		default:
			QC_DBG_ASSERT(false);
		}
	}
	else
	{
		//
		// Update the StreamPosition to reflect the latest gotten Char
		// Note that the space generated above does not increment the StreamPosition
		if(!ret.isEOF())
		{
			position.m_streamPosition.incrementByChar(ret);
		}
	}

	return ret;
}

//=============================================================================
// Scanner::SkipSoftEOF
// 
// When a Scanner position is resting on a Soft EOF, this function
// will jog it forward to the next position.
//=============================================================================
bool Scanner::SkipSoftEOF(ScannerPosition& position)
{
	// 
	// Test we are at ROF for the current entity, and there is another
	// on to go to.
	//
	if((position.m_bufferOffset == position.m_rpBuffer->m_used)
		&& position.m_rpBuffer->m_bEOF && position.m_pNext)
	{
		position = ScannerPosition(*position.m_pNext);
		return true;
	}
	else
	{
		return false;
	}
}

//==============================================================================
// Scanner::GetEntityBuffer
//
// Return a BufferRange object representing an entire Entity.
//==============================================================================
BufferRange Scanner::GetEntityBuffer(const ScannerPosition& position)
{
	// take a copy of the buffer start position and offset
	AutoPtr<Buffer> rpStart = position.m_rpBuffer;
	const size_t startOffset = 0;
	size_t retLength = 0;

	Buffer* pBuffer = position.m_rpBuffer.get();

	//
	// Ensure the entire entity is buffered
	// 
	while(true)
	{
		while(!pBuffer->m_bFull && !pBuffer->m_bEOF)
		{
			pBuffer->read();
		}

		retLength += pBuffer->m_used;

		if(pBuffer->m_bEOF)
		{
			break;
		}
		else
		{
			pBuffer = pBuffer->m_rpNext.get();
			QC_DBG_ASSERT(pBuffer!=0);
		}
	}

	return BufferRange(rpStart.get(), startOffset, retLength);
}

//=============================================================================
// Scanner::GetNextContigString
// 
// Return the next contiguous string of characters pointed at by position, 
// where each character comforms to the classification determined by includeMask
// and does not conform to excludeMask.  If a string is returned then position
// is advanced accordingly.
//
// Note: Contiguous means that all the characters are obtained from the
// same entity and run contiguously.  This does not necessarily mean they
// all have to be contained within the same buffer, they can span
// buffers but the string must be able to be represented by a BufferRange
// which is a logical string of characters that can span buffer boundaries.
//
// Note also, that if the ScannerPosition is pointing at the EOF of an
// entity at entry, then this routine will happily skip to the next entity
// if there is one and the ScannerPosition permits this action. But the
// returned BufferRange will only reference characters from a single entity.
//=============================================================================
BufferRange Scanner::GetNextContigString(ScannerPosition& position,
	                                     CharTypeFacet::Mask includeMask,
	                                     CharTypeFacet::Mask excludeMask,
										 const String& delim,
	                                     Character& nextChar,
	                                     size_t maxSize)
{

	// take a copy of the buffer start position and offset
	AutoPtr<Buffer> rpStart = position.m_rpBuffer;
	size_t startOffset = position.m_bufferOffset;
	size_t retLength = 0;

	size_t bufferOffset = position.m_bufferOffset;
	Buffer* pBuffer = position.m_rpBuffer.get();
	bool bDelimited = !delim.empty();
	const CharType cDelim = bDelimited ? delim[0] : 0;

	while(maxSize == 0 || retLength < maxSize)
	{
		nextChar = Character::EndOfFileCharacter;
		
		//
		// If there is unread data in the buffer, then use it 
		//
		const size_t charsAvailable = (pBuffer->m_used - bufferOffset);
		if(charsAvailable)
		{
			nextChar = Character(pBuffer->m_pData+bufferOffset, charsAvailable);
		}
		else
		{
			// 
			// If we have reached the end of the available portion of the buffer
			// then try and read some more.  This will chain on another buffer
			// if the current buffer is full.
			// 
			// Note: This can throw an exception, so to place the Scanner at the
			// correct exception location, update the position now.
			position.m_bufferOffset = bufferOffset;

			pBuffer->read();

			// If, even after reading, we are still left with nothing, then
			// parhaps the buffer is full and we need to chain to the next one.
			// Otherwise, it must be the end of file condition - and then we
			// are only able to move to a possible next position if we have
			// not yet read anything at all.  Why? because this function
			// is only designed to return contiguous strings from a single entity.
			if(bufferOffset < pBuffer->m_used)
			{
				continue;
			}
			else if(bufferOffset == pBuffer->m_used && !pBuffer->m_bEOF)
			{
				position.m_rpBuffer = pBuffer->m_rpNext;
				pBuffer = position.m_rpBuffer.get();
				bufferOffset = 0;
				continue;
			}
			else if(bufferOffset == pBuffer->m_used && pBuffer->m_bEOF
			        && 0 == retLength && position.m_pNext
					&& position.m_eofAction == ScannerPosition::skip)
			{
				position = *position.m_pNext;
				pBuffer = position.m_rpBuffer.get();
				bufferOffset = position.m_bufferOffset;
				rpStart = pBuffer;
				startOffset = position.m_bufferOffset;
				continue;
			}
		}		
		
		//
		// If the located character is within the include set, then
		// add it to the return string.  Otherwise it's the end of the
		// road for this string.
		//
		if(!nextChar.isEOF() && CharTypeFacet::IsCharType(nextChar, includeMask, excludeMask))
		{
			if(nextChar == cDelim)
			{
				position.m_bufferOffset = bufferOffset;
				if(PeekNextStringConstant(position, delim))
				{
					break;
				}
			}
			retLength+=nextChar.length();
			bufferOffset+=nextChar.length();
			position.m_streamPosition.incrementByChar(nextChar);
		}
		else
		{
			break;
		}
	}

	// set scanner position to point at the next character
	position.m_bufferOffset = bufferOffset;

	return BufferRange(rpStart.get(), startOffset, retLength);
}

//=============================================================================
// Scanner::GetNextString
// 
// Return the next string of characters pointed at by position, where each
// character comforms to the classification determined by includeMask
// and does not conform to excludeMask and is not equal to the delimiter.
// Return the next character in the input stream in nextChar.  The 
// caller can reference this to see if the delimiter was reached.
// If a string is returned then position is advanced accordingly.
//=============================================================================
String Scanner::GetNextStringDelimited(ScannerPosition& position,
                                       CharTypeFacet::Mask includeMask,
                                       CharTypeFacet::Mask excludeMask,
                                       CharType delimiter,
                                       Character& nextChar,
									   size_t maxSize)
{
	String strRet;
	size_t count = 0;
	while (true)
	{
		nextChar = PeekNextCharacter(position);
		if ( (CharTypeFacet::IsCharType(nextChar, includeMask) || includeMask == CharTypeFacet::Any)
		     && !CharTypeFacet::IsCharType(nextChar, excludeMask)
		     && nextChar != delimiter 
			 && !nextChar.isEOF())
		{
			GetNextCharacter(position).appendToString(strRet);
			
			if(maxSize && count++ > maxSize)
				break;
		}
		else
			break;
	}
	return strRet;
}

//=============================================================================
// Scanner::GetNextStringDelimited
// 
// Return the next string of characters pointed at by position, where each
// character comforms to the classification determined by includeMask
// and does not conform to excludeMask and is not equal to the delimiter.
//
// Return the nexr char in the input stream in nextChar.
// If a string is returned then position is advanced accordingly.
//
// Note: For convenience this function is implemented using
// the character version of GetNextStringDelimited.  This could 
// pose a performance problem if it was used extensively.
//=============================================================================
String Scanner::GetNextStringDelimited(ScannerPosition& position,
	                                   CharTypeFacet::Mask includeMask,
	                                   CharTypeFacet::Mask excludeMask,
	                                   const String& delimiter,
	                                   Character& nextChar)
{
	String strRet;
	QC_DBG_ASSERT(!delimiter.empty());
	const CharType cDelim = delimiter.at(0);
	while (true)
	{
		//
		// obtain string up to first character of the delimiter string
		//
		strRet += GetNextStringDelimited(position,
	                                     includeMask,
	                                     excludeMask,
	                                     cDelim,
	                                     nextChar);

		if(nextChar == cDelim)
		{
			if(PeekNextStringConstant(position, delimiter))
			{
				break;
			}
			else
			{
				GetNextCharacter(position).appendToString(strRet);
			}
		}
		else
		{
			// either at EOF or an illegal character!
			break;
		}
	}

	return strRet;
}


//==============================================================================
// Scanner::GetNextContigDecimalString
//
//==============================================================================
String Scanner::GetNextContigDecimalString(ScannerPosition& position,
                                           bool /*bAllowDecimalPoint*/,
                                           Character& nextChar)
{
	String strRet;

	while (true)
	{
		nextChar = PeekNextCharacter(position);

		if(nextChar.isDigit())
		{
			GetNextCharacter(position).appendToString(strRet);
		}
		else
		{
			break;
		}
	}

	return strRet;
}

//==============================================================================
// Scanner::GetNextContigHexString
//
//==============================================================================
String Scanner::GetNextContigHexString(ScannerPosition& position, Character& nextChar)
{
	String strRet;

	while (true)
	{
		nextChar = PeekNextCharacter(position);

		if(nextChar.isHexDigit())
		{
			GetNextCharacter(position).appendToString(strRet);
		}
		else
		{
			break;
		}
	}

	return strRet;

}

//=============================================================================
// Scanner::skipWhiteSpace
// 
//=============================================================================
size_t Scanner::SkipWhiteSpace(ScannerPosition& position)
{
	size_t skipCount(0);

	while (CharTypeFacet::IsWhiteSpace(PeekNextCharacter(position)))
	{
		GetNextCharacter(position);
		skipCount++;
	}
	return skipCount;
}

//==============================================================================
// Scanner::Skip
//
// Skip n UCS4 Characters
//==============================================================================
void Scanner::Skip(ScannerPosition& position, size_t n)
{
	for(size_t i=0; i<n; i++)
	{
		GetNextCharacter(position);
	}
}

//==============================================================================
// Scanner::SkipToDelimiter
//
//==============================================================================
CharType Scanner::SkipToDelimiter(ScannerPosition& position,
                                  CharType delim, CharTypeFacet::Mask includeMask)
{
	Character x; 
	while( (x = PeekNextCharacter(position)) != delim && CharTypeFacet::IsCharType(x, includeMask) && !x.isEOF())
	{
		GetNextCharacter(position);
	}
	return x.first();
}

//=============================================================================
// Scanner::SkipNextCharConstant
// 
// Called by the  parser to test the input stream against a given character.
// If there is a match, then the input position is incremented and we
// return true, otherwise position remains where it is and we return false.
//=============================================================================
bool Scanner::SkipNextCharConstant(ScannerPosition& position, CharType x)
{
	if(PeekNextCharacter(position) == x)
	{
		GetNextCharacter(position);
		return true;
	}
	else
	{
		return false;
	}
}

//=============================================================================
// Scanner::SkipNextStringConstant
// 
// Called by the  parser to test the input stream against a given string.
// If there is a match, then the input position is incremented and we
// return true, otherwise position remains where it is and we return false.
//
// Note: Zero length string is fine!
//=============================================================================
bool Scanner::SkipNextStringConstant(ScannerPosition& position, const String& str)
{
	ScannerPosition myPosition = position;
	
	for(size_t i=0; i<str.size(); i++)
	{
		if(GetNextCharacter(myPosition) != str[i])
			return false;
	}

	// all the string's characters matched the input stream, so advance position
	// and return true.
	position = myPosition;
	return true;
}

//=============================================================================
// Scanner::SkipNextStringConstant
// 
// Called by the  parser to test the input stream against a given string.
// If there is a match, then the input position is incremented and we
// return true, otherwise position remains where it is and we return false.
//
// Note: Overridden for const char*
//=============================================================================
bool Scanner::SkipNextStringConstant(ScannerPosition& position, const char* pStr)
{
	QC_DBG_ASSERT(pStr!=0);
	ScannerPosition myPosition = position;
	size_t stringLen = strlen(pStr);
	
	for(size_t i=0; i<stringLen; i++)
	{
		if(GetNextCharacter(myPosition) != pStr[i])
			return false;
	}

	// all the string's characters matched the input stream, so advance position
	// and return true.
	position = myPosition;
	return true;
}

//=============================================================================
// Scanner::SkipToDelimiter
// 
// Skip the input position up to (but not including) delimiter or EOF, 
// whatever comes first.
//
// Return the next character in the iunput stream.
//=============================================================================
CharType Scanner::SkipToDelimiter(ScannerPosition& position,
                                  const String& delim, CharTypeFacet::Mask includeMask)
{
	QC_DBG_ASSERT(!delim.empty());
	const CharType firstDelimChar = delim[0]; 

	CharType nextChar;
	while((nextChar = SkipToDelimiter(position, firstDelimChar, includeMask)) == firstDelimChar)
	{
		if(PeekNextStringConstant(position, delim))
		{
			break;
		}
		else
		{
			GetNextCharacter(position);
		}
	}

	return nextChar;
}


//=============================================================================
// Scanner::SkipToDelimiters
// 
// Skip the input position up to (but not including) any one of the passed
// delimiter strings or EOF - whatever comes first.
//
// Return the index number of the string found, or -1
//
// Note the delims parameter is an array of char *s.  We do not pass CharType
// for the simple reason that all delimiters will appear in the ascii range.
//=============================================================================
int Scanner::SkipToDelimiters(ScannerPosition& position, size_t numDelimiters,
                              const char* delims[])
{
	Character nextChar; 
	while( (nextChar=PeekNextCharacter(position)).isEOF() != true)
	{
		for(size_t i=0; i<numDelimiters; i++)
		{
			if(PeekNextStringConstant(position, delims[i]))
			{
				return (int)i;
			}
		}
		GetNextCharacter(position);
	}
	return -1;
}

//==============================================================================
// Scanner::BackTrackToChar
//
// Back track up to (but not including) the specified character or the start
// of the buffer, but don't backtrack more than maxLen characters.
//==============================================================================
ScannerPosition Scanner::BackTrackToChar(const ScannerPosition& position, CharType toChar, size_t maxLen)
{
	// take a copy of the passed ScannerPosition
	ScannerPosition prevPosition = position;
	
	Character prevChar;
	size_t count=0;
	
	while(count <= maxLen)
	{
		//
		// Backtracking means 
		while(prevPosition.m_bufferOffset > 0 && !SystemCodeConverter::IsSequenceStartChar(prevPosition.m_rpBuffer->m_pData[--prevPosition.m_bufferOffset]))
		{}

		if(prevPosition.m_bufferOffset > 0)
		{
			const size_t charsAvailable = (prevPosition.m_rpBuffer->m_used - prevPosition.m_bufferOffset);
			prevChar = Character(prevPosition.m_rpBuffer->m_pData+prevPosition.m_bufferOffset, charsAvailable);
			if(prevChar == toChar)
			{
				// we don't want to step back all the way to the delimiter character
				// so position the buffer one (Unicode) Character forward
				prevPosition.m_bufferOffset += prevChar.length();
				break;
			}
			else
			{
				++count;
			}
		}
		else
		{
			break;
		}
	}

	//
	// decrement the streamposition according to the number of characters
	// we've back-tracked.  Note: this would not work if we backtracked past a newline!
	//
	prevPosition.m_streamPosition.decrementColumns(count);

	return prevPosition;
}

const ExternalEntity* Scanner::GetCurrentExternalEntity(const ScannerPosition& position)
{
	const ScannerPosition* pPosition = &position;

	while(pPosition)
	{
		if(pPosition->getEntity().isExternalEntity())
		{
			return static_cast<const ExternalEntity*>(&pPosition->getEntity());
		}
		pPosition = pPosition->m_pNext;
	}
	return 0;
}

QC_XML_NAMESPACE_END



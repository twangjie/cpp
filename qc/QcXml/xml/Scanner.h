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
// Class: Scanner
// 
// Overview
// --------
// Provides the mechanisms to 
// i)   read an input stream
// ii)  buffer the stream data and provide direct and efficient access to
//      any previously read location.  (The stream is read into buffers which
//      are reference-counted, the buffers remain available for so long as a 
//      reference is held by the application - normally via a ScannerPosition
//      object)
// iii) provide customized look-ahead routines to determine the contents of
//      the stream at a given position - making use of the character masks
//      from the CharTypeFacet class.
//
//==============================================================================

#ifndef QC_XML_Scanner_h
#define QC_XML_Scanner_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

#ifndef QC_XML_ScannerPosition_h
#include "ScannerPosition.h"
#endif //QC_XML_ScannerPosition_h

#ifndef QC_XML_CharTypeFacet_h
#include "CharTypeFacet.h"
#endif //QC_XML_CharTypeFacet_h

QC_XML_NAMESPACE_BEGIN

class Buffer;
class ExternalEntity;
class StreamPosition;
class BufferRange;

class QC_XML_PKG Scanner
{
public:

	static ScannerPosition GetPosition(const Entity& entity,
	                                   const StreamPosition& location,
	                                   size_t bufferSize=0);

	static bool PeekNextStringConstant(ScannerPosition& position, const String& str);
	static bool PeekNextStringConstant(ScannerPosition& position, const char* pStr);

	static Character PeekNextCharacter(const ScannerPosition& position);
	static Character PeekNextCharacterImpl(const ScannerPosition& position);

	static String PeekNextContigString(ScannerPosition& position,
	                                   CharTypeFacet::Mask includeMask,
	                                   CharTypeFacet::Mask excludeMask);

	static Character GetNextCharacter(ScannerPosition& position);
	static Character GetNextCharacterImpl(ScannerPosition& position);

	static BufferRange GetNextContigString(ScannerPosition& position,
	                                       CharTypeFacet::Mask includeMask,
	                                       CharTypeFacet::Mask excludeMask,
										   const String& delim,
	                                       Character& nextChar,
	                                       size_t maxSize=0);

	static BufferRange GetEntityBuffer(const ScannerPosition& position);

	static String GetNextStringDelimited(ScannerPosition& position,
	                                     CharTypeFacet::Mask includeMask,
	                                     CharTypeFacet::Mask excludeMask,
	                                     CharType delimiter,
								         Character& nextChar,
										 size_t maxSize=0);

	static String GetNextStringDelimited(ScannerPosition& position,
	                                     CharTypeFacet::Mask includeMask,
	                                     CharTypeFacet::Mask excludeMask,
	                                     const String& delimiter,
								         Character& nextChar);

	static String GetNextContigDecimalString(ScannerPosition& position,
	                                         bool bAllowDecimalPoint,
	                                         Character& nextChar);

	static String GetNextContigHexString(ScannerPosition& position,
	                                     Character& nextChar);

	static size_t SkipWhiteSpace(ScannerPosition& position);
	
	static bool SkipSoftEOF(ScannerPosition& position);

	static void Skip(ScannerPosition& position, size_t n);
	
	static CharType SkipToDelimiter(ScannerPosition& position,
	                                CharType delim, CharTypeFacet::Mask includeMask);
	
	static CharType SkipToDelimiter(ScannerPosition& position,
	                                const String& delim, CharTypeFacet::Mask includeMask);
	
	static int SkipToDelimiters(ScannerPosition& position, size_t numDelimiters,
	                            const char* delims[]);
	
	static bool SkipNextStringConstant(ScannerPosition& position, const String& str);
	static bool SkipNextStringConstant(ScannerPosition& position, const char* pStr);
	
	static bool SkipNextCharConstant(ScannerPosition& position, CharType x);
	
	static ScannerPosition BackTrackToChar(const ScannerPosition& position, CharType toChar, size_t maxLen);

	static const ExternalEntity* GetCurrentExternalEntity(const ScannerPosition& position);

protected:
	static Buffer* ReadAndChainNextBuffer(Entity* pEntity, Buffer* pOldBuffer=NULL);

private:
	Scanner(); // not implemented
};

/*
inline CharType Scanner::GetNextChar(ScannerPosition& position)
{
	Buffer* pBuffer = position.m_rpBuffer.get();
	if (position.m_bufferOffset < pBuffer->m_used)
	{
		const CharType ret = pBuffer->m_pData[position.m_bufferOffset++];
		position.m_streamPosition.incrementByChar(ret);
		return ret;
	}
	else
	{
		return GetNextCharImpl(position);
	}
}
*/
inline Character Scanner::GetNextCharacter(ScannerPosition& position)
{
	Buffer* pBuffer = position.m_rpBuffer.get();
	const size_t charsAvailable = (pBuffer->m_used - position.m_bufferOffset);
	if(charsAvailable)
	{
		const Character ret(pBuffer->m_pData+position.m_bufferOffset, charsAvailable);
		position.m_bufferOffset += ret.length();
		position.m_streamPosition.incrementByChar(ret);
		return ret;
	}
	else
	{
		return GetNextCharacterImpl(position);
	}
}

inline Character Scanner::PeekNextCharacter(const ScannerPosition& position)
{
	Buffer* pBuffer = position.m_rpBuffer.get();
	const size_t charsAvailable = (pBuffer->m_used - position.m_bufferOffset);
	if(charsAvailable)
	{
		return Character(pBuffer->m_pData+position.m_bufferOffset, charsAvailable);
	}
	else
	{
		return PeekNextCharacterImpl(position);
	}
}

QC_XML_NAMESPACE_END

#endif //QC_XML_Scanner_h

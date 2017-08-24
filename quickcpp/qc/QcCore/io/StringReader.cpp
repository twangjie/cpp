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
// Class: StringReader
/**
	@class qc::io::StringReader
	
	@brief A Reader which uses a String as its input source.
	@sa StringWriter
*/
//==============================================================================

#include "StringReader.h"
#include "AtomicReadException.h"

#include "QcCore/base/NullPointerException.h"
#include "QcCore/base/IllegalCharacterException.h"
#include "QcCore/base/SystemUtils.h"

QC_IO_NAMESPACE_BEGIN

//==============================================================================
// StringReader::StringReader
//
/**
   Constructs a StringReader with a copy of @c string as the contained String.
*/
//==============================================================================
StringReader::StringReader(const String& string) :
	m_string(string),
	m_pos(0),
	m_markPos(-1),
	m_bClosed(false)
{
}

//==============================================================================
// StringReader::StringReader
//
/**
   Constructs a StringReader, copying the ::CharType array beginning
   at @c pStr into a String which is used as the input source.

   Unicode characters represented by the ::CharType array should already be
   encoded using the internal @QuickCPP encoding scheme.

   @param pStr pointer to the start of the ::CharType array to copy
   @param length of the ::CharType array
*/
//==============================================================================
StringReader::StringReader(const CharType* pStr, size_t length) :
	m_pos(0),
	m_markPos(-1),
	m_bClosed(false)
{
	if(length)
	{
		if(!pStr) throw NullPointerException();
		m_string.assign(pStr, length);
	}
}

//==============================================================================
// StringReader::close
//
//==============================================================================
void StringReader::close()
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	m_markPos = -1;
	m_bClosed = true;
}

//==============================================================================
// StringReader::mark
//
// Marks the current position in this reader. A subsequent call to the 
// reset method re-positions the reader at the last marked position so that 
// subsequent reads re-read the same characters. 
//
// The readLimit has no effect for StringReaders, as the entire string
// is always available until the Reader is closed. 
//==============================================================================
void StringReader::mark(size_t /*readLimit*/)
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	if(m_bClosed) throw IOException(QC_T("stream is closed"));
	m_markPos = m_pos;
}

//==============================================================================
// StringReader::reset
//
// Reset the position to the last mark() operation.  If there is no mark 
// (due to no mark() call then throw an IOException.
//==============================================================================
void StringReader::reset()
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	if(m_markPos == -1)
	{
		throw IOException(QC_T("unable to reset input stream, no marked position"));
	}
	else
	{
		m_pos = m_markPos;
	}
}

//==============================================================================
// StringReader::markSupported
//
/**
   Tests whether the Reader supports the mark() operation, which it does.
   
   @sa mark()
   @sa reset()
   @returns always returns true for StringReader.
   @synchronized
*/
//==============================================================================
bool StringReader::markSupported() const
{
	return true;
}

//==============================================================================
// StringReader::read
//
//==============================================================================
long StringReader::read(CharType* pBuffer, size_t bufLen)
{
	SystemUtils::TestBufferIsValid(pBuffer, bufLen);
	
	QC_SYNCHRONIZED_PTR(m_rpLock)

	if(m_bClosed) throw IOException(QC_T("stream is closed"));

	const size_t charsAvailable = (m_string.length() - m_pos);

	if(charsAvailable)
	{
		const size_t count = (bufLen < charsAvailable)
		                   ? bufLen
		                   : charsAvailable;

		::memcpy(pBuffer, m_string.data()+m_pos, count*sizeof(CharType));
		m_pos += count;
		return count;
	}
	else
	{
		return EndOfFile;
	}
}

//==============================================================================
// StringReader::readAtomic
//
//==============================================================================
long StringReader::readAtomic(CharType* pBuffer, size_t bufLen)
{
	SystemUtils::TestBufferIsValid(pBuffer, bufLen);

	QC_SYNCHRONIZED_PTR(m_rpLock)

	if(m_bClosed) throw IOException(QC_T("stream is closed"));

	const size_t stringSize = m_string.size();
	
	if(m_pos < stringSize)
	{
		const CharType* pData = m_string.data();

		if(!SystemCodeConverter::IsSequenceStartChar(pData[m_pos]))
		{
			throw AtomicReadException(QC_T("not on character sequence boundary"));
		}

		//
		// If we can slip our entire buffer into the passed buffer then 
		// that's great because our buffer *should* hold an integral number of
		// unicode characters.  But if the passed buffer is too small, we have
		// a choice: either
		// i)  scan the buffer counting characters; or
		// ii) search back for a sequence start character
		// All internal encodings are guaranteed to support the recognition of
		// sequence start characters, so (ii) will be more efficient for any buffer
		// of a significant size. 
		//
		size_t charCount = stringSize - m_pos;
		if(charCount > bufLen)
		{
			charCount = bufLen;
			do {--charCount;}
			while(charCount && !SystemCodeConverter::IsSequenceStartChar(*(pData+m_pos+charCount)));

			size_t charSeqLen = SystemCodeConverter::GetCharSequenceLength(*(pData+m_pos+charCount));
			if((charCount + charSeqLen) <= bufLen)
			{
				charCount += charSeqLen;
			}
		}

		//
		// It's quite possible that we are unable to read any characters because
		// there is insufficient buffer space to read an integral number of 
		// characters.  This is legal.
		//
		if(charCount)
		{
			::memcpy(pBuffer, pData+m_pos, charCount*sizeof(CharType));
			m_pos += charCount;
		}
		return charCount;
	}
	else
	{
		return EndOfFile;
	}
}

//==============================================================================
// StringReader::readAtomic
//
//==============================================================================
Character StringReader::readAtomic()
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	if(m_bClosed) throw IOException(QC_T("stream is closed"));
	
	if(m_pos < m_string.length())
	{
		if(!SystemCodeConverter::IsSequenceStartChar(m_string[m_pos]))
		{
			throw AtomicReadException(QC_T("not on character sequence boundary"));
		}

		const CharType nextChar = m_string[m_pos];
		size_t seqLen = SystemCodeConverter::GetCharSequenceLength(nextChar);

		if((m_pos+seqLen) <= m_string.length())
		{
			size_t oldOffset = m_pos;
			m_pos += seqLen;
			return Character(m_string.data() + oldOffset, seqLen);
		}
		else
		{
			throw IllegalCharacterException(QC_T("unterminated character sequence in String"));
		}
	}
	return Character::EndOfFileCharacter;
}

QC_IO_NAMESPACE_END

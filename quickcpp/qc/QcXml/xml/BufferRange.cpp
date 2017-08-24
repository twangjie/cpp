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

#include "BufferRange.h" 
#include "Buffer.h" 

QC_XML_NAMESPACE_BEGIN

BufferRange::BufferRange(Buffer* pStart, size_t startOffset, size_t size) :
	m_rpStart(pStart),
	m_startOffset(startOffset),
	m_size(size),
	m_numFragments(0)
{
	QC_DBG_ASSERT(m_rpStart);

	//
	// Find out how many buffers are involved in this range
	//
	Buffer* pBuffer = pStart;
	size_t count=0;
	
	while(count < m_size && pBuffer)
	{
		count += (pBuffer->m_used - startOffset);
		startOffset = 0;
		pBuffer = pBuffer->m_rpNext.get();
		m_numFragments++;
	}
	QC_DBG_ASSERT(count >= m_size);
	
	if(m_size == 0)
	{
		releaseBuffer();
	}
}

//==============================================================================
// BufferRange::getSize
//
// Return the size of the character array represented by this BufferRange.
//==============================================================================
size_t BufferRange::getSize() const
{
	return m_size;
}

//==============================================================================
// BufferRange::releaseBuffer
//
// Release all resources held by this BufferRange.
// The BufferRange is meaningless once this function has been called.
//==============================================================================
void BufferRange::releaseBuffer()
{
	m_size = 0;
	m_startOffset = 0;
	m_numFragments = 0;
	m_rpStart.release();
}

//==============================================================================
// BufferRange::asString
//
// Create a string from the BufferRange
//==============================================================================
String BufferRange::asString() const
{
	if(m_numFragments == 0)
	{
		return String();
	}
	else if(m_numFragments == 1)
	{
		Fragment frag=getFragment(0);
		return String(frag.pData, frag.length);
	}
	else
	{
		String strRet;
		for(size_t i=0; i<m_numFragments; i++)
		{
			Fragment frag=getFragment(i);
			strRet.append(frag.pData, frag.length);
		}
		return strRet;
	}
}

//==============================================================================
// BufferRange::appendToString
//
// Append the characters from this BufferRange to the passed string.
//==============================================================================
void BufferRange::appendToString(String& strRet) const
{
	for(size_t i=0; i<m_numFragments; i++)
	{
		Fragment frag=getFragment(i);
		strRet.append(frag.pData, frag.length);
	}
}

//==============================================================================
// BufferRange::getNumFragments
//
// Returns the number of buffer fragments represented by this BufferRange
//==============================================================================
size_t BufferRange::getNumFragments() const
{
	return m_numFragments;
}

//==============================================================================
// BufferRange::getFragment
//
// Returns the requested buffer Fragment
//==============================================================================
BufferRange::Fragment BufferRange::getFragment(size_t index) const
{
	QC_DBG_ASSERT(index < m_numFragments);

	//
	// Locate the desired fragment
	//
	Buffer* pBuffer = m_rpStart.get();
	size_t fragmentIndex=0;
	size_t charsRemaining = m_size;
	size_t offset = m_startOffset;
	
	while(fragmentIndex < index && pBuffer)
	{
		charsRemaining -= (pBuffer->m_used - offset);
		offset = 0;
		pBuffer = pBuffer->m_rpNext.get();
		fragmentIndex++;
	}

	size_t fragSize = (index == m_numFragments-1) ? charsRemaining 
	                                              : ((pBuffer->m_used)-offset);

	QC_DBG_ASSERT(fragmentIndex == index);
	return Fragment(pBuffer->m_pData+offset, fragSize);
}

QC_XML_NAMESPACE_END

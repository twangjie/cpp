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

#include "ScannerPosition.h"
#include "Entity.h"

QC_XML_NAMESPACE_BEGIN

const String sNull;

ScannerPosition::ScannerPosition(Buffer* pBuffer) :
	m_pNext(NULL),
	m_rpBuffer(pBuffer),
	m_bufferOffset(0),
	m_streamPosition(),
	m_eofAction(softEOF)
{
}

ScannerPosition::ScannerPosition(Buffer* pBuffer,
                                 const StreamPosition& position) :
	m_pNext(NULL),
	m_rpBuffer(pBuffer),
	m_bufferOffset(0),
	m_streamPosition(position),
	m_eofAction(softEOF)
{
}

void ScannerPosition::setNextPosition(const ScannerPosition& next, EOFAction eofAction)
{
	QC_DBG_ASSERT(m_pNext == NULL);
	delete m_pNext;
	m_pNext = new ScannerPosition(next);
	m_eofAction = eofAction;
}

//==============================================================================
// ScannerPosition::getEntity
//
//==============================================================================
const Entity& ScannerPosition::getEntity() const
{
	return m_rpBuffer->getEntity();
}

//==============================================================================
// ScannerPosition::getColumnNumber
//
//==============================================================================
size_t ScannerPosition::getColumnNumber() const
{
	return m_streamPosition.getColNo();
}

//==============================================================================
// ScannerPosition::getLineNumber
//
//==============================================================================
size_t ScannerPosition::getLineNumber() const
{
	return m_streamPosition.getLineNo();
}

//==============================================================================
// ScannerPosition::getOffset
//
//==============================================================================
size_t ScannerPosition::getOffset() const
{
	return m_streamPosition.getOffset();
}

//==============================================================================
// ScannerPosition::getPublicId
//
//==============================================================================
String ScannerPosition::getPublicId() const
{
	if(m_rpBuffer)
	{
		return m_rpBuffer->getEntity().getPublicId();
	}
	else
	{
		return sNull;
	}
}

//==============================================================================
// ScannerPosition::getSystemId
//
//==============================================================================
String ScannerPosition::getSystemId() const
{
	if(m_rpBuffer)
	{
		return m_rpBuffer->getEntity().getSystemId();
	}
	else
	{
		return sNull;
	}
}

//==============================================================================
// ScannerPosition::getResolvedSystemId()
//
//==============================================================================
String ScannerPosition::getResolvedSystemId() const
{
	if(m_rpBuffer)
	{
		return m_rpBuffer->getEntity().getResolvedSystemId();
	}
	else
	{
		return String();
	}
}

//==============================================================================
// ScannerPosition::getStreamPosition
//
//==============================================================================
const StreamPosition& ScannerPosition::getStreamPosition() const
{
	return m_streamPosition;
}

//==============================================================================
// ScannerPosition::isValid
//
//==============================================================================
bool ScannerPosition::isValid() const
{
	return (!m_rpBuffer.isNull());
}

QC_XML_NAMESPACE_END

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

#include "InternalEntity.h"
#include "Scanner.h"
#include "BufferRange.h"

#include "QcCore/base/debug.h"

QC_XML_NAMESPACE_BEGIN

InternalEntity::InternalEntity(EntityType type, const Entity* pParent, bool bExternallyDeclared, 
							   const String& name,
							   Reader* pReader, const StreamPosition& location,
							   bool bSimpleData) : 
	Entity(type, pParent, bExternallyDeclared, name),
	m_rpReader(pReader),
	m_bSimpleData(bSimpleData)
{
	m_startPosition = Scanner::GetPosition(*this, location);
}

InternalEntity::~InternalEntity()
{
}

//==============================================================================
// InternalEntity::getStartPosition
//
//==============================================================================
ScannerPosition InternalEntity::getStartPosition()
{
	return m_startPosition;
}

//==============================================================================
// InternalEntity::isParsed
//
//==============================================================================
bool InternalEntity::isParsed() const
{
	return true; // Internal entities are always parsed
}

//==============================================================================
// InternalEntity::isExternalEntity
//
//==============================================================================
bool InternalEntity::isExternalEntity() const
{
	return false; // Internal entities are never external!
}

//==============================================================================
// InternalEntity::isInternalEntity
//
//==============================================================================
bool InternalEntity::isInternalEntity() const
{
	return true;
}

//==============================================================================
// InternalEntity::load
//
//==============================================================================
void InternalEntity::load()
{
	// Internal entities are always loaded
}

//==============================================================================
// InternalEntity::getReader
//
//==============================================================================
AutoPtr<Reader> InternalEntity::getReader() const
{
	return m_rpReader;
}

//==============================================================================
// InternalEntity::getBufferRange
//
// Return a BufferRange representing the entire entity.
//==============================================================================
BufferRange InternalEntity::getBufferRange() const
{
	QC_DBG_ASSERT(m_bSimpleData);
	return Scanner::GetEntityBuffer(m_startPosition);
}

//==============================================================================
// InternalEntity::isSimpleData
//
// Return a flag indicating that this InternalEntity contains only
// simple data, ie no markup or entity references.
//==============================================================================
bool InternalEntity::isSimpleData() const
{
	return m_bSimpleData;
}

QC_XML_NAMESPACE_END


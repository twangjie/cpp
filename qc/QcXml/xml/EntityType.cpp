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

#include "EntityType.h"

QC_XML_NAMESPACE_BEGIN

EntityType::EntityType(Type eType) :
	m_eType(eType)
{
}

EntityType::EntityType(const EntityType& rhs)
{
	*this = rhs;
}

EntityType& EntityType::operator=(const EntityType& rhs)
{
	m_eType = rhs.m_eType;
	return *this;
}

bool EntityType::operator==(const EntityType& rhs) const
{
	return (m_eType == rhs.m_eType);
}

bool EntityType::operator!=(const EntityType& rhs) const
{
	return (m_eType != rhs.m_eType);
}

bool EntityType::operator==(Type eType) const
{
	return (m_eType == eType);
}

bool EntityType::operator!=(Type eType) const
{
	return (m_eType != eType);
}

QC_XML_NAMESPACE_END

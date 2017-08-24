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

#include "Entity.h"

QC_XML_NAMESPACE_BEGIN

//==============================================================================
// Entity::Entity
//
//==============================================================================
Entity::Entity(EntityType type, const Entity* pParent, bool bExternallyDeclared) :
	DTDObject(bExternallyDeclared),
	m_type(type),
	m_pParent(pParent)
{
}

//==============================================================================
// Entity::Entity
//
//==============================================================================
Entity::Entity(EntityType type, const Entity* pParent, bool bExternallyDeclared, const String& name) : 
	DTDObject(bExternallyDeclared),
	m_type(type),
	m_name(name),
	m_pParent(pParent)
{
}

//==============================================================================
// Entity::~Entity
//
//==============================================================================
Entity::~Entity()
{
}

//==============================================================================
// Entity::getType
//
//==============================================================================
EntityType Entity::getType() const
{
	return m_type;
}

//==============================================================================
// Entity::getName
//
//==============================================================================
const String& Entity::getName() const
{
	return m_name;
}

bool Entity::operator==(const Entity& rhs) const
{
	return (this == &rhs); 
}

bool Entity::operator!=(const Entity& rhs) const
{
	return !(*this == rhs); 
}

//==============================================================================
// Entity::getResolvedSystemId
//
// Only ExternalEntities have SystemIDs, so delegate this to parent who will
// know what to do with it.
//==============================================================================
String Entity::getResolvedSystemId() const
{
	// guard against recursion
	QC_DBG_ASSERT(getParent() != 0);
	return getParent()->getResolvedSystemId();
}

//==============================================================================
// Entity::getBaseURI
//
// Only ExternalEntities have base URIs, so delegate this to parent who will
// know what to do with it.
//==============================================================================
String Entity::getBaseURI() const
{
	// guard against recursion
	QC_DBG_ASSERT(getParent() != 0);
	return getParent()->getBaseURI();
}

//==============================================================================
// Entity::getPublicId
//
// Only ExternalEntities have PublicIds, so delegate this to parent who will
// know what to do with it.
//==============================================================================
String Entity::getPublicId() const
{
	// guard against recursion
	QC_DBG_ASSERT(getParent() != 0);
	return getParent()->getPublicId();
}

//==============================================================================
// Entity::getSystemId
//
// Only ExternalEntities have SystemIds, so delegate this to parent who will
// know what to do with it.
//==============================================================================
String Entity::getSystemId() const
{
	// guard against recursion
	QC_DBG_ASSERT(getParent() != 0);
	return getParent()->getSystemId();
}

//==============================================================================
// Entity::getParent
//
//==============================================================================
const Entity* Entity::getParent() const
{
	return m_pParent;
}

//==============================================================================
// Entity::getTopParent
//
//==============================================================================
const Entity* Entity::getTopParent() const
{
	if(m_pParent)
		return m_pParent->getTopParent();
	else
		return this;
}

//==============================================================================
// Entity::validate
//
// Currently just a base implmentation that does nothing.
// See ExternalEntity if you want an active implementation.
//==============================================================================
void Entity::validate(ParserImpl& /*parser*/) const
{
}

QC_XML_NAMESPACE_END


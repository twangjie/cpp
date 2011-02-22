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

#include "CatalogEntry.h"

QC_XMLCAT_NAMESPACE_BEGIN

CatalogEntry::CatalogEntry(const String& identifier, EntryType eEntryType,
						   CatalogResolver::Preference ePreference) :
	m_identifier(identifier),
	m_eEntryType(eEntryType),
	m_ePreference(ePreference)
{
}

CatalogEntry::~CatalogEntry()
{
}

//==============================================================================
// CatalogEntry::getIdentifier
//
//==============================================================================
const String& CatalogEntry::getIdentifier() const
{
	return m_identifier;
}

//==============================================================================
// CatalogEntry::getEntryType
//
//==============================================================================
CatalogEntry::EntryType CatalogEntry::getEntryType() const
{
	return m_eEntryType;
}

//==============================================================================
// CatalogEntry::getPreference
//
//==============================================================================
CatalogResolver::Preference CatalogEntry::getPreference() const
{
	return m_ePreference;
}

QC_XMLCAT_NAMESPACE_END

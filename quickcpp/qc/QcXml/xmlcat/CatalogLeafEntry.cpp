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

#include "CatalogLeafEntry.h"

QC_XMLCAT_NAMESPACE_BEGIN

CatalogLeafEntry::CatalogLeafEntry(const String& identifier, EntryType eEntryType,
                                   CatalogResolver::Preference ePreference,
                                   const String& uriReference) :
	CatalogEntry(identifier, eEntryType, ePreference),
	m_uriReference(uriReference)
{
}

//==============================================================================
// CatalogLeafEntry::getUriReference
//
//==============================================================================
const String& CatalogLeafEntry::getUriReference() const
{
	return m_uriReference;
}

//==============================================================================
// CatalogLeafEntry::resolveEntity
//
//==============================================================================
bool CatalogLeafEntry::resolveEntity(const String& publicId, const String& systemId,
                                     const CatalogFile::List& /*catList*/,
                                     String& returnUri, bool& /*bDelegated*/) const
{
	if(m_eEntryType == SYSTEM && m_identifier == systemId)
	{
		returnUri = m_uriReference;
		return true;
	}
	else if(m_eEntryType == REWRITE_SYSTEM && 
			systemId.substr(0, m_identifier.length()) == m_identifier)
	{
		returnUri = m_uriReference + systemId.substr(m_identifier.length());
		return true;
	}
	else if(m_ePreference == CatalogResolver::SYSTEM && !systemId.empty())
	{
		return false;
	}
	else if(m_eEntryType == PUBLIC && m_identifier == publicId)
	{
		returnUri = m_uriReference;
		return true;
	}
	else
	{
		return false;
	}
}

//==============================================================================
// CatalogLeafEntry::resolveURI
//
//==============================================================================
bool CatalogLeafEntry::resolveURI(const String& uri, const CatalogFile::List& /*catList*/,
                                  String& returnUri, bool& /*bDelegated*/) const
{
	if(m_eEntryType == URI && m_identifier  == uri)
	{
		returnUri = m_uriReference;
		return true;
	}
	else if(m_eEntryType == REWRITE_URI && 
			uri.substr(0, m_identifier.length()) == m_identifier)
	{
		returnUri = m_uriReference + uri.substr(m_identifier.length());
		return true;
	}
	else
	{
		return false;
	}
}

QC_XMLCAT_NAMESPACE_END

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

#include "CatalogDelegatorEntry.h"

QC_XMLCAT_NAMESPACE_BEGIN

const String sBlank;

CatalogDelegatorEntry::CatalogDelegatorEntry(const String& identifier, EntryType eEntryType,
                                             CatalogResolver::Preference ePreference,
                                             const URL& catalogUrl,
											 const CatalogResolver& resolver) :
	CatalogEntry(identifier, eEntryType, ePreference),
	m_catalogSet(resolver)
{
	m_catalogSet.addFile(catalogUrl);
}

//==============================================================================
// CatalogDelegatorEntry::getCatalogSet
//
//==============================================================================
const CatalogSet& CatalogDelegatorEntry::getCatalogSet() const
{
	return m_catalogSet;
}

//==============================================================================
// CatalogDelegatorEntry::resolveEntity
//
// When delagating a request to a new catalog, only the public Id or system Id
// that caused the match is used for further matching.
//
//==============================================================================
bool CatalogDelegatorEntry::resolveEntity(const String& publicId, const String& systemId,
	const CatalogFile::List& catList,
	String& returnUri, bool& bDelegated) const
{
	if(m_ePreference == CatalogResolver::SYSTEM && !systemId.empty())
	{
		return false;
	}

	if(m_eEntryType == PUBLIC && publicId.substr(0, m_identifier.size()) == m_identifier)
	{
		bDelegated = true;
		bool bDummy = false;
		return m_catalogSet.resolveEntity(publicId, sBlank, catList, returnUri, bDummy);
	}
	else if(m_eEntryType == SYSTEM && systemId.substr(0, m_identifier.size()) == m_identifier)
	{
		bDelegated = true;
		bool bDummy = false;
		return m_catalogSet.resolveEntity(sBlank, systemId, catList, returnUri, bDummy);
	}
	else
	{
		return false;
	}
}

//==============================================================================
// CatalogDelegatorEntry::resolveURI
//
//==============================================================================
bool CatalogDelegatorEntry::resolveURI(const String& uri, 
                                       const CatalogFile::List& catList,
                                       String& returnUri, bool& bDelegated) const
{
	if(m_eEntryType == URI && uri.substr(0, m_identifier.size()) == m_identifier)
	{
		bDelegated = true;
		bool bDummy = false;
		return m_catalogSet.resolveURI(uri, catList, returnUri, bDummy);
	}
	else
	{
		return false;
	}
}

QC_XMLCAT_NAMESPACE_END

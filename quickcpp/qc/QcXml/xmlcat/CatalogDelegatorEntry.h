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
// Class: CatalogDelegatorEntry
// 
// Overview
// --------
// A CatalogEntry derivative for delegateURI type catalog entries.
//
//==============================================================================

#ifndef QC_XMLCAT_CatalogDelegatorEntry_h
#define QC_XMLCAT_CatalogDelegatorEntry_h

#include "defs.h"
#include "CatalogEntry.h"
#include "CatalogSet.h"

QC_XMLCAT_NAMESPACE_BEGIN

class QC_XMLCAT_PKG CatalogDelegatorEntry : public CatalogEntry
{
public:

	CatalogDelegatorEntry(const String& identifier,
	                      EntryType eEntryType,
				          CatalogResolver::Preference ePreference,
						  const URL& catalogUrl,
						  const CatalogResolver& resolver);

	const CatalogSet& getCatalogSet() const;

	bool operator<(const CatalogDelegatorEntry& rhs) const;

	// Resolver functions
	virtual bool resolveEntity(const String& publicId, const String& systemId,
	                           const CatalogFile::List& catList,
	                           String& returnUri, bool& bDelegated) const;

	virtual bool resolveURI(const String& uri, const CatalogFile::List& catList,
	                        String& returnUri, bool& bDelegated) const;

private:
	CatalogSet m_catalogSet;
};

QC_XMLCAT_NAMESPACE_END

#endif //QC_XMLCAT_CatalogDelegatorEntry_h

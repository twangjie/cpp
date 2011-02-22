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
// Class: CatalogEntry
// 
// Overview
// --------
// A CatalogEntry derivative for leaf nodes.
//
//==============================================================================

#ifndef QC_XMLCAT_CatalogEntry_h
#define QC_XMLCAT_CatalogEntry_h

#include "defs.h"
#include "CatalogResolver.h"
#include "CatalogFile.h"

QC_XMLCAT_NAMESPACE_BEGIN

class QC_XMLCAT_PKG CatalogEntry
{
public:
	enum EntryType {PUBLIC, SYSTEM, REWRITE_SYSTEM, URI, REWRITE_URI};

	CatalogEntry(const String& identifier,
	             EntryType eEntryType,
				 CatalogResolver::Preference ePreference);

	virtual ~CatalogEntry();

	const String& getIdentifier() const;

	EntryType getEntryType() const;

	CatalogResolver::Preference getPreference() const;

	// Resolver functions
	virtual bool resolveEntity(const String& publicId, const String& systemId,
	                           const CatalogFile::List& catList,
	                           String& returnUri, bool& bDelegated) const =0;

	virtual bool resolveURI(const String& uri, const CatalogFile::List& catList,
	                        String& returnUri, bool& bDelegated) const =0;

protected:
	String m_identifier;
	EntryType m_eEntryType;
	CatalogResolver::Preference m_ePreference;
};

QC_XMLCAT_NAMESPACE_END

#endif //QC_XMLCAT_CatalogEntry_h

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
// Class: CatalogResolver
// 
// Overview
// --------
// A class to act as the ublic interface to XML Entity Resolution using
// Oasis XML Catalogs.
//
// The CatalogResolver holds a CatalogSet which contains a number of
// CatalogFiles, each CatalogFile representing the catalog entries from a 
// particular catalog file.
// 
// This is an implementation of the (draft) specification from
// the Oasis Entity Resolution Technical Committee
// http://www.oasis-open.org/committees/entityspec-2001-05-25.html
//
//==============================================================================

#ifndef QC_XMLCAT_CatalogResolver_h
#define QC_XMLCAT_CatalogResolver_h

#include "defs.h"

QC_XMLCAT_NAMESPACE_BEGIN

class CatalogEventHandler;
class CatalogSet;

class QC_XMLCAT_PKG CatalogResolver : public virtual ManagedObject
{

public:

	enum Preference {PUBLIC, SYSTEM};

	CatalogResolver();
	~CatalogResolver();

	void addPrimaryCatalog(const URL& url);
	void addSecondaryCatalog(const URL& url);
	void resetSecondaryCatalog();

	bool resolveEntity(const String& publicId, const String& systemId,
                       String& returnUri) const;

	bool resolveURI(const String& uri, String& returnUri) const;

	CatalogEventHandler* getEventHandler() const;
	void setEventHandler(CatalogEventHandler* pHandler);

	static void SetGlobalPreference(Preference ePreference);
	static Preference GetGlobalPreference();
	static bool IsPublicidURN(const String& urn);
	static String UnwrapPublicidURN(const String& urn);
	static String GetCatalogPITarget();

private:
	CatalogSet* m_pPrimaryCatalogSet;
	CatalogSet* m_pSecondaryCatalogSet;
	CatalogEventHandler* m_pEventHandler;
	
	static Preference s_eGlobalPreference;
};

QC_XMLCAT_NAMESPACE_END

#endif //QC_XMLCAT_CatalogResolver_h

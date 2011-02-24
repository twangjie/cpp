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
// Class: CatalogFile
// 
// Overview
// --------
// A class to encapsulate an XML Entity Resolution Catalog document.
// 
// This is an implementation of the (draft) specification from
// the Oasis Entity Resolution Technical Committee
// http://www.oasis-open.org/committees/entityspec-2001-05-07.html
//
//==============================================================================

#ifndef QC_XMLCAT_CatalogFile_h
#define QC_XMLCAT_CatalogFile_h

#include "defs.h"
#include <deque>
#include <list>

QC_XMLCAT_NAMESPACE_BEGIN

class CatalogSet;
class CatalogEntry;
class CatalogDelegatorEntry;

class QC_XMLCAT_PKG CatalogFile : public virtual QCObject
{
	friend class CatalogParserHandler;

public:

	CatalogFile(const URL& url, CatalogSet& catalogSet);
	~CatalogFile();

	typedef std::list<String> List;

	bool resolveEntity(const String& publicId, const String& systemId,
	                   const List& catList, 
	                   String& returnUri, bool& bDelegated) const;

	bool resolveURI(const String& uri, const List& catList, 
	                String& returnUri, bool& bDelegated) const;

	CatalogSet& getEnclosingSet() const;

	const URL& getURL() const;

protected:

	void open();
	void addLeafEntry(CatalogEntry* pCatalogEntry);
	void addDelegatorEntry(CatalogDelegatorEntry* pCatalogEntry);
	bool testCircularReference(const List& catList) const;

private:

	typedef std::deque<CatalogEntry*> CatalogEntryList;
	typedef std::deque<CatalogDelegatorEntry*> CatalogDelegatorEntryList;
	CatalogEntryList m_uriEntryList;
	CatalogEntryList m_rewriteUriEntryList;
	CatalogEntryList m_publicEntryList;
	CatalogEntryList m_systemEntryList;
	CatalogEntryList m_rewriteSystemEntryList;
	CatalogDelegatorEntryList m_uriDelegatorList;
	CatalogDelegatorEntryList m_publicDelegatorList;
	CatalogDelegatorEntryList m_systemDelegatorList;

	URL m_url;
	CatalogSet& m_enclosingSet;
	bool m_bOpen;
};

QC_XMLCAT_NAMESPACE_END

#endif //QC_XMLCAT_CatalogFile_h


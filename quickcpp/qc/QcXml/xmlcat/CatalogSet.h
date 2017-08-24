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
// Class: CatalogSet
// 
// Overview
// --------
// A container class for a set of CatalogFiles.
//
//==============================================================================

#ifndef QC_XMLCAT_CatalogSet_h
#define QC_XMLCAT_CatalogSet_h

#include "defs.h"
#include "CatalogFile.h"
#include <list>

QC_XMLCAT_NAMESPACE_BEGIN

class CatalogResolver;

class QC_XMLCAT_PKG CatalogSet
{
	friend class CatalogParserHandler;
	friend class CatalogSetDelegatorEntry;

public:
	
	CatalogSet(const CatalogResolver& resolver);

	bool resolveEntity(const String& publicId, const String& systemId,
	                   const CatalogFile::List& catList, String& returnUri,
					   bool& bReturnDelegated) const;

	bool resolveURI(const String& uri, const CatalogFile::List& catList,
	                String& returnUri, bool& bReturnDelegated) const;

	void addFile(const URL& catalogFile, const CatalogFile* pFrom=0);

	const CatalogResolver& getResolver() const;

private:
	typedef std::list< AutoPtr<CatalogFile> > CatalogFiles;
	CatalogFiles m_catalogFiles;
	const CatalogResolver& m_resolver;
};

QC_XMLCAT_NAMESPACE_END

#endif //QC_XMLCAT_CatalogSet_h


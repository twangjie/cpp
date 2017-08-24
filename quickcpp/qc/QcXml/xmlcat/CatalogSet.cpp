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

#include "CatalogSet.h"
#include "CatalogFile.h"
#include "CatalogResolver.h"
#include "CatalogEventHandler.h"

QC_XMLCAT_NAMESPACE_BEGIN

CatalogSet::CatalogSet(const CatalogResolver& resolver) :
	m_resolver(resolver)
{
}

//==============================================================================
// CatalogSet::resolveEntity
//
// Iterate thru all the CatalogFiles attempting to find a match.  As soon as a 
// positive match is found, or the request has been delegated to another
// CatalogSet, we quit.
//==============================================================================
bool CatalogSet::resolveEntity(const String& publicId, const String& systemId,
                               const CatalogFile::List& catList,
                               String& returnUri,
							   bool& bReturnDelegated) const
{
	bReturnDelegated = false;
	CatalogFiles::const_iterator i=m_catalogFiles.begin();
	for(; i!=m_catalogFiles.end() && !bReturnDelegated; ++i)
	{
		const AutoPtr<CatalogFile>& rpCatalogFile = *i;

		if(rpCatalogFile->resolveEntity(publicId, systemId, catList,
		                                returnUri, bReturnDelegated))
		{
			return true;
		}
	}
	return false;
}

//==============================================================================
// CatalogSet::resolveURI
//
// Iterate thru all the CatalogSet parts (the CatalogSet list in the parlance
// of the Oasis specification) attempting to find a match.  As soon as a 
// positive match is found, or the request has been delegated to another
// CatalogSet, we quit.
//==============================================================================
bool CatalogSet::resolveURI(const String& uri, const CatalogFile::List& catList,
                            String& returnUri,
							bool& bReturnDelegated) const
{
	bReturnDelegated = false;
	CatalogFiles::const_iterator i=m_catalogFiles.begin();
	for(; i!=m_catalogFiles.end() && !bReturnDelegated; ++i)
	{
		const AutoPtr<CatalogFile>& rpCatalogFile = *i;
		if(rpCatalogFile->resolveURI(uri, catList, returnUri, bReturnDelegated))
		{
			return true;
		}
	}

	return false;
}
	
//==============================================================================
// CatalogSet::addFile
//
//==============================================================================
void CatalogSet::addFile(const URL& catalogFile, const CatalogFile* pFrom)
{
	//
	// Test if the file already exists in our set.  If so, it 
	// cannot be allowed in because it would imply one of
	// a) A duplicate nextCatalog element
	// b) A circular nextCatalog reference
	//

	CatalogFiles::const_iterator i=m_catalogFiles.begin();
	for(; i!=m_catalogFiles.end(); ++i)
	{
		const AutoPtr<CatalogFile>& rpCatalogFile = *i;
		if(rpCatalogFile->getURL().sameFile(catalogFile))
		{
			// Error! duplicate
			CatalogEventHandler* pHandler = getResolver().getEventHandler();
			if(pHandler)
			{
				String errMsg = QC_T("the catalog entry file: ");
				errMsg += catalogFile.toExternalForm();
				errMsg += QC_T(" is already contained in the set of catalog files");
				String fileName;
				if(pFrom)
				{
					fileName = pFrom->getURL().getFile();
				}
				pHandler->onCatalogError(CatalogEventHandler::Error, 0, errMsg, fileName, 0, 0);
			}
			return;
		}
	}

	AutoPtr<CatalogFile> rpNewFile(new CatalogFile(catalogFile, *this));
	m_catalogFiles.push_back(rpNewFile);
}

//==============================================================================
// CatalogSet::getResolver
//
//==============================================================================
const CatalogResolver& CatalogSet::getResolver() const
{
	return m_resolver;
}

QC_XMLCAT_NAMESPACE_END

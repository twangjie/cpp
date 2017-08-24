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

#include "CatalogFile.h"
#include "CatalogSet.h"
#include "CatalogResolver.h"
#include "CatalogEntry.h"
#include "CatalogDelegatorEntry.h"
#include "CatalogEventHandler.h"
#include "CatalogParserHandler.h"

#include "QcCore/base/debug.h"
#include "QcCore/base/Exception.h"
#include "QcCore/util/stlutils.h"
#include "QcXml/xml/Parser.h"
#include "QcXml/xml/ParserFactory.h"
#include "QcXml/xml/XMLFeatures.h"

#include <algorithm>

QC_XMLCAT_NAMESPACE_BEGIN

using namespace xml;
using namespace util;

bool sortEntries(CatalogEntry* const& p1, CatalogEntry* const& p2)
{
	return !(p1->getIdentifier().size() < p2->getIdentifier().size());
}

//==============================================================================
// <template> bool resolveEntityAll
//
// Iterate over a collection of CatalogEntries calling resolveEntity().  Stop
// if a match is found.
//==============================================================================
template<typename Iter>
bool resolveEntityAll(Iter begin, Iter end, const String& publicId,
					  const String& systemId, const CatalogFile::List& catList,
                      String& returnUri, bool& bDelegated)
{
	for(; begin!=end; ++begin)
	{
		if((*begin)->resolveEntity(publicId, systemId, catList, returnUri, bDelegated))
		{
			return true;
		}
	}
	return false;
}


//==============================================================================
// <template> bool resolveUriAll
//
// Iterate over a collection of CatalogEntries calling resolveURI().  Stop
// if a match is found.
//==============================================================================
template<typename Iter>
bool resolveUriAll(Iter begin, Iter end, const String& uri, 
                   const CatalogFile::List& catList,
                   String& returnUri, bool& bDelegated)
{
	for(; begin!=end; ++begin)
	{
		if((*begin)->resolveURI(uri, catList, returnUri, bDelegated))
		{
			return true;
		}
	}
	return false;
}


CatalogFile::CatalogFile(const URL& url, CatalogSet& enclosingSet) :
	m_url(url),
	m_enclosingSet(enclosingSet),
	m_bOpen(false)
{
}

//==============================================================================
// CatalogFile destructor
//
// Manage the memory of the contained lists of pointers
//==============================================================================
CatalogFile::~CatalogFile()
{
	deleteAll(m_uriEntryList.begin(), m_uriEntryList.end());
	deleteAll(m_rewriteUriEntryList.begin(), m_rewriteUriEntryList.end());
	deleteAll(m_publicEntryList.begin(), m_publicEntryList.end());
	deleteAll(m_systemEntryList.begin(), m_systemEntryList.end());
	deleteAll(m_rewriteSystemEntryList.begin(), m_rewriteSystemEntryList.end());
	deleteAll(m_uriDelegatorList.begin(), m_uriDelegatorList.end());
	deleteAll(m_publicDelegatorList.begin(), m_publicDelegatorList.end());
	deleteAll(m_systemDelegatorList.begin(), m_systemDelegatorList.end());
}


//==============================================================================
// CatalogFile::resolveEntity
//
//==============================================================================
bool CatalogFile::resolveEntity(const String& publicId, const String& systemId,
								const List& catList,
	                            String& returnUri, bool& bDelegated) const
{
	bDelegated = false;

	if(testCircularReference(catList))
	{
		return false;
	}

	//
	// Add the name of this catalog to the CatalogList so that circularity may be
	// detected.
	//
	List myList = catList;
	myList.push_back(m_url.toExternalForm());

	//
	// Allow lazy reading of the catalog file by opening it on demand
	//
	if(!m_bOpen)
	{
		const_cast<CatalogFile*>(this)->open();
	}

	if(!systemId.empty())
	{
		if(resolveEntityAll(m_systemEntryList.begin(), m_systemEntryList.end(), publicId, systemId, myList, returnUri, bDelegated))
		{
			return true;
		}

		if(resolveEntityAll(m_rewriteSystemEntryList.begin(), m_rewriteSystemEntryList.end(), publicId, systemId, myList, returnUri, bDelegated))
		{
			return true;
		}

		if(resolveEntityAll(m_systemDelegatorList.begin(), m_systemDelegatorList.end(), publicId, systemId, myList, returnUri, bDelegated))
		{
			return true;
		}
		else if(bDelegated)
		{
			return false;
		}
	}


	if(!publicId.empty())
	{
		if(resolveEntityAll(m_publicEntryList.begin(), m_publicEntryList.end(), publicId, systemId, myList, returnUri, bDelegated))
		{
			return true;
		}

		if(resolveEntityAll(m_publicDelegatorList.begin(), m_publicDelegatorList.end(), publicId, systemId, myList, returnUri, bDelegated))
		{
			return true;
		}
		else if(bDelegated)
		{
			return false;
		}
	}

	return false;
}

//==============================================================================
// CatalogFile::resolveURI
//
//==============================================================================
bool CatalogFile::resolveURI(const String& uri, const List& catList,
                             String& returnUri, bool& bDelegated) const
{
	if(testCircularReference(catList))
	{
		return false;
	}

	//
	// Add the name of this catalog to the CatalogList so that circularity may be
	// detected.
	//
	List myList = catList;
	myList.push_back(m_url.toExternalForm());

	bDelegated = false;

	//
	// Allow lazy reading of the catalog file by opening it on demand
	//
	if(!m_bOpen)
	{
		const_cast<CatalogFile*>(this)->open();
	}

	if(resolveUriAll(m_uriEntryList.begin(), m_uriEntryList.end(), uri, myList, returnUri, bDelegated))
	{
		return true;
	}

	if(resolveUriAll(m_rewriteUriEntryList.begin(), m_rewriteUriEntryList.end(), uri, myList, returnUri, bDelegated))
	{
		return true;
	}

	if(resolveUriAll(m_uriDelegatorList.begin(), m_uriDelegatorList.end(), uri, myList, returnUri, bDelegated))
	{
		return true;
	}
	
	return false;
}

//==============================================================================
// CatalogFile::testCircularReference
//
// Test if the current CatalogFile is already present in the call stack.
//==============================================================================
bool CatalogFile::testCircularReference(const List& catList) const
{
	if(std::find(catList.begin(), catList.end(), m_url.toExternalForm()) != catList.end())
	{
		CatalogEventHandler* pHandler = m_enclosingSet.getResolver().getEventHandler();
		if(pHandler)
		{
			String errMsg = QC_T("circular reference to ");
			errMsg += m_url.getFile();
			pHandler->onCatalogError(CatalogEventHandler::Error, 0, errMsg, m_url.getFile(), 0, 0);
		}
		return true;
	}
	return false;
}

//==============================================================================
// CatalogFile::open
//
//==============================================================================
void CatalogFile::open()
{
	AutoPtr<Parser> rpParser = ParserFactory::CreateXMLParser();

	CatalogParserHandler parserEventHandler(*this, *rpParser.get());
	rpParser->setContentEventHandler(&parserEventHandler);
	rpParser->setErrorEventHandler(&parserEventHandler);
	rpParser->enableFeature(XMLFeatures::NamespaceSupport, true);

	try
	{
		rpParser->parseURL(m_url.toExternalForm());
	}
	catch(Exception& /*e*/)
	{
	}

	//
	// Sort the delegator lists so that the delegator with the longest
	// uri reference appear first.
	//
	std::stable_sort(m_systemDelegatorList.begin(), m_systemDelegatorList.end(), sortEntries);
	std::stable_sort(m_publicDelegatorList.begin(), m_publicDelegatorList.end(), sortEntries);
	std::stable_sort(m_uriDelegatorList.begin(), m_uriDelegatorList.end(), sortEntries);
	
	// And likewise for rewrite entries
	std::stable_sort(m_rewriteSystemEntryList.begin(), m_rewriteSystemEntryList.end(), sortEntries);
	std::stable_sort(m_rewriteUriEntryList.begin(), m_rewriteUriEntryList.end(), sortEntries);

	m_bOpen = true;
}

void CatalogFile::addLeafEntry(CatalogEntry* pCatalogEntry)
{
	QC_DBG_ASSERT(pCatalogEntry!=0);

	switch(pCatalogEntry->getEntryType())
	{
	case CatalogEntry::URI:
		m_uriEntryList.push_back(pCatalogEntry);
		break;
	case CatalogEntry::REWRITE_URI:
		m_rewriteUriEntryList.push_back(pCatalogEntry);
		break;
	case CatalogEntry::PUBLIC:
		m_publicEntryList.push_back(pCatalogEntry);
		break;
	case CatalogEntry::SYSTEM:
		m_systemEntryList.push_back(pCatalogEntry);
		break;
	case CatalogEntry::REWRITE_SYSTEM:
		m_rewriteSystemEntryList.push_back(pCatalogEntry);
		break;
	}
}

void CatalogFile::addDelegatorEntry(CatalogDelegatorEntry* pCatalogEntry)
{
	QC_DBG_ASSERT(pCatalogEntry!=0);

	switch(pCatalogEntry->getEntryType())
	{
	case CatalogEntry::URI:
		m_uriDelegatorList.push_back(pCatalogEntry);
		break;
	case CatalogEntry::PUBLIC:
		m_publicDelegatorList.push_back(pCatalogEntry);
		break;
	case CatalogEntry::SYSTEM:
		m_systemDelegatorList.push_back(pCatalogEntry);
		break;
	default:
		QC_DBG_ASSERT(false);
		break;
	}
}

//==============================================================================
// CatalogFile::getEnclosingSet
//
// Return the CatalogSet of which this is a part.
//==============================================================================
CatalogSet& CatalogFile::getEnclosingSet() const
{
	return m_enclosingSet;
}

//==============================================================================
// CatalogFile::getURL
//
// Return the URL for this Catalog entry file.
//==============================================================================
const URL& CatalogFile::getURL() const
{
	return m_url;
}

QC_XMLCAT_NAMESPACE_END

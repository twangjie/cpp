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

#include "CatalogResolver.h"
#include "CatalogSet.h"
#include "CatalogFile.h"

#include "QcCore/base/NumUtils.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/base/debug.h"
#include "QcCore/net/URLEncoder.h"

QC_XMLCAT_NAMESPACE_BEGIN

using net::URLEncoder;

static const String sPubId = QC_T("urn:publicid:");
static const String sNull;

CatalogResolver::Preference CatalogResolver::s_eGlobalPreference = CatalogResolver::PUBLIC;


//==============================================================================
// CatalogResolver::CatalogResolver
//
// Construction.
//
// Note, m_pEventHandler is an unowned pointer
//       m_pCatalogSet is owned.     
//==============================================================================
CatalogResolver::CatalogResolver() :
	m_pPrimaryCatalogSet(0),
	m_pSecondaryCatalogSet(0),
	m_pEventHandler(0)
{
	m_pPrimaryCatalogSet = new CatalogSet(*this);
}

//==============================================================================
// CatalogResolver::~CatalogResolver
//
//==============================================================================
CatalogResolver::~CatalogResolver()
{
	delete m_pPrimaryCatalogSet;
	delete m_pSecondaryCatalogSet;
}

//==============================================================================
// CatalogResolver::resolveEntity
//
// Public interface for resolving entities using a public and/or system
// identifier.
//==============================================================================
bool CatalogResolver::resolveEntity(const String& publicId, const String& systemId,
                            String& returnUri) const
{
	//
	// If the public identifier is a URI in the publicid namespace, it is
	// converted into a public identifier by unwrapping the URN.
	//
	String myPublicId = IsPublicidURN(publicId)
	                  ? UnwrapPublicidURN(publicId)
					  : StringUtils::NormalizeWhiteSpace(publicId);

	//
	// If the system identifier is a URI in the publicid namespace, it is
	// converted into a public identfier by unwrapping the URN.  In this case
	// one of the following must apply:-
	//
	// 1) No public identifier was provided
	// 2) The normalized public identifier is identical to the public 
	//    identifier constructed by unwrapping the URN
	// 3) The normalized public identifier is diffewrent to the supplied
	//    public identifier.  This is an error.  In this case the supplied
	//    system identifier is discarded.
	//
	// As we have no interest in reporting the error in (3), we simply
	// check that no public identifier has been provided before
	// overriding it with the contents of the system identifier.
	//
	bool bSystemIdISPublicId = IsPublicidURN(systemId);

	if(bSystemIdISPublicId && myPublicId.empty())
	{
		myPublicId = UnwrapPublicidURN(systemId);
	}

	const String& mySystemId = bSystemIdISPublicId
	                         ? sNull
                             : URLEncoder::RawEncode(systemId);

	CatalogFile::List catList;
	
	bool bDelegated = false;

	if(m_pPrimaryCatalogSet->resolveEntity(myPublicId, mySystemId, catList, returnUri, bDelegated))
	{
		return true;
	}
	else if(!bDelegated && m_pSecondaryCatalogSet)
	{
		return m_pSecondaryCatalogSet->resolveEntity(myPublicId, mySystemId, catList, returnUri, bDelegated);
	}
	else
	{
		return false;
	}
}

//==============================================================================
// CatalogResolver::resolveURI
//
// Iterate thru all the CatalogResolver parts (the CatalogResolver list in the parlance
// of the Oasis specification) attempting to find a match.  As soon as a 
// positive match is found, or the request has been delegated to another
// CatalogResolver, we quit.
//==============================================================================
bool CatalogResolver::resolveURI(const String& uri, String& returnUri) const
{
	bool bRet; // = false;
	//
	// If the URI is a URN in the publicid namespace, it is converted into
	// a public identifier by unwrapping the URN.  Resolution then continues
	// by following the semantics (and CatalogResolver entries) of external identifier
	// resolution (i.e. External Entity resolution).
	//
	CatalogFile::List catList;
	bool bDelegated = false;

	if(IsPublicidURN(uri))
	{
		const String& publicId = UnwrapPublicidURN(uri);
		bRet = m_pPrimaryCatalogSet->resolveEntity(publicId, sNull, catList, returnUri, bDelegated);
		bRet = bRet || !bDelegated && m_pSecondaryCatalogSet && m_pSecondaryCatalogSet->resolveEntity(publicId, sNull, catList, returnUri, bDelegated);
	}
	else
	{
	    bRet = m_pPrimaryCatalogSet->resolveURI(uri, catList, returnUri, bDelegated);
		bRet = bRet || !bDelegated && m_pSecondaryCatalogSet && m_pSecondaryCatalogSet->resolveURI(uri, catList, returnUri, bDelegated);
	}
	
	return bRet;
}
	
//==============================================================================
// CatalogResolver::addPrimaryCatalog
//
//==============================================================================
void CatalogResolver::addPrimaryCatalog(const URL& catalogFile)
{
	m_pPrimaryCatalogSet->addFile(catalogFile);
}

//==============================================================================
// CatalogResolver::addSecondaryCatalog
//
//==============================================================================
void CatalogResolver::addSecondaryCatalog(const URL& catalogFile)
{
	if(!m_pSecondaryCatalogSet)
	{
		m_pSecondaryCatalogSet = new CatalogSet(*this);
	}

	m_pSecondaryCatalogSet->addFile(catalogFile);
}

//==============================================================================
// CatalogResolver::resetSecondaryCatalog
//
//==============================================================================
void CatalogResolver::resetSecondaryCatalog()
{
	delete m_pSecondaryCatalogSet;
	m_pSecondaryCatalogSet = 0;
}

//==============================================================================
// CatalogResolver::SetGlobalPreference
//
//==============================================================================
void CatalogResolver::SetGlobalPreference(Preference ePreference)
{
	s_eGlobalPreference = ePreference;
}

//==============================================================================
// CatalogResolver::GetGlobalPreference
//
//==============================================================================
CatalogResolver::Preference CatalogResolver::GetGlobalPreference()
{
	return s_eGlobalPreference;
}

//==============================================================================
// CatalogResolver::getEventHandler
//
//==============================================================================
CatalogEventHandler* CatalogResolver::getEventHandler() const
{
	return m_pEventHandler;
}

//==============================================================================
// CatalogResolver::setErrorEventHandler
//
//==============================================================================
void CatalogResolver::setEventHandler(CatalogEventHandler* pHandler)
{
	m_pEventHandler = pHandler;
}

//==============================================================================
// CatalogResolver::IsPublicidURN
//
//==============================================================================
bool CatalogResolver::IsPublicidURN(const String& urn)
{
	if(urn.substr(0, sPubId.size()) == sPubId)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//==============================================================================
// CatalogResolver::UnwrapPublicidURN
//
// Unwrap a public id that has been encoded in the publicid urn namespace.
// This is perforemd in accordance with the internet draft:
// "A URN Namespace for Public Identifiers" by Normal Walsh et al.
//==============================================================================
String CatalogResolver::UnwrapPublicidURN(const String& urn)
{
	String ret;
	QC_DBG_ASSERT(IsPublicidURN(urn));
	
	//
	// Skip past the urn:publicid: prefix
	//
	size_t pos = sPubId.size();
	size_t size = urn.size();

	while(pos < size)
	{
		CharType x = urn[pos];
		switch(x)
		{
			case ':':
				ret += QC_T("//");
				break;
			case ';':
				ret += QC_T("::");
				break;
			case '+':
				ret += QC_T(" ");
				break;
			case '%':
				if(pos+2 < size)
				{
					String hex = urn.substr(pos+1,2);
					int n = NumUtils::ToInt(hex, 16);
					CharType c(n);
					ret += c;
					pos+=2;
				}
				break;
			default:
				ret += x;
		}
		pos++;
	}
	return ret;
}

//==============================================================================
// CatalogResolver::GetCatalogPITarget
//
// Return the reserved Processing Instruction Target for OASIS xml catalog
// entries.
//==============================================================================
String CatalogResolver::GetCatalogPITarget()
{
	return QC_T("oasis-xml-catalog");
}

QC_XMLCAT_NAMESPACE_END

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
// From http://www.oasis-open.org/committees/entity/spec-2001-05-17.html
//
// Section 5 . The XML Entity Catalog Format
//
// The Oasis XML Catalog Standard defines an entity catalog in XML.
// It consists of elements from the OASIS Entity Catalog Namespace,
// "urn:oasis:names:tc:entity:xmlns:xml:catalog".
// Elements and attributes from other namespaces are are allowed, but
// they must be ignored. If an element is ignored, all of its descendents are
// also ignored, regardless of their namespace.
//==============================================================================

#include "CatalogParserHandler.h"
#include "CatalogLeafEntry.h"
#include "CatalogDelegatorEntry.h"
#include "CatalogEventHandler.h"
#include "CatalogResolver.h"
#include "CatalogFile.h"
#include "CatalogSet.h"

#include "QcCore/base/debug.h"
#include "QcCore/base/StringUtils.h"
#include "QcXml/xml/Attribute.h"
#include "QcXml/xml/AttributeSet.h"
#include "QcXml/xml/XMLNames.h"
#include "QcXml/xml/Position.h"
#include "QcCore/net/URLEncoder.h"

QC_XMLCAT_NAMESPACE_BEGIN

using namespace xml;
using net::URLEncoder;

const String OasisCatalogNamespace = QC_T("urn:oasis:names:tc:entity:xmlns:xml:catalog");
const String GroupElementName = QC_T("group");
const String CatalogElementName = QC_T("catalog");

CatalogParserHandler::CatalogParserHandler(CatalogFile& catalogFile, const Parser& parser) :
	m_catalogFile(catalogFile),
	m_parser(parser)
{
}

CatalogParserHandler::~CatalogParserHandler()
{
}

//==============================================================================
// CatalogParserHandler::onStartElement
//
//==============================================================================
void CatalogParserHandler::onStartElement(const QName& elementName, bool /*bEmptyElement*/, const AttributeSet& attributes)
{
	CatalogSerialisationContext context = m_contextStack.size() ? m_contextStack.top() : CatalogSerialisationContext(m_catalogFile.m_url);
	
	if(context.m_bIncluded && elementName.getNamespaceURI() == OasisCatalogNamespace)
	{
		if(elementName.getLocalName() == CatalogElementName
		&& !m_contextStack.empty())
		{
			reportError(Fatal, QC_T("catalog element can only occur at the root"), m_parser.getCurrentPosition());
		}
		else if(elementName.getLocalName() == GroupElementName
		&& context.m_elementName != CatalogElementName)
		{
			reportError(Fatal, QC_T("group element can only occur within the catalog element"), m_parser.getCurrentPosition());
		}
		else if(elementName.getLocalName() == GroupElementName ||
			   elementName.getLocalName() == CatalogElementName)
		{
			AutoPtr<Attribute> rpPreferAttr = attributes.getAttribute(QC_T("prefer"));
			if(rpPreferAttr)
			{
				context.m_preference = rpPreferAttr->getValue();
			}

			AutoPtr<Attribute> rpBaseAttr = attributes.getAttribute(XMLNames::XMLNamespaceURI, QC_T("base"));
			if(rpBaseAttr)
			{
				context.m_base = rpBaseAttr->getValue();
			}
		}
		else
		{
			createCatalogEntry(elementName.getLocalName(), attributes, context);
		}
	}
	else
	{
		context.m_bIncluded = false;
	}

	context.m_elementName = elementName.getLocalName();
	m_contextStack.push(context);
}

void CatalogParserHandler::onEndElement(const QName& /*elementName*/)
{
	QC_DBG_ASSERT(m_contextStack.size());
	m_contextStack.pop();
}

//==============================================================================
// CatalogParserHandler::createCatalogEntry
//
// Called with an element from the Oasis Catalog namespace.  Our job is to 
// create an appropriate CatalogEntry.
//==============================================================================
void CatalogParserHandler::createCatalogEntry(const String& elementName,
                                             const AttributeSet& attributes,
                                             CatalogSerialisationContext& context)
{
	//
	// Only deal with child elements that are correctly parented.
	//
	if(context.m_elementName != GroupElementName &&
	   context.m_elementName != CatalogElementName)
	{
		return;
	}

	//
	// All catalog entries may contain an (optional) xml:base attribute
	//
	URL baseUri = context.m_base;
	const CatalogResolver& resolver = m_catalogFile.getEnclosingSet().getResolver();

	AutoPtr<Attribute> rpBaseAttr = attributes.getAttribute(XMLNames::XMLNamespaceURI, QC_T("base"));
	if(rpBaseAttr)
	{
		baseUri = URL(baseUri, rpBaseAttr->getValue());
	}

	CatalogResolver::Preference preference = CatalogResolver::GetGlobalPreference();
	if(context.m_preference == QC_T("public"))
	{
		preference = CatalogResolver::PUBLIC;
	}
	else if(context.m_preference == QC_T("system"))
	{
		preference = CatalogResolver::SYSTEM;
	}

	if(elementName == QC_T("public"))
	{
		const String& publicId = 
			StringUtils::NormalizeWhiteSpace(getRequiredAttribute(elementName, QC_T("publicId"), attributes));

		const String& uri = getRequiredAttribute(elementName, QC_T("uri"), attributes);

		m_catalogFile.addLeafEntry(
			new CatalogLeafEntry(publicId, CatalogEntry::PUBLIC, preference, URL(baseUri, uri).toExternalForm()));
	}
	else if(elementName == QC_T("system"))
	{
		const String& systemId = URLEncoder::RawEncode(getRequiredAttribute(elementName, QC_T("systemId"), attributes));
		const String& uri = getRequiredAttribute(elementName, QC_T("uri"), attributes);

		m_catalogFile.addLeafEntry(
			new CatalogLeafEntry(systemId, CatalogEntry::SYSTEM, preference, URL(baseUri, uri).toExternalForm()));
	}
	else if(elementName == QC_T("rewriteSystem"))
	{
		const String& startString = URLEncoder::RawEncode(getRequiredAttribute(elementName, QC_T("systemIdStartString"), attributes));
		const String& prefix = getRequiredAttribute(elementName, QC_T("rewritePrefix"), attributes);

		m_catalogFile.addLeafEntry(
			new CatalogLeafEntry(startString, CatalogEntry::REWRITE_SYSTEM, preference, URL(baseUri, prefix).toExternalForm()));
	}
	else if(elementName == QC_T("uri"))
	{
		const String& name = URLEncoder::RawEncode(getRequiredAttribute(elementName, QC_T("name"), attributes));
		const String& uri = getRequiredAttribute(elementName, QC_T("uri"), attributes);

		m_catalogFile.addLeafEntry(
			new CatalogLeafEntry(name, CatalogEntry::URI, preference, URL(baseUri, uri).toExternalForm()));
	}
	else if(elementName == QC_T("rewriteURI"))
	{
		const String& startString = URLEncoder::RawEncode(getRequiredAttribute(elementName, QC_T("uriStartString"), attributes));
		const String& prefix = getRequiredAttribute(elementName, QC_T("rewritePrefix"), attributes);

		m_catalogFile.addLeafEntry(
			new CatalogLeafEntry(startString, CatalogEntry::REWRITE_URI, preference, URL(baseUri, prefix).toExternalForm()));
	}
	else if(elementName == QC_T("delegatePublic"))
	{
		const String& publicIdStartString = 
			StringUtils::NormalizeWhiteSpace(getRequiredAttribute(elementName, QC_T("publicIdStartString"), attributes));

		const String& catalog = getRequiredAttribute(elementName, QC_T("catalog"), attributes);

		m_catalogFile.addDelegatorEntry(
			new CatalogDelegatorEntry(publicIdStartString, CatalogEntry::PUBLIC, preference, URL(baseUri, catalog), resolver));
	}
	else if(elementName == QC_T("delegateSystem"))
	{
		const String& systemIdStartString = URLEncoder::RawEncode(getRequiredAttribute(elementName, QC_T("systemIdStartString"), attributes));
		const String& catalog = getRequiredAttribute(elementName, QC_T("catalog"), attributes);

		m_catalogFile.addDelegatorEntry(
			new CatalogDelegatorEntry(systemIdStartString, CatalogEntry::SYSTEM, preference, URL(baseUri, catalog), resolver));
	}
	else if(elementName == QC_T("delegateURI"))
	{
		const String& uriStartString = URLEncoder::RawEncode(getRequiredAttribute(elementName, QC_T("uriStartString"), attributes));
		const String& catalog = getRequiredAttribute(elementName, QC_T("catalog"), attributes);

		m_catalogFile.addDelegatorEntry(
			new CatalogDelegatorEntry(uriStartString, CatalogEntry::URI, preference, URL(baseUri, catalog), resolver));
	}
	else if(elementName == QC_T("nextCatalog"))
	{
		const String& catalog = getRequiredAttribute(elementName, QC_T("catalog"), attributes);

		m_catalogFile.m_enclosingSet.addFile(URL(baseUri, catalog), &m_catalogFile);
	}
}

String CatalogParserHandler::getRequiredAttribute(const String& elementName,
                                                 const String& attrName,
                                                 const AttributeSet& attributes)
{
	AutoPtr<Attribute> rpAttr = attributes.getAttribute(attrName);
	if(rpAttr)
	{
		return rpAttr->getValue();
	}
	else
	{
		String errMsg = QC_T("Missing required attribute: '");
		errMsg += attrName;
		errMsg += QC_T("' from element '");
		errMsg += elementName;
		errMsg += QC_T("'");
		reportError(Fatal, errMsg, m_parser.getCurrentPosition());
		return String();
	}
}

//==============================================================================
// CatalogParserHandler::onError
//
// This is the error event handler that is registered for the parsing of the
// xml catalog files.  We are only really interested in Fatal errors.
//
// Errors *may* be reported to the application (if they have given us an
// error handler), but the severity will be downgraded to "Warning".
//==============================================================================
void CatalogParserHandler::onError(int level, long /*reserved*/,
                                   const String& message,
                                   const Position& position,
                                   const ContextString* /*pContextString*/)
{
	reportError(level, message, position);
}

void CatalogParserHandler::reportError(int level, const String& message, const Position& position)
{
	if(level >= Fatal)
	{
		CatalogEventHandler* pHandler = m_catalogFile.getEnclosingSet().getResolver().getEventHandler();
		if(pHandler)
		{
			// to avoid clumsy error messages containing full URLs, we extract the
			// filename from the system identifier
			URL sys(URL(QC_T("file:")), position.getSystemId());
			pHandler->onCatalogError(CatalogEventHandler::Error, 0, message,
			                         sys.getFile(),
			                         position.getLineNumber(), position.getColumnNumber());
		}
	}
}

void CatalogParserHandler::onComment(const CharType* /*pComment*/, size_t /*length*/) {}
void CatalogParserHandler::onCommentStart(bool /*bInsideDTD*/) {};
void CatalogParserHandler::onCommentEnd(bool /*bInsideDTD*/) {};
void CatalogParserHandler::onPI(const String& /*PITarget*/, const String& /*PIValue*/) {};
void CatalogParserHandler::onWhitespace(const CharType* /*pString*/, size_t /*length*/) {};
void CatalogParserHandler::onIgnorableWhitespace(const CharType* /*pString*/, size_t /*length*/) {};
void CatalogParserHandler::onCharData(const CharType* /*pString*/, size_t /*length*/) {};
void CatalogParserHandler::onStartCData() {};
void CatalogParserHandler::onEndCData() {};
void CatalogParserHandler::onNamespaceBegin(const String& /*prefix*/, const String& /*uri*/) {};
void CatalogParserHandler::onNamespaceChange(const String& /*prefix*/, const String& /*uriFrom*/, const String& /*uriTo*/, bool /*bRestoring*/) {};
void CatalogParserHandler::onNamespaceEnd(const String& /*prefix*/, const String& /*uri*/) {};
void CatalogParserHandler::onStartDocument(const String& /*systemId*/) {};
void CatalogParserHandler::onEndDocument() {};
void CatalogParserHandler::onXMLDeclaration(const String& /*version*/, const String& /*encoding*/, bool /*bStandalone*/) {};

QC_XMLCAT_NAMESPACE_END

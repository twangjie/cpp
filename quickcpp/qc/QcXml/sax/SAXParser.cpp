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

#include "SAXParser.h" 
#include "LocatorAdaptor.h"
#include "SAXFeatures.h"
#include "SAXNotRecognizedException.h"
#include "SAXNotSupportedException.h"
#include "SAXParseException.h"
#include "AttributeSetAdapter.h"

#include "QcCore/base/NullPointerException.h"
#include "QcXml/xml/ParserFactory.h"
#include "QcXml/xml/EntityType.h"
#include "QcXml/xml/Position.h"
#include "QcXml/xml/XMLFeatures.h"
#include "QcCore/net/URL.h"
#include "QcCore/io/IOException.h"
#include "QcCore/net/MalformedURLException.h"

QC_SAX_NAMESPACE_BEGIN 

using xml::ParserFactory;
using xml::EntityType;
using xml::XMLFeatures;
using net::URL;
using net::MalformedURLException;
using io::IOException;

SAXParser::SAXParser() : 
	m_bResolveDTDURIs(true),
	m_bResolveEntityURIs(true)
{
	m_rpParser = ParserFactory::CreateXMLParser();
	m_rpLocator = new LocatorAdaptor(m_rpParser.get(), m_bResolveEntityURIs);

	//todo - check we have enabled all the default features
	m_rpParser->enableFeature(XMLFeatures::NamespaceSupport, true);

	m_rpParser->setContentEventHandler(this);
	m_rpParser->setEntityEventHandler(this);
	m_rpParser->setEntityResolutionHandler(this);
	m_rpParser->setErrorEventHandler(this);
	m_rpParser->setDTDEventHandler(this);
}

//==============================================================================
// SAXParser::parse
//
//==============================================================================
void SAXParser::parse(const String& systemId)
{
	AutoPtr<InputSource> rpIS(new InputSource(systemId));
	parse(rpIS.get());
}

//==============================================================================
// SAXParser::parse
//
//==============================================================================
void SAXParser::parse(InputSource* pInputSource)
{
	if(!pInputSource) throw NullPointerException();

	try
	{
		m_rpParser->parseInputSource(pInputSource);
	}
	catch(IOException&)
	{
		throw;
	}
	catch(SAXException&)
	{
		throw;
	}
	catch(Exception& e)
	{
		throw SAXException(e.toString());
	}
}

//==============================================================================
// SAXParser::getContentHandler
//
//==============================================================================
AutoPtr<ContentHandler> SAXParser::getContentHandler() const
{
	return m_rpContentHandler;
}

//==============================================================================
// SAXParser::getDeclHandler
//
//==============================================================================
AutoPtr<DeclHandler> SAXParser::getDeclHandler() const
{
	return m_rpDeclHandler;
}

//==============================================================================
// SAXParser::getDTDHandler
//
//==============================================================================
AutoPtr<DTDHandler> SAXParser::getDTDHandler() const
{
	return m_rpDTDHandler;
}

//==============================================================================
// SAXParser::getEntityResolver
//
//==============================================================================
AutoPtr<EntityResolver> SAXParser::getEntityResolver() const
{
	return m_rpEntityResolver;
}

//==============================================================================
// SAXParser::getErrorHandler
//
//==============================================================================
AutoPtr<ErrorHandler> SAXParser::getErrorHandler() const
{
	return m_rpErrorHandler;
}

//==============================================================================
// SAXParser::getLexicalHandler
//
//==============================================================================
AutoPtr<LexicalHandler> SAXParser::getLexicalHandler() const
{
	return m_rpLexicalHandler;
}

//==============================================================================
// SAXParser::getFeature
//
//==============================================================================
bool SAXParser::getFeature(const String& name) const
{
	bool bRet = false;

	if(name == SAXFeatures::namespaces)
	{
		bRet = m_rpParser->isFeatureEnabled(XMLFeatures::NamespaceSupport);
	}
	else if(name == SAXFeatures::namespace_prefixes)
	{
		bRet = m_rpParser->isFeatureEnabled(XMLFeatures::ReportNamespaceDeclarations);
	}
	else if(name == SAXFeatures::interoperability)
	{
		bRet = m_rpParser->isFeatureEnabled(XMLFeatures::InteroperabilityTests);
	}
	else if(name == SAXFeatures::warnings)
	{
		bRet = m_rpParser->isFeatureEnabled(XMLFeatures::WarningTests);
	}
	else if(name == SAXFeatures::validation)
	{
		bRet = m_rpParser->isFeatureEnabled(XMLFeatures::Validate);
	}
	else if(name == SAXFeatures::external_general_entities)
	{
		bRet = m_rpParser->isFeatureEnabled(XMLFeatures::ReadExternalGeneralEntities);
	}
	else if(name == SAXFeatures::external_parameter_entities)
	{
		bRet = m_rpParser->isFeatureEnabled(XMLFeatures::ReadExternalParameterEntities);
	}
	else if(name == SAXFeatures::lexical_handler_parameter_entities)
	{
		bRet = m_rpParser->isFeatureEnabled(XMLFeatures::ReportPEBoundaries);
	}
	else if(name == SAXFeatures::resolve_entity_uris)
	{
		bRet = m_bResolveEntityURIs;
	}
	else if(name == SAXFeatures::resolve_dtd_uris)
	{
		bRet = m_bResolveDTDURIs;
	}
	else if(name == SAXFeatures::validate_namespace_declarations)
	{
		bRet = m_rpParser->isFeatureEnabled(XMLFeatures::ValidateNamespaceDeclarations);
	}
	else if(name == SAXFeatures::enable_relative_namespace_uri_test)
	{
		bRet = m_rpParser->isFeatureEnabled(XMLFeatures::RelativeNamespaceURITest);
	}
	else if(name == SAXFeatures::is_standalone)
	{
		if(m_rpParser->isParseInProgress())
		{
			bRet = m_rpParser->isStandaloneDocument();
		}
		else
		{
			throw SAXNotSupportedException(name + QC_T(" is not available at this time"));
		}
	}
	else
	{
		throw SAXNotRecognizedException(name);
	}

	return bRet;
}

//==============================================================================
// SAXParser::setContentHandler
//
// As soon as we are given a ContentHandler, we respond with a call to
// ContentHandler::setDocumentLocator().  We cannot delay this until
// StartDocument() because (a) it would be inefficient to call for each document
// and (b), there is no guarantee that this ContentHandler was available when
// the startDocument event occurred.
//==============================================================================
void SAXParser::setContentHandler(ContentHandler* pHandler)
{
	m_rpContentHandler = pHandler;
	if(m_rpContentHandler && m_rpParser)
	{
		m_rpContentHandler->setDocumentLocator(m_rpLocator.get());
	}
}

//==============================================================================
// SAXParser::setDeclHandler
//
//==============================================================================
void SAXParser::setDeclHandler(DeclHandler* pHandler)
{
	m_rpDeclHandler = pHandler;
}

//==============================================================================
// SAXParser::setDTDHandler
//
//==============================================================================
void SAXParser::setDTDHandler(DTDHandler* pHandler)
{
	m_rpDTDHandler = pHandler;
}

//==============================================================================
// SAXParser::setEntityResolver
//
//==============================================================================
void SAXParser::setEntityResolver(EntityResolver* pResolver)
{
	m_rpEntityResolver = pResolver;
}

//==============================================================================
// SAXParser::setErrorHandler
//
//==============================================================================
void SAXParser::setErrorHandler(ErrorHandler* pHandler)
{
	m_rpErrorHandler = pHandler;
}

//==============================================================================
// SAXParser::setLexicalHandler
//
//==============================================================================
void SAXParser::setLexicalHandler(LexicalHandler* pHandler)
{
	m_rpLexicalHandler = pHandler;

	//
	// If we have a LexicalHandler, this implies that we must report
	// document and DTD comments
	//
	bool bSetFeature = (pHandler != 0);
	m_rpParser->enableFeature(XMLFeatures::ReportDocumentComments, bSetFeature);
	m_rpParser->enableFeature(XMLFeatures::ReportDTDComments, bSetFeature);
}

//==============================================================================
// SAXParser::setFeature
//
//==============================================================================
void SAXParser::setFeature(const String& name, bool bSet)
{
	//
	// All sax.org features are read-only during a parse
	//
	if(m_rpParser->isParseInProgress())
	{
		const String sax_org = QC_T("http://xml.org/sax/features/");
		if(name.length() > sax_org.length() && name.substr(0, sax_org.length()) == sax_org)
		{
			throw SAXNotSupportedException(name + QC_T(" is read-only at this time"));
		}
	}

	if(name == SAXFeatures::namespaces)
	{
		m_rpParser->enableFeature(XMLFeatures::NamespaceSupport, bSet);
	}
	else if(name == SAXFeatures::namespace_prefixes)
	{
		m_rpParser->enableFeature(XMLFeatures::ReportNamespaceDeclarations, bSet);
	}
	else if(name == SAXFeatures::interoperability)
	{
		m_rpParser->enableFeature(XMLFeatures::InteroperabilityTests, bSet);
	}
	else if(name == SAXFeatures::warnings)
	{
		m_rpParser->enableFeature(XMLFeatures::WarningTests, bSet);
	}
	else if(name == SAXFeatures::validation)
	{
		m_rpParser->enableFeature(XMLFeatures::Validate, bSet);
	}
	else if(name == SAXFeatures::resolve_dtd_uris)
	{
		m_bResolveDTDURIs = bSet;
	}
	else if(name == SAXFeatures::resolve_entity_uris)
	{
		m_bResolveEntityURIs = bSet;
		if(m_rpLocator)
		{
			m_rpLocator->setResolveSystemIds(bSet);
		}
	}
	else if(name == SAXFeatures::external_general_entities)
	{
		m_rpParser->enableFeature(XMLFeatures::ReadExternalGeneralEntities, bSet);
	}
	else if(name == SAXFeatures::external_parameter_entities)
	{
		m_rpParser->enableFeature(XMLFeatures::ReadExternalParameterEntities, bSet);
	}
	else if(name == SAXFeatures::lexical_handler_parameter_entities)
	{
		//
		// This feature requires the setting of a special feature, which makes the
		// XML parser non-conformant (badly nested PEs will be reported as errors)
		//
		m_rpParser->enableFeature(XMLFeatures::ReportPEBoundaries, bSet);
	}
	else if(name == SAXFeatures::validate_namespace_declarations)
	{
		m_rpParser->enableFeature(XMLFeatures::ValidateNamespaceDeclarations, bSet);
	}
	else if(name == SAXFeatures::enable_relative_namespace_uri_test)
	{
		m_rpParser->enableFeature(XMLFeatures::RelativeNamespaceURITest, bSet);
	}
	else if(name == SAXFeatures::is_standalone)
	{
		throw SAXNotSupportedException(name + QC_T(" is a read-only feature"));
	}
	else
	{
		throw SAXNotRecognizedException(name);
	}
}

//==============================================================================
// SAXParser::onComment
//
//==============================================================================
void SAXParser::onComment(const CharType* pComment, size_t length)
{
	if(m_rpLexicalHandler)
	{
		m_commentBuffer.append(pComment, length);
	}
}

//==============================================================================
// SAXParser::onCommentStart
//
//==============================================================================
void SAXParser::onCommentStart(bool /*bInsideDTD*/)
{
	m_commentBuffer.clear();
}

//==============================================================================
// SAXParser::onCommentEnd
//
//==============================================================================
void SAXParser::onCommentEnd(bool /*bInsideDTD*/)
{
	if(m_rpLexicalHandler && m_commentBuffer.size())
	{
		m_rpLexicalHandler->comment(m_commentBuffer.data(), m_commentBuffer.size());
	}
}

//==============================================================================
// SAXParser::onStartElement
//
//==============================================================================
void SAXParser::onStartElement(const QName& elementName, bool /*bEmptyElement*/, const AttributeSet& attributes)
{
	if(m_rpContentHandler)
	{
		AttributeSetAdapter saxAttributes(attributes);
		m_rpContentHandler->startElement(elementName.getNamespaceURI(),
		                                 elementName.getLocalName(),
		                                 elementName.getRawName(),
		                                 saxAttributes);
	}
}

//==============================================================================
// SAXParser::onEndElement
//
//==============================================================================
void SAXParser::onEndElement(const QName& elementName)
{
	if(m_rpContentHandler)
	{
		m_rpContentHandler->endElement(elementName.getNamespaceURI(),
		                               elementName.getLocalName(),
									   elementName.getRawName());
	}
}

//==============================================================================
// SAXParser::onError
//
// Translate the XML Parser error event into a SAX ErrorHandler event.
//
// Note that in the absence of a registered ErrorHandler, a SAXParseException
// is thrown for Fatal errors.
//==============================================================================
void SAXParser::onError(int level, long /*reserved*/, const String& error, const Position& position, const ContextString* /*pContext*/)
{
	if(m_rpErrorHandler || (level==ErrorEventHandler::Fatal))
	{
		SAXParseException exception(error,
		                            position.getPublicId(),
		                            position.getSystemId(),
									position.getLineNumber(),
		                            position.getColumnNumber());

		if(level == ErrorEventHandler::Warning)
		{
			m_rpErrorHandler->warning(exception);
		}
		else if(level == ErrorEventHandler::Error)
		{
			m_rpErrorHandler->error(exception);
		}
		else if(m_rpErrorHandler)
		{
			m_rpErrorHandler->fatalError(exception);
		}
		else
		{
			throw exception;
		}
	}
}

//==============================================================================
// SAXParser::onPI
//
//==============================================================================
void SAXParser::onPI(const String& PITarget, const String& PIValue)
{
	if(m_rpContentHandler)
	{
		m_rpContentHandler->processingInstruction(PITarget, PIValue);
	}
}

//==============================================================================
// SAXParser::onWhitespace
//
// This is called when white-space is encountered outside of any mark-up
// (ie outside the document element)
//==============================================================================
void SAXParser::onWhitespace(const CharType* pStr, size_t length)
{
	if(m_rpContentHandler)
	{
		m_rpContentHandler->characters(pStr, length);
	}
}

//==============================================================================
// SAXParser::onIgnorableWhitespace
//
// This is called when white-space is encountered within elements that have
// pure element content (ie they cannot contain character data - so white-space
// cannot be used for any reason except prettifyingthe document)
//==============================================================================
void SAXParser::onIgnorableWhitespace(const CharType* pStr, size_t length)
{
	if(m_rpContentHandler)
	{
		m_rpContentHandler->ignorableWhitespace(pStr, length);
	}
}

//==============================================================================
// SAXParser::onCharData
//
//==============================================================================
void SAXParser::onCharData(const CharType* pStart, size_t length)
{
	if(m_rpContentHandler)
	{
		m_rpContentHandler->characters(pStart, length);
	}
}

//==============================================================================
// SAXParser::onResolveEntity
//
//==============================================================================
AutoPtr<XMLInputSource> SAXParser::onResolveEntity(const String& /*name*/,
                                                  EntityType /*type*/,
                                                  const String& publicId,
                                                  const String& systemId,
                                                  const String& /*baseUri*/)
{
	if(m_rpEntityResolver)
	{
		return m_rpEntityResolver->
			resolveEntity(publicId, resolveSystemId(systemId, false)).get();
	}
	else
	{
		return 0;
	}
}

//==============================================================================
// SAXParser::onUnexpandedEntity
//
//==============================================================================
void SAXParser::onUnexpandedEntity(const String& /*name*/,
	                               EntityType /*type*/,
	                               const String& /*publicId*/,
	                               const String& /*systemId*/,
	                               const String& /*baseUri*/)
{
}

//==============================================================================
// SAXParser::onNamespaceBegin
//
//==============================================================================
void SAXParser::onNamespaceBegin(const String& prefix, const String& uri)
{
	if(m_rpContentHandler)
	{
		m_rpContentHandler->startPrefixMapping(prefix, uri);
	}
}

//==============================================================================
// SAXParser::onNamespaceChange
//
//==============================================================================
void SAXParser::onNamespaceChange(const String& prefix, const String& /*uriFrom*/, const String& uriTo, bool bRestoring)
{
	if(m_rpContentHandler)
	{
		if(bRestoring)
		{
			m_rpContentHandler->endPrefixMapping(prefix);
		}
		else
		{
			m_rpContentHandler->startPrefixMapping(prefix, uriTo);
		}
	}
}


//==============================================================================
// SAXParser::onNamespaceEnd
//
//==============================================================================
void SAXParser::onNamespaceEnd(const String& prefix, const String& /*uri*/)
{
	if(m_rpContentHandler)
	{
		m_rpContentHandler->endPrefixMapping(prefix);
	}
}

//==============================================================================
// SAXParser::onNotationDecl
//
//==============================================================================
void SAXParser::onNotationDecl(const String& name, const String& publicId, const String& systemId)
{
	if(m_rpDTDHandler)
	{
		m_rpDTDHandler->notationDecl(name, publicId, resolveSystemId(systemId, true));
	}
}

//==============================================================================
// SAXParser::onUnparsedEntityDecl
//
//==============================================================================
void SAXParser::onUnparsedEntityDecl(const String& name, const String& publicId, const String& systemId, const String& notationName)
{
	if(m_rpDTDHandler)
	{
		m_rpDTDHandler->unparsedEntityDecl(name, publicId, resolveSystemId(systemId, true), notationName);
	}
}

//==============================================================================
// SAXParser::onExternalEntityDecl
//
//==============================================================================
void SAXParser::onExternalEntityDecl(const String& name, EntityType type, const String& publicId, const String& systemId)
{
	if(m_rpDeclHandler)
	{
		const String& entityName = (type == EntityType::Parameter)
		                         ? String(QC_T("%")) + name
		                         : name;
			
		m_rpDeclHandler->externalEntityDecl(entityName, publicId, resolveSystemId(systemId, true));
	}
}

//==============================================================================
// SAXParser::onInternalEntityDecl
//
//==============================================================================
void SAXParser::onInternalEntityDecl(const String& name, EntityType type, const String& value)
{
	if(m_rpDeclHandler)
	{
		const String& entityName = (type == EntityType::Parameter)
		                         ? String(QC_T("%")) + name
		                         : name;
			
		m_rpDeclHandler->internalEntityDecl(entityName, value);
	}
}

//==============================================================================
// SAXParser::onAttributeDecl
//
// From SAX 2.0-ext documentation...
//
// Only the effective (first) declaration for an attribute will be reported.
// The type will be one of the strings "CDATA", "ID", "IDREF", "IDREFS", 
// "NMTOKEN", "NMTOKENS", "ENTITY", "ENTITIES", a parenthesized 
// token group with the separator "|" and all white-space removed, or the 
// word "NOTATION" followed by a space followed by a parenthesized token 
// group with all white-space removed.
//
//==============================================================================
void SAXParser::onAttributeDecl(const String& eName,
                                const String& aName,
                                const String& type,
                                const String& valueDefault,
                                const String& enumGroup,
                                const String& value)
{
	if(m_rpDeclHandler)
	{
		if(type == QC_T("NOTATION"))
		{
			String saxType = QC_T("NOTATION ") + enumGroup;
			m_rpDeclHandler->attributeDecl(eName, aName, saxType, valueDefault, value);

		}
		else
		{
			const String saxType = enumGroup.size() ? enumGroup : type;
			m_rpDeclHandler->attributeDecl(eName, aName, saxType, valueDefault, value);
		}
	}
}

//==============================================================================
// SAXParser::onElementDecl
//
//==============================================================================
void SAXParser::onElementDecl(const String& name, const String& model)
{
	if(m_rpDeclHandler)
	{
		m_rpDeclHandler->elementDecl(name, model);
	}
}

//==============================================================================
// SAXParser::onStartDocument
//
//==============================================================================
void SAXParser::onStartDocument(const String& /*systemId*/)
{
	if(m_rpContentHandler)
	{
		m_rpContentHandler->startDocument();
	}
}

//==============================================================================
// SAXParser::onEndDocument
//
//==============================================================================
void SAXParser::onEndDocument()
{
	if(m_rpContentHandler)
	{
		m_rpContentHandler->endDocument();
	}
}

//==============================================================================
// SAXParser::onStartDTD
//
//==============================================================================
void SAXParser::onStartDTD(const String& name, const String& publicId, const String& systemId)
{
	if(m_rpLexicalHandler)
	{
		m_rpLexicalHandler->startDTD(name, publicId, systemId);
	}
}

//==============================================================================
// SAXParser::onEndDTD
//
//==============================================================================
void SAXParser::onEndDTD()
{
	if(m_rpLexicalHandler)
	{
		m_rpLexicalHandler->endDTD();
	}
}

//==============================================================================
// SAXParser::onStartEntity
//
//==============================================================================
void SAXParser::onStartEntity(const String& name, EntityType type)
{
	if(m_rpLexicalHandler)
	{
		String entityName = name;
		
		if(type == EntityType::Parameter)
		{
			entityName = String(QC_T("%")) + name;
		}
		else if(type == EntityType::DTD)
		{
			entityName = QC_T("[dtd]");
		}
		else
		{
			entityName = name;
		}
			
		m_rpLexicalHandler->startEntity(entityName);
	}
}

//==============================================================================
// SAXParser::onEndEntity
//
//==============================================================================
void SAXParser::onEndEntity(const String& name, EntityType type)
{
	if(m_rpLexicalHandler)
	{
		const String& entityName = (type == EntityType::Parameter)
		                         ? String(QC_T("%")) + name
		                         : name;
			
		m_rpLexicalHandler->endEntity(entityName);
	}
}

//==============================================================================
// SAXParser::onStartCData
//
//==============================================================================
void SAXParser::onStartCData()
{
	if(m_rpLexicalHandler)
	{
		m_rpLexicalHandler->startCDATA();
	}
}

//==============================================================================
// SAXParser::onEndCData
//
//==============================================================================
void SAXParser::onEndCData()
{
	if(m_rpLexicalHandler)
	{
		m_rpLexicalHandler->endCDATA();
	}
}

//==============================================================================
// SAXParser::onXMLDeclaration
//
// There is no SAX interface that takes this!
//==============================================================================
void SAXParser::onXMLDeclaration(const String& /*version*/, const String& /*encoding*/, bool /*bStandalone*/)
{
}

//==============================================================================
// SAXParser::onStartAttlist
//
//==============================================================================
void SAXParser::onStartAttlist(const String& /*eName*/)
{
}

//==============================================================================
// SAXParser::onEndAttlist
//
//==============================================================================
void SAXParser::onEndAttlist()
{
}

//==============================================================================
// SAXParser::resolveSystemID
//
// Private helper function
//==============================================================================
String SAXParser::resolveSystemId(const String& id, bool bDTDDecl) const
{
	if((bDTDDecl && m_bResolveDTDURIs) || (!bDTDDecl && m_bResolveEntityURIs))
	{
		try
		{
			return URL(m_rpParser->getCurrentBaseURI(), id).toExternalForm();
		}
		catch(MalformedURLException& /*e*/)
		{
			return id;
		}
	}
	else
	{
		return id;
	}
}

QC_SAX_NAMESPACE_END


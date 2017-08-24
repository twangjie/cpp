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
// Class: SAXParser
// 
// Overview
// --------
// Implementation of XMLReader using the QC XML Parser.
//
//=============================================================================

#ifndef QC_SAX_SAXParser_h
#define QC_SAX_SAXParser_h

#include "defs.h"
#include "XMLReader.h"
#include "ContentHandler.h"
#include "DTDHandler.h"
#include "EntityResolver.h"
#include "LexicalHandler.h"
#include "LocatorAdaptor.h"

#include "QcCore/base/AutoBuffer.h"
#include "QcXml/xml/Parser.h"
#include "QcXml/xml/ContentEventHandler.h"
#include "QcXml/xml/DTDEventHandler.h"
#include "QcXml/xml/EntityEventHandler.h"
#include "QcXml/xml/EntityResolutionHandler.h"
#include "QcXml/xml/ErrorEventHandler.h"

QC_SAX_NAMESPACE_BEGIN

using xml::Parser;
using xml::ContentEventHandler;
using xml::DTDEventHandler;
using xml::EntityEventHandler;
using xml::EntityResolutionHandler;
using xml::ErrorEventHandler;
using xml::Position;
using xml::ContextString;
using xml::BufferRange;
using xml::EntityType;
using xml::QName;
using xml::AttributeSet;

class QC_SAX_PKG SAXParser : public XMLReader,
                              public ContentEventHandler,
                              public DTDEventHandler,
                              public ErrorEventHandler,
                              public EntityEventHandler,
                              public EntityResolutionHandler
{
public: 
	SAXParser();

public: // sax::XMLReader overrides...
	virtual void parse(const String& systemId);
	virtual void parse(InputSource* pInputSource);

	virtual AutoPtr<ContentHandler> getContentHandler() const;
	virtual AutoPtr<DeclHandler> getDeclHandler() const;
	virtual AutoPtr<DTDHandler> getDTDHandler() const;
	virtual AutoPtr<EntityResolver> getEntityResolver() const;
	virtual AutoPtr<ErrorHandler> getErrorHandler() const;
	virtual AutoPtr<LexicalHandler> getLexicalHandler() const;
	virtual bool getFeature(const String& name) const;

	virtual void setContentHandler(ContentHandler* pHandler);
	virtual void setDeclHandler(DeclHandler* pHandler);
	virtual void setDTDHandler(DTDHandler* pHandler);
	virtual void setEntityResolver(EntityResolver* pResolver);
	virtual void setErrorHandler(ErrorHandler* pHandler);
	virtual void setLexicalHandler(LexicalHandler* pHandler);
	virtual void setFeature(const String& name, bool bSet);

protected: 

	// ContentEventHandler methods
	virtual void onComment(const CharType* pComment, size_t length);
	virtual void onCommentStart(bool bInsideDTD);
	virtual void onCommentEnd(bool bInsideDTD);
	virtual void onPI(const String& PITarget, const String& PIValue);
	virtual void onStartElement(const QName& elementName, bool bEmptyElement, const AttributeSet& attributes);
	virtual void onEndElement(const QName& elementName);
	virtual void onWhitespace(const CharType* pChars, size_t length);
	virtual void onIgnorableWhitespace(const CharType* pChars, size_t length);
	virtual void onCharData(const CharType* pChars, size_t length);
	virtual void onNamespaceBegin(const String& prefix, const String& uri);
	virtual void onNamespaceChange(const String& prefix, const String& uriFrom, const String& uriTo, bool bRestoring);
	virtual void onNamespaceEnd(const String& prefix, const String& uri);
	virtual void onStartCData();
	virtual void onEndCData();
	virtual void onStartDocument(const String& systemId);
	virtual void onEndDocument();
	virtual void onXMLDeclaration(const String& version, const String& encoding, bool bStandalone);

	// ErrorEventHandler methods
	virtual void onError(int level, long reserved, const String& error, const Position& position, const ContextString* pContext);

	// DTDEventHandler methods
	virtual void onStartDTD(const String& name, const String& publicId, const String& systemId);
	virtual void onEndDTD();
	virtual void onNotationDecl(const String& name, const String& publicId, const String& systemId);
	virtual void onUnparsedEntityDecl(const String& name, const String& publicId, const String& systemId, const String& notationName);
	virtual void onExternalEntityDecl(const String& name, EntityType type, const String& publicId, const String& systemId);
	virtual void onInternalEntityDecl(const String& name, EntityType type, const String& value);
	virtual void onStartAttlist(const String& eName);
	virtual void onEndAttlist();
	virtual void onAttributeDecl(const String& eName, const String& aName, const String& type, const String& valueDefault, const String& enumGroup, const String& value);
	virtual void onElementDecl(const String& name, const String& model);

	// EntityResolver methods
	virtual AutoPtr<XMLInputSource> onResolveEntity(const String& name,
	                                               EntityType type,
	                                               const String& publicId,
	                                               const String& systemId,
	                                               const String& baseUri);
	
	// EntityEventHandler methods
	virtual void onUnexpandedEntity(const String& name,
	                                EntityType type,
	                                const String& publicId,
	                                const String& systemId,
	                                const String& baseUri);

	virtual void onStartEntity(const String& name,
	                           EntityType type);

	virtual void onEndEntity(const String& name,
	                         EntityType type);

private:
	String resolveSystemId(const String& id, bool bDTDDecl) const;

private:
	AutoPtr<ContentHandler> m_rpContentHandler;
	AutoPtr<DeclHandler> m_rpDeclHandler;
	AutoPtr<DTDHandler> m_rpDTDHandler;
	AutoPtr<EntityResolver> m_rpEntityResolver;
	AutoPtr<ErrorHandler> m_rpErrorHandler;
	AutoPtr<LexicalHandler> m_rpLexicalHandler;
	AutoPtr<Parser> m_rpParser;
	AutoPtr<LocatorAdaptor> m_rpLocator;
	AutoBuffer<CharType> m_commentBuffer;
	bool m_bResolveDTDURIs;
	bool m_bResolveEntityURIs;
};

QC_SAX_NAMESPACE_END

#endif //QC_SAX_SAXParser_h


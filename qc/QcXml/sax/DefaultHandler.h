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

#ifndef QC_SAX_DefaultHandler_h
#define QC_SAX_DefaultHandler_h

#include "ContentHandler.h"
#include "DTDHandler.h"
#include "EntityResolver.h"
#include "ErrorHandler.h"

QC_SAX_NAMESPACE_BEGIN

class QC_SAX_PKG DefaultHandler : public ContentHandler,
                                   public DTDHandler,
                                   public EntityResolver,
                                   public ErrorHandler
{
public:

	// ContentHandler methods...
	virtual void characters(const CharType* pStart, size_t length);
	virtual void endElement(const String& namespaceURI,
	                        const String& localName,
	                        const String& qName);
	virtual void endPrefixMapping(const String& prefix);
	virtual void ignorableWhitespace(const CharType* pStart, size_t length);
	virtual void processingInstruction(const String& target, const String& data);
	virtual void startElement(const String& namespaceURI,
	                          const String& localName,
	                          const String& qName,
	                          const Attributes& atts);
	virtual void startPrefixMapping(const String& prefix, const String& uri);
	virtual void startDocument();
	virtual void skippedEntity(const String& name);
	virtual void endDocument();
	virtual void setDocumentLocator(Locator* pLocator);

	// DTDHandler methods...
	virtual void notationDecl(const String& name, const String& publicId, const String& systemId);
	virtual void unparsedEntityDecl(const String& name, const String& publicId, const String& systemId, const String& notationName);


	// ErrorHandler methods...
	virtual void error(const SAXParseException& exception);
	virtual void fatalError(const SAXParseException& exception);
	virtual void warning(const SAXParseException& exception);
	
	// EntityResolver methods...
	virtual AutoPtr<InputSource> resolveEntity(const String& publicId,
	                                          const String& systemId);
};

QC_SAX_NAMESPACE_END

#endif // QC_SAX_DefaultHandler_h

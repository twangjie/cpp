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

#ifndef QC_SAX_ContentHandler_h
#define QC_SAX_ContentHandler_h

#include "defs.h"
#include "Attributes.h"

QC_SAX_NAMESPACE_BEGIN 

class Locator;

class QC_SAX_PKG ContentHandler : public virtual QCObject
{
public:

	virtual void characters(const CharType* pStart, size_t length) = 0;
	virtual void endDocument() = 0;

	virtual void endElement(const String& namespaceURI,
	                        const String& localName,
	                        const String& qName) = 0;

	virtual void endPrefixMapping(const String& prefix) = 0;
	virtual void ignorableWhitespace(const CharType* pStart, size_t length) = 0;
	virtual void processingInstruction(const String& target, const String& data) = 0;
	virtual void setDocumentLocator(Locator* pLocator) = 0;
	virtual void skippedEntity(const String& name) = 0;
	virtual void startDocument() = 0;

	virtual void startElement(const String& namespaceURI,
	                          const String& localName,
	                          const String& qName,
	                          const Attributes& atts) = 0;

	virtual void startPrefixMapping(const String& prefix, const String& uri) = 0;
};

QC_SAX_NAMESPACE_END

#endif //QC_SAX_ContentHandler_h

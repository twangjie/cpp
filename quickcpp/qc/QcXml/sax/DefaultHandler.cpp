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
// Class DefaultHandler
/**
	@class qc::sax::DefaultHandler
	
	@brief Default base class for SAX2 event handlers.

	<p>This class is available as a convenience base class for SAX2
	applications: it provides default implementations for all of the
	callbacks in the four core SAX2 handler classes:</p>

	- EntityResolver
	- DTDHandler
	- ContentHandler
	- ErrorHandler

	<p>Application writers can extend this class when they need to
	implement only part of an interface; parser writers can
	instantiate this class to provide default handlers when the
	application has not supplied its own.</p>
*/
//==============================================================================

#include "DefaultHandler.h"
#include "SAXParseException.h"

QC_SAX_NAMESPACE_BEGIN

//==============================================================================
// DefaultHandler::startDocument
//
//==============================================================================
void DefaultHandler::startDocument()
{
}

//==============================================================================
// DefaultHandler::endDocument
//
//==============================================================================
void DefaultHandler::endDocument()
{
}

//==============================================================================
// DefaultHandler::characters
//
//==============================================================================
void DefaultHandler::characters(const CharType* /*pChars*/, size_t /*length*/)
{
}

//==============================================================================
// DefaultHandler::ignorableWhitespace
//
//==============================================================================
void DefaultHandler::ignorableWhitespace(const CharType* /*pStart*/, size_t /*length*/)
{
}

//==============================================================================
// DefaultHandler::processingInstruction
//
//==============================================================================
void DefaultHandler::processingInstruction(const String& /*target*/, const String& /*data*/)
{
}

//==============================================================================
// DefaultHandler::startElement
//
//==============================================================================
void DefaultHandler::startElement(const String& /*namespaceURI*/,
                                  const String& /*localName*/,
                                  const String& /*qName*/,
                                  const Attributes& /*atts*/)
{
}

//==============================================================================
// DefaultHandler::endElement
//
//==============================================================================
void DefaultHandler::endElement(const String& /*namespaceURI*/,
                                const String& /*localName*/,
                                const String& /*qName*/)
{
}

//==============================================================================
// DefaultHandler::startPrefixMapping
//
//==============================================================================
void DefaultHandler::startPrefixMapping(const String& /*prefix*/, const String& /*uri*/)
{
}

//==============================================================================
// DefaultHandler::endPrefixMapping
//
//==============================================================================
void DefaultHandler::endPrefixMapping(const String& /*prefix*/)
{
}

//==============================================================================
// DefaultHandler::setDocumentLocator
//
//==============================================================================
void DefaultHandler::setDocumentLocator(Locator* /*pLocator*/)
{
}

//==============================================================================
// DefaultHandler::skippedEntity
//
//==============================================================================
void DefaultHandler::skippedEntity(const String& /*name*/)
{
}

// DT
//==============================================================================
// DefaultHandler::notationDecl
//
//==============================================================================
void DefaultHandler::notationDecl(const String& /*name*/, const String& /*publicId*/, const String& /*systemId*/)
{
}

//==============================================================================
// DefaultHandler::unparsedEntityDecl
//
//==============================================================================
void DefaultHandler::unparsedEntityDecl(const String& /*name*/, const String& /*publicId*/, const String& /*systemId*/, const String& /*notationName*/)
{
}

//==============================================================================
// DefaultHandler::error
//
//==============================================================================
void DefaultHandler::error(const SAXParseException& /*exception*/)
{
}

//==============================================================================
// DefaultHandler::fatalError
//
/**
	Receive notification of a non-recoverable error.

	<p>This corresponds to the definition of "fatal error" in
	section 1.2 of the W3C XML 1.0 Recommendation.  For example, a
	parser would use this callback to report the violation of a
	well-formedness constraint.</p>

	<p>The application must assume that the document is unusable
	after the parser has invoked this method, and should continue
	(if at all) only for the sake of collecting additional error
	messages.</p>

    <p>The default implementation throws the @c exception
	   parameter as an exception.</p>

	@param exception The error information encapsulated in a
					 SAXParseException.  
	@throws SAXException The application may throw an exception during processing.
            If so, the SAX XML parser will stop parsing the current document.
*/
//==============================================================================
void DefaultHandler::fatalError(const SAXParseException& exception)
{
	throw exception;
}

//==============================================================================
// DefaultHandler::warning
//
//==============================================================================
void DefaultHandler::warning(const SAXParseException& /*exception*/)
{
}

//==============================================================================
// DefaultHandler::resolveEntity
//
//==============================================================================
AutoPtr<InputSource> DefaultHandler::resolveEntity(const String& /*publicId*/,
	                                              const String& /*systemId*/)
{
	return 0;
}

QC_SAX_NAMESPACE_END

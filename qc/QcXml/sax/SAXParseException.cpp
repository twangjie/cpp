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
// Class: SAXParseException
// 
/**
	@class qc::sax::SAXParseException
	
	@brief Encapsulate an XML parse error or warning.
	       
	<p>This exception may include information for locating the error in
	the original XML document, as if it came from a Locator object.
	Note that although the application will receive a SAXParseException 
	as the argument to the handlers in the ErrorHandler interface,
	the application is not actually required to throw the exception; instead,
	it can simply read the information in it and take a different action.</p>
*/
//==============================================================================

#include "SAXParseException.h"
#include "Locator.h"

QC_SAX_NAMESPACE_BEGIN

//==============================================================================
// SAXParseException::SAXParseException
//
/**
   Create a new SAXParseException from a message and a Locator.
   The information from the Locator is copied into internal state variables.
   
   @param message The error or warning message.
   @param locator A Locator object giving the location within the document
          where the error occurred.  
*/
//==============================================================================
SAXParseException::SAXParseException(const String& message, const Locator& locator) :
	SAXException(message),
	m_publicId(locator.getPublicId()),
	m_systemId(locator.getSystemId()),
	m_columnNumber(locator.getColumnNumber()),
	m_lineNumber(locator.getLineNumber())
{
}
	
//==============================================================================
// SAXParseException::SAXParseException
//
/**
   Create a new SAXParseException.

   All parameters except the message are as if they were provided by a Locator.

   @param message The error or warning message.
   @param publicId The public identifier of the entity that generated the error or warning.
   @param systemId The system identifier of the entity that generated the error or warning.
   @param lineNumber The line number of the end of the text that caused the error or warning.
   @param columnNumber The column number of the end of the text that cause the error or warning
*/
//==============================================================================
SAXParseException::SAXParseException(const String& message,
                                     const String& publicId,
                                     const String& systemId,
                                     long lineNumber,
                                     long columnNumber) :
	SAXException(message),
	m_publicId(publicId),
	m_systemId(systemId),
	m_columnNumber(columnNumber),
	m_lineNumber(lineNumber)
{
}

//==============================================================================
// SAXParseException::getColumnNumber
//
/**
	Returns the column number of the end of the text where the exception occurred. 
    <p>The first column in a line is position 1.</p>

	@returns An integer representing the column number, or -1 if none is available.
	@sa Locator::getColumnNumber()
*/
//==============================================================================
long SAXParseException::getColumnNumber() const
{
	return m_columnNumber;
}

//==============================================================================
// SAXParseException::getLineNumber
//
/**
	Returns the line number of the end of the text where the exception occurred. 

    <p>The first line is line 1.</p>

	@returns An integer representing the line number, or -1 if none is available.
	@sa Locator::getLineNumber()
*/
//==============================================================================
long SAXParseException::getLineNumber() const
{
	return m_lineNumber;
}

//==============================================================================
// SAXParseException::getPublicId
//
/**
   Get the public identifier of the entity where the exception occurred.

   @returns A string containing the public identifier, or the empty string
            if none is available.
   @sa Locator::getPublicId()
*/
//==============================================================================
String SAXParseException::getPublicId() const
{
	return m_publicId;
}

//==============================================================================
// SAXParseException::getSystemId
//
/**
   Get the system identifier of the entity where the exception occurred.

   @returns A string containing the system identifier, or the empty string
            if none is available.
   @sa Locator::getSystemId()
*/
//==============================================================================
String SAXParseException::getSystemId() const
{
	return m_systemId;
}

QC_SAX_NAMESPACE_END

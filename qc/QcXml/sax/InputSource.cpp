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
// Class InputSource
/**
	@class qc::sax::InputSource
	
	@brief A single input source for an XML entity.

	<p>This class allows a SAX application to encapsulate information
	about an input source in a single object, which may include
	a public identifier, a system identifier, a byte stream (possibly
	with a specified encoding), and/or a character stream.</p>
	
	<p>There are two places that the application will deliver this
	input source to the parser: as the argument to the XMLReader::parse()
	method, or as the return value of the EntityResolver.resolveEntity()
	method.</p>
	
	<p>The SAX parser will use the InputSource object to determine how
	to read XML input.  If there is a character stream available, the
	parser will read that stream directly; if not, the parser will use
	a byte stream, if available; if neither a character stream nor a
	byte stream is available, the parser will attempt to open a URL
	connection to the resource identified by the system identifier.</p>
	
	<p>An InputSource object belongs to the application: the SAX parser
	shall never modify it in any way (it may modify a copy if necessary).</p>
	
	@sa XMLReader::parse()
	@sa EntityResolver::resolveEntity()
	@sa io::InputStream
	@sa io::Reader
*/
//==============================================================================


#include "InputSource.h"

QC_SAX_NAMESPACE_BEGIN

//==============================================================================
// InputSource::InputSource
//
/**
   Default constructor.
*/
//==============================================================================
InputSource::InputSource()
{
}

//==============================================================================
// InputSource::InputSource
//
/**
	Create a new input source with a system identifier.
	
	<p>Applications may use setPublicId() to include a 
	public identifier as well, or setEncoding() to specify
	the character encoding, if known.</p>
	
	<p>If the system identifier is a URL, it should be full resolved.</p>
	
	@param systemId The system identifier (URI).
	@sa setPublicId()
	@sa setSystemId()
	@sa setByteStream()
	@sa setEncoding()
	@sa setCharacterStream()
*/
//==============================================================================
InputSource::InputSource(InputStream* pByteStream) :
    XMLInputSource(pByteStream)
{
}

//==============================================================================
// InputSource::InputSource
//
/**
	Create a new input source with a character stream.
	
	<p>Application writers may use setSystemId() to provide a base 
	for resolving relative URIs, and setPublicId() to include a 
	public identifier.</p>
	
	<p>The character stream must not include a byte order mark.</p>
	
	@sa setPublicId()
	@sa setSystemId()
	@sa setByteStream()
	@sa setCharacterStream()
*/
//==============================================================================
InputSource::InputSource(Reader* pCharacterStream) :
	XMLInputSource(pCharacterStream)
{
}

//==============================================================================
// InputSource::InputSource
//
/**
	Create a new input source with a byte stream.
	
	<p>Application writers may use setSystemId() to provide a base 
	for resolving relative URIs, setPublicId() to include a 
	public identifier, and/or setEncoding() to specify the byte stream's
	character encoding.</p>
	
	@param byteStream The raw byte stream containing the document.
	@sa setPublicId()
	@sa setSystemId()
	@sa setEncoding()
	@sa setByteStream()
	@sa setCharacterStream()
*/
//==============================================================================
InputSource::InputSource(const String& systemId) :
	XMLInputSource(systemId)
{
}

//==============================================================================
// InputSource::getByteStream
//
/**
	Get the byte stream for this input source.
	
	<p>The getEncoding() method will return the character
	encoding for this byte stream, or the empty string if unknown.</p>
	
	@return The byte stream, or null if none was supplied.
	@sa getEncoding()
	@sa setByteStream()
*/
//==============================================================================
AutoPtr<InputStream> InputSource::getByteStream() const
{
	return getInputStream();
}

//==============================================================================
// InputSource::getCharacterStream
//
/**
	Get the character stream for this input source.
	
	@return The character stream, or null if none was supplied.
	@sa setCharacterStream()
*/
//==============================================================================
AutoPtr<Reader> InputSource::getCharacterStream() const
{
	return getReader();
}

//==============================================================================
// InputSource::getSystemId
//
/**
	Set the system identifier for this input source.
	
	<p>The system identifier is optional if there is a byte stream
	or a character stream, but it is still useful to provide one,
	since the application can use it to resolve relative URIs
	and can include it in error messages and warnings (the parser
	will attempt to open a connection to the URI only if
	there is no byte stream or character stream specified).</p>
	
	<p>If the application knows the character encoding of the
	object pointed to by the system identifier, it can specify
	the encoding using the setEncoding() method.</p>
	
	<p>If the system ID is a URL, it should be fully resolved.</p>
	
	@param systemId The system identifier as a string.
	@sa setEncoding()
	@sa getSystemId()
	@sa Locator::getSystemId()
	@sa SAXParseException::getSystemId()
*/
//==============================================================================
String InputSource::getSystemId() const
{
	return XMLInputSource::getSystemId();
}

//==============================================================================
// InputSource::getEncoding
//
/**
	Get the character encoding for a byte stream or URI.
	
	@return The encoding, or the empty string if none was supplied.
	@sa setByteStream()
	@sa getSystemId()
	@sa getByteStream()
*/
//==============================================================================
String InputSource::getEncoding() const
{
	return XMLInputSource::getEncoding();
}

//==============================================================================
// InputSource::setEncoding
//
/**
	Set the character encoding, if known.
	
	<p>The encoding must be a string acceptable for an
	XML encoding declaration (see section 4.3.3 of the XML 1.0
	recommendation).</p>
	
	<p>This method has no effect when the application provides a
	character stream.</p>
	
	@param encoding A string describing the character encoding.
	@sa setSystemId()
	@sa setByteStream()
	@sa getEncoding()
*/
//==============================================================================
void InputSource::setEncoding(const String& encoding)
{
	XMLInputSource::setEncoding(encoding);
}

//==============================================================================
// InputSource::getPublicId
//
/**
	Get the public identifier for this input source.
	
	@return The public identifier, or the empty string if none was supplied.
	@sa setPublicId()
*/
//==============================================================================
String InputSource::getPublicId() const
{
	return XMLInputSource::getPublicId();
}

//==============================================================================
// InputSource::setSystemId
//
/**
	Set the system identifier for this input source.
	
	<p>The system identifier is optional if there is a byte stream
	or a character stream, but it is still useful to provide one,
	since the application can use it to resolve relative URIs
	and can include it in error messages and warnings (the parser
	will attempt to open a connection to the URI only if
	there is no byte stream or character stream specified).</p>
	
	<p>If the application knows the character encoding of the
	object pointed to by the system identifier, it can specify
	the encoding using the setEncoding() method.</p>
	
	<p>If the system identifier is a URL, it should be fully resolved.</p>
	
	@param systemId The system identifier as a string.
	@sa setEncoding()
	@sa getSystemId()
	@sa Locator::getSystemId()
	@sa SAXParseException::getSystemId()
*/
//==============================================================================
void InputSource::setSystemId(const String& systemId)
{
	XMLInputSource::setSystemId(systemId);
}

//==============================================================================
// InputSource::setPublicId
//
/**
	Set the public identifier for this input source.
	
	<p>The public identifier is always optional: if the application
	writer includes one, it will be provided as part of the
	location information.</p>
	
	@param publicId The public identifier as a string.
	@sa getPublicId
	@sa Locator::getPublicId()
	@sa SAXParseException::getPublicId()
*/
//==============================================================================
void InputSource::setPublicId(const String& publicId)
{
	XMLInputSource::setPublicId(publicId);
}

//==============================================================================
// InputSource::setCharacterStream
//
/**
	Set the character stream for this input source.
	
	<p>If there is a character stream specified, the SAX parser
	will ignore any byte stream and will not attempt to open
	a URL connection to the system identifier.</p>
	
	@param characterStream The character stream containing the
	       XML document or other entity.
	@sa getCharacterStream()
	@sa io::Reader
*/
//==============================================================================
void InputSource::setCharacterStream(Reader* pCharacterStream)
{
	XMLInputSource::setReader(pCharacterStream);
}

//==============================================================================
// InputSource::setByteStream
//
/**
	Set the byte stream for this input source.
	
	<p>The SAX parser will ignore this if there is also a character
	stream specified, but it will use a byte stream in preference
	to opening a URL connection itself.</p>
	
	<p>If the application knows the character encoding of the
	byte stream, it should set it with the setEncoding() method.</p>
	
	@param byteStream A byte stream containing an XML document or
	       other entity.
	@sa setEncoding()
	@sa getByteStream()
	@sa getEncoding()
	@sa io::InputStream
*/
//==============================================================================
void InputSource::setByteStream(InputStream* pByteStream)
{
	XMLInputSource::setInputStream(pByteStream);
}

QC_SAX_NAMESPACE_END

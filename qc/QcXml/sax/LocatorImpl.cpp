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
// Class LocatorImpl
/**
	@class qc::sax::LocatorImpl
	
	@brief Provides a convenience implementation of Locator.

	<p>This class is available mainly for application writers, who
	can use it to make a persistent snapshot of a locator at any
	point during a document parse:</p>
	
    @code
    void MyContentHandler::setLocator(Locator* pLocator)
    {
        // Store the locator for future ContentHandler events
        m_rpLocator = pLocator;
    }
    
    void MyContentHandler::startDocument()
    {
        // save the location of the start of the document
        // in a LocatorImpl member for future use.
        m_startLoc = *m_rpLocator;
    }
    @endcode
	
	<p>Normally, parser writers will not use this class, since it
	is more efficient to provide location information only when
	requested, rather than constantly updating a Locator object.</p>
*/
//==============================================================================

#include "LocatorImpl.h"

QC_SAX_NAMESPACE_BEGIN 

//==============================================================================
// LocatorImpl::LocatorImpl
//
/**
	Default constructor.
	<p>This will not normally be useful, since the main purpose
    of this class is to make a snapshot of an existing Locator.</p>
*/
//==============================================================================
LocatorImpl::LocatorImpl() :
	m_colNo(-1),
	m_lineNo(-1)
{
}

//==============================================================================
// LocatorImpl::LocatorImpl
//
/**
	Copy constructor.
	
	<p>Create a persistent copy of the current state of a locator.
	When the original locator changes, this copy will still keep
	the original values.</p>
	
	@param rhs The locator to copy.
*/
//==============================================================================
LocatorImpl::LocatorImpl(const Locator& rhs) :
	m_colNo(rhs.getColumnNumber()),
	m_lineNo(rhs.getLineNumber()),
	m_publicId(rhs.getPublicId()),
	m_systemId(rhs.getSystemId())
{
}

//==============================================================================
// LocatorImpl::getColumnNumber
//
/**
   Returns the stored column number, or -1 if none is available.
   
   <p>Note that column number is 1-based, therefore zero should never
   be returned.</p>
   @sa setColumnNumber()
*/
//==============================================================================
long LocatorImpl::getColumnNumber() const
{
	return m_colNo;
}

//==============================================================================
// LocatorImpl::getLineNumber
//
/**
   Returns the stored line number, or -1 if none is available.

   <p>Note that line number is 1-based, therefore zero should never
   be returned.</p>

   @sa setLineNumber()
*/
//==============================================================================
long LocatorImpl::getLineNumber() const
{
	return m_lineNo;
}

//==============================================================================
// LocatorImpl::getPublicId
//
/**
   Returns the stored public identifier, which may be an empty string.
   @sa setPublicId()
*/
//==============================================================================
String LocatorImpl::getPublicId() const
{
	return m_publicId;
}

//==============================================================================
// LocatorImpl::getSystemId
//
/**
   Returns the stored system identifier, which may be an empty string.
   @sa setSystemId()
*/
//==============================================================================
String LocatorImpl::getSystemId() const
{	
	return m_systemId;
}

//==============================================================================
// LocatorImpl::setColumnNumber
//
/**
	Set the column number for this locator (1-based).
	
	@param columnNumber The column number, or -1 if none is available.
	@sa getColumnNumber()
*/
//==============================================================================
void LocatorImpl::setColumnNumber(long columnNumber)
{
}

//==============================================================================
// LocatorImpl::setLineNumber
//
/**
	Set the line number for this locator (1-based).
	
	@param lineNumber The line number, or -1 if none is available.
	@sa getLineNumber()
*/
//==============================================================================
void LocatorImpl::setLineNumber(long lineNumber)
{
}

//==============================================================================
// LocatorImpl::setPublicId
//
/**
	Set the public identifier for this locator.
	
	@param publicId The new public identifier, or the empty string
	       if none is available.
	@sa getPublicId()
*/
//==============================================================================
void LocatorImpl::setPublicId(const String& publicId)
{
	m_publicId = publicId;
}

//==============================================================================
// LocatorImpl::setSystemId
//
/**
	Set the system identifier for this locator.
	
	@param systemId The new system identifier, or the empty string
	       if none is available.
	@sa getSystemId()
*/
//==============================================================================
void LocatorImpl::setSystemId(const String& systemId)
{
	m_systemId = systemId;
}

QC_SAX_NAMESPACE_END

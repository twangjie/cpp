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

#include "ExternalEntity.h"
#include "EntityEventHandler.h"
#include "EntityResolutionHandler.h"
#include "ParserImpl.h"
#include "Scanner.h"
#include "XMLFilterReader.h"
#include "XMLInputSource.h"
#include "XMLMessages.h"

#include "QcCore/base/debug.h"
#include "QcCore/base/NullPointerException.h"
#include "QcCore/base/System.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/util/MessageFormatter.h"
#include "QcCore/util/MIMEType.h"
#include "QcCore/net/URL.h"
#include "QcCore/net/URLConnection.h"
#include "QcCore/net/MalformedURLException.h"
#include "QcCore/io/File.h"
#include "QcCore/io/BufferedInputStream.h"
#include "QcCore/io/InputStreamReader.h"

QC_XML_NAMESPACE_BEGIN

using namespace net;
using namespace io;
using namespace util;

const static String sUTF8 = QC_T("UTF-8");
const static String sXML = QC_T("xml");

//==============================================================================
// ExternalEntity::ExternalEntity
//
// Construct a custom External Entity.  Normally used to create
// the document entity and a customized DTD
//==============================================================================
ExternalEntity::ExternalEntity(ParserImpl& parser, EntityType type,
                               XMLInputSource* pInputSource) : 
	Entity(type, 0, false),
	m_parser(parser),
	m_bLoaded(false)
{
	if(!pInputSource) throw NullPointerException();
	QC_DBG_ASSERT(type == EntityType::DTD || type == EntityType::Document);

	m_rpInputSource = new XMLInputSource(*pInputSource);
	m_systemId = pInputSource->getSystemId();
	m_publicId = pInputSource->getPublicId();
}

//==============================================================================
// ExternalEntity::ExternalEntity
//
// Construct an External Entity that has been declared in the DTD
//==============================================================================
ExternalEntity::ExternalEntity(ParserImpl& parser, EntityType type, const Entity* pParent,
                               bool bExternallyDeclared,
                               const String& name,
                               const String& publicId,
                               const String& systemId,
                               const String& notation) :
	Entity(type, pParent, bExternallyDeclared, name),
	m_parser(parser),
	m_bLoaded(false),
	m_notation(notation),
	m_systemId(systemId),
	m_publicId(publicId)
{
	QC_DBG_ASSERT(type != EntityType::Document);
	QC_DBG_ASSERT(pParent!=0);

	m_rpInputSource = new XMLInputSource(systemId);
	m_rpInputSource->setPublicId(publicId);
}


//==============================================================================
// ExternalEntity::load
//
// Start reading the ExternalEntity.
//
// If the XMLInputSource (which we copied on construction) has a character 
// reader then we don't need to do very much at all.  On the other hand
// if we just have a SystemId then we need to create an InputStream and we
// need to discover what type of wierd unfriendly encoding that stream
// uses before we can create a Reader to normalize it.
//==============================================================================
void ExternalEntity::load()
{
	if(isLoaded() || !isParsed())
		return;

	resolveEntity();

	m_rpReader = m_rpInputSource->getReader();

	//
	// If a Reader has been provided then we don't need to do much except
	// scan past the TextDecl if this isn't the document entity.
	//
	if(m_rpReader)
	{
		if(getType() != EntityType::Document)
		{
			// We must set the start position for non-document entities
			// and we need it anyway in order to parse the TextDecl
			m_startPosition = Scanner::GetPosition(*this, StreamPosition());
			
			String version, declEncoding;
			m_parser.parseTextDecl(m_startPosition, version, declEncoding);
		}
	}
	else
	{
		AutoPtr<InputStream> rpInputStream = m_rpInputSource->getInputStream();
		String extEncoding;

		//
		// If an InputStream hasn't even been provided, then we need to create
		// one by de-referencing the URL
		//
		if(rpInputStream.isNull())
		{
			URL url(m_resolvedSystemId);
			AutoPtr<URLConnection> rpCon = url.openConnection();
			rpCon->connect();

			// It is possible that the URLConnection will be able to give
			// us information about the encoding of the InputStream.  If so this
			// takes priority over the encoding in any TextDecl.
			//
			// RFCs 2045, 2376 and 3023 give details about the encoding
			// of XML entities and various MIME type registrations.
			// In a nutshell, we believe that the presence of a "charset"
			// parameter dictates the character encoding.
			//
			String strMimeType = rpCon->getContentType();
			if(!strMimeType.empty())
			{
				MIMEType mimeType(strMimeType);
				extEncoding = mimeType.getParameter(QC_T("charset"));
			}
			
			rpInputStream = rpCon->getInputStream();

			//
			// If the URL designates a protocol that support redirection
			// (such as HTTP), then the URL of the final connection
			// may differ from the original request URL.  We need to
			// know about this so that relative URLs of contained
			// entities will point to the correct location.
			//
			m_resolvedSystemId = rpCon->getURL().toExternalForm();
		}

		// All external entities (including the document entity)
		// may have an xml declaration possibly with an encoding and/or
		// a byte order mark.
		//
		// All this needs to be determined before we create the final Reader
		// for the stream.
		//
		// In order to test the first n characters of the input stream
		// we need to wrap it in a stream that permits the mark/reset
		// functionality - which the BufferedInputStream does
		AutoPtr<InputStream> rpBufferedStream = new BufferedInputStream(rpInputStream.get());

		createReader(m_parser, extEncoding, rpBufferedStream.get());

		// we must store the start position for non-document entities
		if(getType() != EntityType::Document)
		{
			m_startPosition = Scanner::GetPosition(*this, StreamPosition());
		}
	}

	// free up our XMLInputSource copy which is no-longer required
	m_rpInputSource.release();
	m_bLoaded = true;
}

//==============================================================================
// ExternalEntity::resolveEntity
//
// Helper method to call the appropriate EntityResolver to resolve the
// publicid/systemid into a valid XMLInputSource.
//==============================================================================
void ExternalEntity::resolveEntity()
{
	//
	// We only attempt to resolve entities that have a parent, i.e. they
	// have been declared in a DTD or DOCTYPE decl.
	//
	if(getParent())
	{
		EntityResolutionHandler* pResolver = m_parser.getEntityResolutionHandler();
		AutoPtr<XMLInputSource> rpResolvedIS;

		if(pResolver)
		{
			rpResolvedIS = pResolver->onResolveEntity(
					  getName(),
					  getType(),
					  m_rpInputSource->getPublicId(),
					  m_rpInputSource->getSystemId(),
					  getBaseURI());

			//
			// If an XMLInputSource has been returned, then any SystemId will have been
			// "rebased" that is, relative URIs will not be relative to the current
			// entity but will be relative to the something else, e.g. the xml:base
			// of a catalog.  This affects how we deal with relative URIs - if they
			// have been rebased then "file:" URLs are opened relative to the
			// current working directory, not made relative to the owning entity.
			//
		}

		if(rpResolvedIS)
		{
			m_rpInputSource = rpResolvedIS;
			m_resolvedSystemId = m_rpInputSource->getSystemId();
		}
		else
		{
			try
			{
				m_resolvedSystemId = 
					URL(getBaseURI(), m_rpInputSource->getSystemId()).toExternalForm();
			}
			catch(MalformedURLException&)
			{
				m_resolvedSystemId = m_rpInputSource->getSystemId();
			}
		}
	}
	else
	{
		// Default to using the file: protcol
		URL fileProtocol(QC_T("file:"));
		try
		{
			m_resolvedSystemId = 
				URL(fileProtocol, m_rpInputSource->getSystemId()).toExternalForm();
		}
		catch(MalformedURLException&)
		{
			m_resolvedSystemId = m_rpInputSource->getSystemId();
		}
	}
}

//==============================================================================
// ExternalEntity::createReader
//
// This function will automatically sense the encoding method used
// for the input stream and create an appropriate Reader
//
// The encoding may be declared with a Byte Order Mark
// (see XML1.0 4.3.3 & Appendix F), or by reading the first few bytes of the
// stream (which must be "<?xml" for non UTF-8/UTF-16 encoded files) and seeing
// how they are represented.
// 
// XML1.0 section 4.3.3 states that entities encoded in UTF-16 must begin
// with a byte order mark.  
//
// It also states:-
// "In the absence of external encoding information (such as MIME headers),
// parsed entities which are stored in an encoding other than UTF-8 or UTF-16
// must begin with a text declaration containing an encoding declaration."
//
// So, if there is no Byte Order Mark and no encoding declaration then
// the external entity must be encoded in UTF-8.
//==============================================================================
void ExternalEntity::createReader(ParserImpl& parser, const String& extEncoding,
                                  InputStream* pInputStream)
{
	size_t nByteOrderMarkSize=0;

	String encoding = 
		InputStreamReader::SenseEncoding(pInputStream, nByteOrderMarkSize);
	
	if(nByteOrderMarkSize)
	{
		QC_DBG_ASSERT(!encoding.empty());
		pInputStream->skip(nByteOrderMarkSize);
	}
	else
	{
		encoding = sUTF8;

		QC_DBG_ASSERT(pInputStream->markSupported());
		pInputStream->mark(4);
	
		//
		// Read the first 4 bytes from the stream
		// and test to see if we have a byte order mark
		//

		Byte decl[4];	// Hopefully an xml declaration

		// Taken from Appendix F, XML1.0, Second Edition:-
		//

		if(pInputStream->read(decl, 4)==4)
		{

			if(decl[0] == 0 && decl[1] == 0 && decl[2] == 0 && decl[3] == '<')
				encoding = QC_T("UCS-4BE");
			else if(decl[0] == '<' && decl[1] == 0 && decl[2] == 0 && decl[3] == 0)
				encoding = QC_T("UCS-4LE");
			else if(decl[0] == 0 && decl[1] == 0 && decl[2] == '<' && decl[3] == 0)
				encoding = QC_T("UCS-4-2143");
			else if(decl[0] == 0 && decl[1] == '<' && decl[2] == 0 && decl[3] == 0)
				encoding = QC_T("UCS-4-3412");
			else if(decl[0] == 0 && decl[1] == '<' && decl[2] == 0 && decl[3] == '?')
				encoding = QC_T("UTF-16BE");
			else if(decl[0] == '<' && decl[1] == 0 && decl[2] == '?' && decl[3] == 0)
				encoding = QC_T("UTF-16LE");
			else if(decl[0] == '<' && decl[1] == '?' && decl[2] == 'x' && decl[3] == 'm')
				encoding = sUTF8;
			else if(decl[0] == 0x4c && decl[1] == 0x6f && decl[2] == 0xa7 && decl[3] == 0x94)
				encoding = QC_T("ECBDIC");
		}
		pInputStream->reset();
	}

	//
	// Okay, we have sensed the InputStream's encoding - but now we may just disregard it
	// if we have external encoding information.
	//
	if(!extEncoding.empty())
	{
		encoding = extEncoding;
	}

	size_t bufSize = 1000;
	pInputStream->mark(bufSize);

	//
	// Now create a (temporary) Reader for the purposes of parsing the 
	// TextDecl.
	//
	// This doesn't require (currently) any normalisation of newlines
	// so we can use a basic InputStreamReader.  We don't wish to employ
	// strict encoding rules at this stage.
	//
	m_rpReader = new InputStreamReader(pInputStream, encoding, false);

	String version, declEncoding;

	//
	// Get a ScannerPosition to be used for parsing the TextDecl
	// Notice that we use a buffer size equivalent to the value that we have marked,
	// so that we don't attempt to read ahead of what has been marked.
	//
	ScannerPosition entPos = Scanner::GetPosition(*this, StreamPosition(), bufSize);

	size_t textDeclSize = parser.parseTextDecl(entPos, version, declEncoding);
	
	//
	// We now have the declared encoding (if any) as well as the BOM-derived
	// encoding and possibly an External encoding.
	//
	// External Encodings rule.  After that I suppose the textDecl
	// encoding rules - but it must be compatible with the BOM encoding which
	// we used to parse the textDecl!
	//
	// For UTF-16 they probably won't match because the encoding name should
	// be UTF-16 and the BOM-derived name will have LE or BE appended.  In
	// this case we obviously need to use the BOM name, but how do we
	// make this more generic?
	//
	if(textDeclSize && extEncoding.empty() && !declEncoding.empty())
	{
		if(StringUtils::CompareNoCase(declEncoding, QC_T("UTF-16")) == 0)
		{
			// leave encoding alone
		}
		else
		{
			encoding = declEncoding;
		}
	}

	// Reset the input stream back to the beginning (after the BOM)
	pInputStream->reset();

	// Now create a new reader for the input stream.  This time we request 
	// strict encoding.
	// todo: make this a parser feature?

	QC_DBG_ASSERT(!encoding.empty());

	m_rpReader = new InputStreamReader(pInputStream, encoding, true);

	// If this isn't the document entity then we want to skip past
	// the TextDecl (it is a property of the entity and is not parsed)
	if(getType() != EntityType::Document)
	{
		m_rpReader->skip(textDeclSize);
	}

	//
	// And now that the reader is pointing at the correct position,
	// we can wrap it in a filter to normaize newlines
	//
	m_rpReader = new XMLFilterReader(m_rpReader.get());
}

//==============================================================================
// ExternalEntity::getStartPosition
//
// Obtain a ScannerPosition indicating the start of this entity.
//==============================================================================
ScannerPosition ExternalEntity::getStartPosition()
{
	QC_DBG_ASSERT(isParsed());

	load();

	//
	// Note: we do not hold a start position for the document entity
	// because we will never need to re-scan from the start.  For all
	// other entities we do.  
	//
	// Todo: release PE entities when DTD fully parsed
	//
	if(getType() == EntityType::Document)
	{
		return Scanner::GetPosition(*this, StreamPosition());
	}
	else
	{
		return m_startPosition;
	}
}

//==============================================================================
// ExternalEntity::isParsed
//
//==============================================================================
bool ExternalEntity::isParsed() const
{
	return (m_notation.empty());
}

//==============================================================================
// ExternalEntity::getNotation
//
//==============================================================================
const String& ExternalEntity::getNotation() const
{
	return m_notation;
}

//==============================================================================
// ExternalEntity::isLoaded
//
//==============================================================================
bool ExternalEntity::isLoaded() const
{
	return m_bLoaded;
}

//==============================================================================
// ExternalEntity::getReader
//
//==============================================================================
AutoPtr<Reader> ExternalEntity::getReader() const
{
	return m_rpReader;
}

//==============================================================================
// ExternalEntity::getURL
//
// Return the resolved system identifier for this eternal entity.
// Note: It is possible that this won't be a valid URL.  This will be true
// if the entity resolver provided an InputStream or Reader, or if an
// unsupported protocol is being used - which is being resolved by the Resolver.
//==============================================================================
String ExternalEntity::getResolvedSystemId() const
{
	return m_resolvedSystemId;
}

//==============================================================================
// ExternalEntity::validate
//
// Perform validation checks.
// This function is called after the entire DTD (internal and external subsets)
// has been created.
//==============================================================================
void ExternalEntity::validate(ParserImpl& parser) const
{
	//
	// Entities with an NDATA decl must refer to a declared NOTATION
	//
	if(!isParsed()) // ie we have an NDATA decl
	{
		if(!parser.isNotationDeclared(getNotation()))
		{
			const String& errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_UNDECLNOTNENTITY,
				"undeclared notation '{0}' referenced in declaration of entity '{1}'"),
				getNotation(),
				getName());

			parser.errorDetected(Parser::Error, errMsg, EXML_UNDECLNOTNENTITY);
		}
	}
}

//==============================================================================
// ExternalEntity::getSystemId
//
// Return the system identifier for this external entity.
//==============================================================================
String ExternalEntity::getSystemId() const
{
	return m_systemId;
}

//==============================================================================
// ExternalEntity::getPublicId
//
// Return the public identifier for this external entity
//==============================================================================
String ExternalEntity::getPublicId() const
{
	return m_publicId;
}

//==============================================================================
// ExternalEntity::isExternalEntity
//
//==============================================================================
bool ExternalEntity::isExternalEntity() const
{
	return true;
}

//==============================================================================
// ExternalEntity::isInternalEntity
//
//==============================================================================
bool ExternalEntity::isInternalEntity() const
{
	return false;
}

//==============================================================================
// ExternalEntity::getBaseURI
//
//==============================================================================
String ExternalEntity::getBaseURI() const
{
	if(getParent())
	{
		return getParent()->getResolvedSystemId();
	}
	else
	{
		return getResolvedSystemId();
	}
}

QC_XML_NAMESPACE_END


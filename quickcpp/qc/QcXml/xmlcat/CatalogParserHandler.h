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

#ifndef CatalogParserHandler_h
#define CatalogParserHandler_h

#include "defs.h"
#include "CatalogSerialisationContext.h"

#include "QcXml/xml/ContentEventHandler.h"
#include "QcXml/xml/ErrorEventHandler.h"
#include "QcXml/xml/Parser.h"

#include <stack>
#include <deque>

QC_XMLCAT_NAMESPACE_BEGIN

using namespace xml;

class CatalogFile;

class QC_XMLCAT_PKG CatalogParserHandler : public ContentEventHandler,
                                           public ErrorEventHandler
{
public:

	CatalogParserHandler(CatalogFile& catalogFile, const Parser& parser);
	virtual ~CatalogParserHandler();
	
protected:

	// ContentEventHandler methods...
	virtual void onComment(const CharType* pComment, size_t length);
	virtual void onCommentStart(bool bInsideDTD);
	virtual void onCommentEnd(bool bInsideDTD);
	virtual void onPI(const String& PITarget, const String& PIValue);
	virtual void onStartElement(const QName& elementName, bool bEmptyElement, const AttributeSet& attributes);
	virtual void onEndElement(const QName& elementName);
	virtual void onWhitespace(const CharType* pStr, size_t len);
	virtual void onIgnorableWhitespace(const CharType* pStr, size_t len);
	virtual void onCharData(const CharType* pStr, size_t len);
	virtual void onStartCData();
	virtual void onEndCData();
	virtual void onNamespaceBegin(const String& prefix, const String& uri);
	virtual void onNamespaceChange(const String& prefix, const String& uriFrom, const String& uriTo, bool bRestoring);
	virtual void onNamespaceEnd(const String& prefix, const String& uri);
	virtual void onStartDocument(const String& systemId);
	virtual void onEndDocument();
	virtual void onXMLDeclaration(const String& version, const String& encoding, bool bStandalone);

	// ErrorEventHandler methods...
	virtual void onError(int level,
	                     long /*reserved*/,
	                     const String& message,
	                     const Position& position,
	                     const ContextString* pContextString);


protected:
	void createCatalogEntry(const String& elementName,
	                        const AttributeSet& attributes,
	                        CatalogSerialisationContext& context);

	String getRequiredAttribute(const String& elementName,
	                            const String& attrName,
	                            const AttributeSet& attributes);

	void reportError(int level, const String& message, const Position& position);

private:
	CatalogFile& m_catalogFile;
	typedef std::stack<CatalogSerialisationContext, std::deque<CatalogSerialisationContext> > ContextStack;
	ContextStack m_contextStack;
	const Parser& m_parser;
};

QC_XMLCAT_NAMESPACE_END

#endif // CatalogParserHandler_h

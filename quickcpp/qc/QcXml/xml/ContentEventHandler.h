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

#ifndef QC_XML_ContentEventHandler_h
#define QC_XML_ContentEventHandler_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

#include "EventHandler.h"

QC_XML_NAMESPACE_BEGIN

class QName;
class BufferRange;
class AttributeSet;

class QC_XML_PKG ContentEventHandler : public EventHandler
{
public:

	virtual void onComment(const CharType* pComment, size_t length) = 0;
	virtual void onCommentStart(bool bInsideDTD) = 0;
	virtual void onCommentEnd(bool bInsideDTD) = 0;
	virtual void onPI(const String& PITarget, const String& PIValue) = 0;
	virtual void onStartElement(const QName& elementName, bool bEmptyElement, const AttributeSet& attributes) = 0;
	virtual void onEndElement(const QName& elementName) = 0;
	virtual void onWhitespace(const CharType* pStr, size_t len) = 0;
	virtual void onIgnorableWhitespace(const CharType* pStr, size_t len) = 0;
	virtual void onCharData(const CharType* pString, size_t len) = 0;
	virtual void onStartCData() = 0;
	virtual void onEndCData() = 0;
	virtual void onNamespaceBegin(const String& prefix, const String& uri) = 0;
	virtual void onNamespaceChange(const String& prefix, const String& uriFrom, const String& uriTo, bool bRestoring) = 0;
	virtual void onNamespaceEnd(const String& prefix, const String& uri) = 0;
	virtual void onStartDocument(const String& systemId) = 0;
	virtual void onEndDocument() = 0;
	virtual void onXMLDeclaration(const String& version, const String& encoding, bool bStandalone) = 0;
};

QC_XML_NAMESPACE_END

#endif //QC_XML_ContentEventHandler_h

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

#ifndef QC_XML_DTDEventHandler_h
#define QC_XML_DTDEventHandler_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

#include "EventHandler.h"

QC_XML_NAMESPACE_BEGIN

class EntityType;

class QC_XML_PKG DTDEventHandler : public EventHandler
{
public:
	virtual void onStartDTD(const String& name, const String& publicId, const String& systemId) = 0;
	virtual void onEndDTD() = 0;
	virtual void onNotationDecl(const String& name, const String& publicId, const String& systemId) = 0;
	virtual void onUnparsedEntityDecl(const String& name, const String& publicId, const String& systemId, const String& notationName) = 0;
	virtual void onExternalEntityDecl(const String& name, EntityType type, const String& publicId, const String& systemId) = 0;
	virtual void onInternalEntityDecl(const String& name, EntityType type, const String& value) = 0;
	virtual void onStartAttlist(const String& eName) = 0;
	virtual void onEndAttlist() = 0;
	virtual void onAttributeDecl(const String& eName, const String& aName, const String& type, const String& valueDefault, const String& enumGroup, const String& value) = 0;
	virtual void onElementDecl(const String& name, const String& model) = 0;
};

QC_XML_NAMESPACE_END

#endif //QC_XML_DTDEventHandler_h

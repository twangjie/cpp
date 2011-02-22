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

#ifndef QC_XML_Entity_h
#define QC_XML_Entity_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

#include "EntityType.h"
#include "ScannerPosition.h"
#include "DTDObject.h"

#include "QcCore/net/URL.h"

QC_XML_NAMESPACE_BEGIN

using net::URL;

class ParserImpl;

class QC_XML_PKG Entity : public DTDObject
{
public:

	Entity(EntityType type, const Entity* pParent, bool bExternallyDeclared);
	Entity(EntityType type, const Entity* pParent, bool bExternallyDeclared, const String& name);
	virtual ~Entity();

	virtual void validate(ParserImpl& parser) const;
	bool operator==(const Entity& rhs) const;
	bool operator!=(const Entity& rhs) const;
	EntityType getType() const;
	const String& getName() const;

	virtual bool isExternalEntity() const = 0;
	virtual bool isInternalEntity() const = 0;
	virtual bool isParsed() const = 0;
	virtual void load() = 0;
	virtual ScannerPosition getStartPosition() = 0;
	virtual AutoPtr<Reader> getReader() const = 0;
	virtual String getResolvedSystemId() const;
	virtual String getBaseURI() const;
	virtual String getPublicId() const;
	virtual String getSystemId() const;

	const Entity* getParent() const;
	const Entity* getTopParent() const;

private: // not implemented
	Entity(const Entity& rhs);                  // entities cannot be copied
	const Entity& operator=(const Entity& rhs); // nor assigned

private:
	EntityType m_type;
	String m_name;
	const Entity* m_pParent;
};

QC_XML_NAMESPACE_END

#endif //QC_XML_Entity_h

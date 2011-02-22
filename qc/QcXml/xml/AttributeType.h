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
//
// Class: AttributeType
// 
// Overview
// --------
// Records information about an element type's attribute definition (from a DTD)
//
//==============================================================================

#ifndef QC_XML_AttributeType_h
#define QC_XML_AttributeType_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

#include "DTDObject.h"
#include "QName.h"

#include <set>

QC_XML_NAMESPACE_BEGIN

class Attribute;
class ParserImpl;
class ElementType;

class QC_XML_PKG AttributeType : public DTDObject
{
public:

	enum Type {UNKNOWN, CDATA, ID, IDREF, IDREFS, ENTITY, ENTITIES, NMTOKEN,
	           NMTOKENS, NOTATION, ENUMERATION};

	enum DefaultType {REQUIRED, IMPLIED, DEFAULT, FIXED};

	AttributeType(ElementType& elementType, bool bExternallyDeclared, const QName& name);
	~AttributeType();

	DefaultType getDefaultType() const;
	Type getType() const;
	String getTypeAsString() const;
	String getEnumListAsString() const;
	const String& getDefaultValue() const;
	const QName& getName() const;

	void setDefaultType(DefaultType eDefaultType);
	void setType(Type eType);
	void setDefaultValue(const String& value, ParserImpl& parser);

	bool addEnum(const String& name, ParserImpl& parser);
	bool containsEnum(const String& name) const;
	bool hasDefaultValue() const;

	void validate(ParserImpl& parser) const;
	void validateAttribute(const Attribute& attr, ParserImpl& parser) const;
	bool isEnumerated() const;
	bool isStringType() const;
	bool isTokenized() const;

	static String GetTypeAsString(Type eType);

private:
	bool testNmToken(bool bDefault, bool bName, const String& value, ParserImpl& parser) const;
	bool testNmTokens(bool bDefault, bool bName, const String& value, ParserImpl& parser) const;
	bool testEntity(const String& value, ParserImpl& parser) const;

private: // not implemented
	AttributeType(const AttributeType& rhs);            // cannot be copied
	AttributeType& operator=(const AttributeType& rhs); // nor assigned

private:
	String m_defaultValue;
	typedef std::set<String, std::less<String> > EnumSet;
	EnumSet m_enums;
	DefaultType m_eDefaultType;
	ElementType& m_elementType;
	Type m_eType;
	QName m_name;
};

QC_XML_NAMESPACE_END

#endif //QC_XML_AttributeType_h

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

#ifndef QC_XML_ElementType_h
#define QC_XML_ElementType_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

#include "AttributeType.h"

#include "DTDObject.h"
#include "Attribute.h"
#include <map>

QC_XML_NAMESPACE_BEGIN

class Parser;
class ElementContentSpec;
class AttributeSet;

class QC_XML_PKG ElementType : public DTDObject
{
public:
	typedef AutoPtr<AttributeType> RPAttrType;
	typedef std::map<String, RPAttrType, std::less<String> > AttributeTypeMap;
	
	enum ContentType {ANY, EMPTY, MIXED, SPECIFIED};

	ElementType(const QName& name);
	virtual ~ElementType();

	AutoPtr<AttributeType> getAttributeType(const QName& name) const;
	AutoPtr<AttributeType> addAttributeType(const QName& name, bool bExternallyDeclared);

	const QName& getName() const;
	bool isDefined() const;
	void setDefined(bool bDefined, bool bExternallyDeclared);

	const AttributeTypeMap& getAttributeTypeMap() const;

	bool hasIdAttribute() const;
	const String& getIdAttributeName() const;
	void setIdAttributeName(const String& rawname);

	bool hasNotationAttribute() const;
	const String& getNotationAttributeName() const;
	void setNotationAttributeName(const String& rawname);

	ElementContentSpec* getContentSpec() const;
	ContentType getContentType() const;
	String getContentTypeAsString() const;
	void setContentType(ContentType eContentType);

	virtual void validate(ParserImpl& parser) const;
	virtual void validateMissingAttributes(AttributeSet& attrSet, bool bValidate, ParserImpl& parser) const;

	bool containsEnum(const String& value) const;
	bool allowsCharData() const;
	bool allowsWhitespace() const;

private: // not implemented
	ElementType(const ElementType& rhs);            // cannot be copied
	ElementType& operator=(const ElementType& rhs); // nor assigned

private:
	AttributeTypeMap m_attributeTypeMap;
	QName m_name;
	bool m_bDefined;
	ContentType m_eContentType;
	ElementContentSpec* m_pContentSpec;
	String m_idAttributeName;
	String m_notationAttributeName;
};

QC_XML_NAMESPACE_END

#endif //QC_XML_ElementType_h

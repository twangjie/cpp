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

#include "ElementType.h"
#include "AttributeSet.h"
#include "ElementContentSpec.h"
#include "ParserImpl.h"
#include "XMLMessages.h"

#include "QcCore/base/debug.h"
#include "QcCore/base/System.h"
#include "QcCore/util/MessageFormatter.h"

QC_XML_NAMESPACE_BEGIN

using namespace util;

const static String sXML = QC_T("xml");

ElementType::ElementType(const QName& name) : 
	DTDObject(true),
	m_name(name),
	m_bDefined(false),
	m_eContentType(ANY),
	m_pContentSpec(0)
{
}

ElementType::~ElementType()
{
	delete m_pContentSpec;
}

//=============================================================================
// ElementType::getAttributeType
// 
// Locate and return an AttributeType from our internal map.
//
// Returns a NULL ptr if no matching entry exists.
//
//=============================================================================
AutoPtr<AttributeType> ElementType::getAttributeType(const QName& name) const
{
	AttributeTypeMap::const_iterator iter = m_attributeTypeMap.find(name.getRawName());
	if(iter != m_attributeTypeMap.end())
	{
		return (*iter).second;
	}
	else
	{
		return 0;
	}
}

//=============================================================================
// ElementType::addAttributeType
// 
// Add a new attribute to our map
//
//=============================================================================
AutoPtr<AttributeType> ElementType::addAttributeType(const QName& name, bool bExternallyDeclared)
{
	QC_DBG_ASSERT(m_attributeTypeMap.find(name.getRawName()) == m_attributeTypeMap.end());
	AutoPtr<AttributeType> pRet(new AttributeType(*this, bExternallyDeclared, name));
	m_attributeTypeMap[name.getRawName()] = pRet;
	return pRet;
}

//==============================================================================
// ElementType::getName
//
//==============================================================================
const QName& ElementType::getName() const
{
	return m_name;
}

//==============================================================================
// ElementType::isDefined
//
// Elements can be created in a DTD via an <!ELEMENT delcaration or via an
// <!ATTLIST declaration.  In the latter case the element is "declared" but
// not yet "defined".
//==============================================================================
bool ElementType::isDefined() const
{
	return m_bDefined;
}

//==============================================================================
// ElementType::setDefined
//
//==============================================================================
void ElementType::setDefined(bool bDefined, bool bExternallyDeclared)
{
	setExternallyDeclared(bExternallyDeclared);
	m_bDefined = bDefined;
}

//==============================================================================
// ElementType::getAttributeTypeMap
//
//==============================================================================
const ElementType::AttributeTypeMap& ElementType::getAttributeTypeMap() const
{
	return m_attributeTypeMap;
}

//==============================================================================
// ElementType::validate
//
// Perform post DTD validation, i.e. validation checks that must be performed
// when the entire DTD has been processed.
//==============================================================================
void ElementType::validate(ParserImpl& parser) const
{
	AttributeTypeMap::const_iterator iter;
	for(iter=m_attributeTypeMap.begin(); iter!=m_attributeTypeMap.end(); ++iter)
	{
		const AutoPtr<AttributeType>& rpAttr = (*iter).second;
		rpAttr->validate(parser);
	}

	//
	// A couple of warning/compatability checks follow
	//
	if(parser.m_features.m_bDoWarningChecks)
	{
		if(m_eContentType == EMPTY && hasNotationAttribute())
		{
			// Validity Constraint: (For compatability) No Notation on Empty Element

			const String& errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_EMPTYELEMNOTN,
				"attribute '{0}' of type NOTATION must not be declared on element '{1}' which has been declared EMPTY"),
				getNotationAttributeName(),
				m_name.getRawName());

			parser.errorDetected(Parser::Error, errMsg, EXML_EMPTYELEMNOTN);
		}

		// XML 1.0, 3.3 says we can (at user option) issue a warning 
		// if the element has not been declared
		if(!m_bDefined)
		{
			const String& errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_ELEMUNDEFATTLIST,
				"attribute list declared for undefined element '{0}'"),
				m_name.getRawName());

			parser.errorDetected(Parser::Warning, errMsg, EXML_ELEMUNDEFATTLIST);
		}
	}
}

ElementType::ContentType ElementType::getContentType() const
{
	return m_eContentType;
}

void ElementType::setContentType(ContentType eContentType)
{
	m_eContentType = eContentType;

	if(m_pContentSpec == 0 && m_eContentType != ANY)
	{
		m_pContentSpec = new ElementContentSpec(m_name.getRawName());
	}
}

ElementContentSpec* ElementType::getContentSpec() const
{
	return m_pContentSpec;
}

//==============================================================================
// ElementType::validateMissingAttributes
//
// Test if all required attributes have been specified and add attributes
// that have a default value
//
//==============================================================================
void ElementType::validateMissingAttributes(AttributeSet& attSet, bool bValidate, ParserImpl& parser) const
{
	AttributeTypeMap::const_iterator iter;
	for(iter=m_attributeTypeMap.begin(); iter!=m_attributeTypeMap.end(); ++iter)
	{
		const AutoPtr<AttributeType>& rpAttrType = (*iter).second;
		if(rpAttrType->getDefaultType() == AttributeType::REQUIRED)
		{
			if(bValidate && !attSet.getAttribute(rpAttrType->getName().getRawName()))
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_ATTRREQUIRED,
					"required attribute '{0}' has not been supplied for element '{1}'"),
					rpAttrType->getName().getRawName(),
					getName().getRawName());

				parser.errorDetected(Parser::Error, errMsg, EXML_ATTRREQUIRED);
			}
		}
		else if(rpAttrType->getDefaultType() != AttributeType::IMPLIED)
		{
			// XML 1.0 says that attributes with default value
			// that are not present should be created
			if(!attSet.getAttribute(rpAttrType->getName().getRawName()))
			{
				AutoPtr<Attribute> rpAttr = new Attribute(rpAttrType->getName(), rpAttrType->getDefaultValue(), rpAttrType->getTypeAsString());
				
				attSet.addAttribute(rpAttr.get());

				//
				// If we have had to add a defaulted attribute, and if the attribute
				// definition is external, and the document claims to be standalone
				// then we have a vandity constraint error
				//
				if(bValidate && parser.isStandaloneDocument() && rpAttrType->isExternallyDeclared())
				{
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_ATTRDEFAULTNOTSA,
						"externally declared attribute '{0}' for element '{1}' has a default value of '{2}' which must be specified in a standalone document"),
						rpAttrType->getName().getRawName(),
						getName().getRawName(),
						rpAttrType->getDefaultValue());

					parser.errorDetected(Parser::Error, errMsg, EXML_ATTRDEFAULTNOTSA);
				}
			}
		}
	}
}

//==============================================================================
// ElementType::hasIdAttribute
//
// Return true is there exists an attribute of type ID for this element.
//==============================================================================
bool ElementType::hasIdAttribute() const
{
	return !m_idAttributeName.empty();
}

//==============================================================================
// ElementType::getIdAttributeName
//
//==============================================================================
const String& ElementType::getIdAttributeName() const
{
	return m_idAttributeName;
}

//==============================================================================
// ElementType::setIdAttributeName
//
// set the name of the ID atrribute for this Element
//==============================================================================
void ElementType::setIdAttributeName(const String& rawname)
{
	m_idAttributeName = rawname;
}

//==============================================================================
// ElementType::hasNotationAttribute
//
// Return true is there exists an attribute of type NOTATION for this element.
//==============================================================================
bool ElementType::hasNotationAttribute() const
{
	return !m_notationAttributeName.empty();
}

//==============================================================================
// ElementType::getNotationAttributeName
//
//==============================================================================
const String& ElementType::getNotationAttributeName() const
{
	return m_notationAttributeName;
}

//==============================================================================
// ElementType::setNotationAttributeName
//
// set the name of the NOTATION atrribute for this Element
//==============================================================================
void ElementType::setNotationAttributeName(const String& rawname)
{
	m_notationAttributeName = rawname;
}

//==============================================================================
// ElementType::containsEnum
//
// Simple routine to test for the presence of a particular enum value in all
// the attributes defined for this element type.
//==============================================================================
bool ElementType::containsEnum(const String& value) const
{
	AttributeTypeMap::const_iterator iter;
	for(iter=m_attributeTypeMap.begin(); iter!=m_attributeTypeMap.end(); ++iter)
	{
		const AutoPtr<AttributeType>& rpAttr = (*iter).second;
		if(rpAttr->containsEnum(value))
		{
			return true;
		}
	}
	return false;
}

//==============================================================================
// ElementType::allowsCharData
//
// Simple function to decide whether or not character data is permitted within
// this element type.
//==============================================================================
bool ElementType::allowsCharData() const
{
	return (m_eContentType == ANY || m_eContentType == MIXED);
}

//==============================================================================
// ElementType::allowsWhitespace
//
// Simple function to decide whether or not white-space is permitted within
// this element type.
//==============================================================================
bool ElementType::allowsWhitespace() const
{
	return (m_eContentType != EMPTY);
}

//==============================================================================
// ElementType::getContentTypeAsString
//
//==============================================================================
String ElementType::getContentTypeAsString() const
{
	if(m_eContentType == EMPTY)
	{
		return QC_T("EMPTY");
	}
	else if(m_eContentType == ANY)
	{
		return QC_T("ANY");
	}
	else
	{
		return m_pContentSpec->asString((m_eContentType == MIXED));
	}
}

QC_XML_NAMESPACE_END


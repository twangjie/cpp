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

#include "AttributeType.h"
#include "Attribute.h"
#include "ElementType.h"
#include "Entity.h"
#include "ParserImpl.h"
#include "XMLMessages.h"

#include "QcCore/base/debug.h"
#include "QcCore/base/System.h"
#include "QcCore/util/MessageFormatter.h"
#include "QcCore/util/StringTokenizer.h"

#include <algorithm>

QC_XML_NAMESPACE_BEGIN

using namespace util;

const String sSpace = QC_T(" ");
const String sXML = QC_T("xml");

AttributeType::AttributeType(ElementType& elementType, bool bExternallyDeclared, const QName& name) : 
	DTDObject(bExternallyDeclared),
	m_elementType(elementType),
	m_name(name)
{
}

AttributeType::~AttributeType()
{
}

//==============================================================================
// AttributeType::getDefaultType
//
//==============================================================================
AttributeType::DefaultType AttributeType::getDefaultType() const
{
	return m_eDefaultType;
}

//==============================================================================
// AttributeType::getType
//
//==============================================================================
AttributeType::Type AttributeType::getType() const
{
	return m_eType;
}

//==============================================================================
// AttributeType::getName
//
//==============================================================================
const QName& AttributeType::getName() const
{
	return m_name;
}

//==============================================================================
// AttributeType::getDefaultValue
//
//==============================================================================
const String& AttributeType::getDefaultValue() const
{
	return m_defaultValue;
}

//==============================================================================
// AttributeType::setDefaultType
//
//==============================================================================
void AttributeType::setDefaultType(DefaultType eDefaultType)
{
	m_eDefaultType = eDefaultType;
}

//==============================================================================
// AttributeType::setType
//
//==============================================================================
void AttributeType::setType(Type eType)
{
	m_eType = eType;
}

//==============================================================================
// AttributeType::setDefaultValue
//
// The value that will be used 
// (a) to test against #FIXED attributes
// (b) to use as #DEFAULT
//==============================================================================
void AttributeType::setDefaultValue(const String& value, ParserImpl& parser)
{
	m_defaultValue = value;

	//
	// Validity Constraint: Attribute Default Value
	//
	if(parser.m_features.m_bDoValidityChecks)
	{
		switch(m_eType)
		{
			case ID:
			case UNKNOWN:
				// Do nothing, this is a weird situation where the
				// syntax doesn't match with the type.  But that mismatch
				// is only a VC, so it is possible.
				break;

			case CDATA:
				break;

			case ENTITY:
			case IDREF:
				testNmToken(true, true/*Name*/, value, parser);
				break;

			case ENTITIES:
			case IDREFS:
				testNmTokens(true, true/*Name*/, value, parser);
				break;

			case NMTOKEN:
				testNmToken(true, false/*NmToken*/, value, parser);
				break;

			case NMTOKENS:
				testNmTokens(true, false/*NmToken*/, value, parser);
				break;

			case NOTATION:
				if(!testNmToken(true, true/*Name*/, value, parser))
				{
					break;
				}
				// else fall thru and check enumerations
			case ENUMERATION:
				if(!containsEnum(value))
				{
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_ATTRDEFAULTENUM,
						"default value for attribute '{0}' must be a value within the enumeration list"),
						getName().getRawName());

					parser.errorDetected(Parser::Error, errMsg, EXML_ATTRDEFAULTENUM);
				}
		}
	}
}

//==============================================================================
// AttributeType::addEnum
//
// Note: the parser will already have validated the enum value,but not whether
// or not it was a duplicate
//
//==============================================================================
bool AttributeType::addEnum(const String& value, ParserImpl& parser)
{
	QC_DBG_ASSERT(isEnumerated());

	bool bExistingEnum = false;

	if(parser.m_features.m_bDoInteroperabilityChecks)
	{
		bExistingEnum = m_elementType.containsEnum(value);
	}

	bool bInsertOk = m_enums.insert(value).second;

	if(parser.m_features.m_bDoValidityChecks)
	{
		//
		// Validity constraint: No duplicate tokens
		//
		if(!bInsertOk)
		{
			const String& errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_ATTRDUPLTOKEN,
				"duplicate token value '{0}' for attribute '{1}'"),
				value, getName().getRawName());

			parser.errorDetected(Parser::Error, errMsg, EXML_ATTRDUPLTOKEN);
		}
		else if(bExistingEnum) // therefore interoperability tests must be enabled
		{
			QC_DBG_ASSERT(parser.m_features.m_bDoInteroperabilityChecks);

			const String& errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_INTEROPENUM,
				"for interoperability, token value '{0}' must not appear more than once in attributes for element '{1}'"),
				value, m_elementType.getName().getRawName());

			parser.errorDetected(Parser::Error, errMsg, EXML_INTEROPENUM);
		}
	}

	return bInsertOk;
}

//==============================================================================
// AttributeType::isEnumerated
//
// Return true if this Attribute is an enumeration. ie it contains a set
// of valid values.
//==============================================================================
bool AttributeType::isEnumerated() const
{
	return (m_eType == ENUMERATION || m_eType == NOTATION);
}

//==============================================================================
// AttributeType::validateAttribute
//
// Validate the contents of the passed Attribute to see if they match up
// with what we expect
//
// Note: XML 1.0 2nd Ed Errata 20 changed the definition of
// Names and NmTokens to:-
//
// [6] Names ::= Name (#x20 Name)*
// [8] Nmtokens ::= Nmtoken (#x20 Nmtoken)*
//
//==============================================================================
void AttributeType::validateAttribute(const Attribute& attr, ParserImpl& parser) const
{
	QC_DBG_ASSERT(parser.m_features.m_bDoValidityChecks);

	const bool bReportWarnings = parser.m_features.m_bDoWarningChecks;

	const String& attrValue = attr.getValue();

	if(attrValue.empty())
	{
		if(m_eType != CDATA)
		{
			//
			// Only CDATA attributes are permitted to have an empty default value
			//
			const String& errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_ATTREMPTY,
				"{0} attribute '{1}' may not have an empty value"),
				getTypeAsString(),
				getName().getRawName());

			parser.errorDetected(Parser::Error, errMsg, EXML_IDATTRDEFAULT);
		}
		else //CDATA
		{
			//
			// Custom QC Warning: #REQUIRED attributes should not be empty
			//
			if(bReportWarnings && m_eDefaultType == REQUIRED)
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_REQDEMPTY,
					"attribute '{0}' is declared as #REQUIRED but is empty"),
					getName().getRawName());

				parser.errorDetected(Parser::Warning, errMsg, EXML_REQDEMPTY);
			}
		}
	}
	else // !empty
	{
		// For enumerated types, if the value matches one of the valid enums
		// then no further testing is required
		if(isEnumerated())
		{
			if(!containsEnum(attrValue))
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_ATTRNOTINENUMLIST,
					"attribute '{0}' has invalid value, must have a value from the list: {1}"),
					getName().getRawName(),
					getEnumListAsString());

				parser.errorDetected(Parser::Error, errMsg, EXML_ATTRNOTINENUMLIST);
			}
		}
		// ID attributes must be "names" and must be unique within 
		// the document
		else if(m_eType == ID)
		{
			if(testNmToken(false, true/*name*/, attrValue, parser))
			{
				if(!parser.addElementId(attrValue))
				{
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_ATTRIDUNIQ,
						"attribute '{0}' has an ID value of '{1}' which is not unique within the document"),
						getName().getRawName(),
						attr.getValue());

					parser.errorDetected(Parser::Error, errMsg, EXML_ATTRIDUNIQ);
				}
			}
		}
		else
		{
			StringTokenizer tokenizer(attrValue, sSpace);

			while(true)
			{
				const String& value = isTokenized() ? tokenizer.nextToken()
													: attrValue;

				// IDREF attributes must be "Names" and must reference IDs
				// within the document.  However, the ID may be later on in the
				// document so we just add it to a list of IDREFs which can be checked
				// at the end of the document
				if(m_eType == IDREF || m_eType == IDREFS)
				{
					if(testNmToken(false, true/*Name*/, value, parser))
					{
						parser.addElementIdRef(value);
					}
				}

				// NMTOKEN/NMTOKENS attributes must be "NmTokens"
				if(m_eType == NMTOKEN || m_eType == NMTOKENS)
				{
					testNmToken(false, false/*NmToken*/, value, parser);
				}

				// ENTITY attributes must be "Names" and must refer to
				// unparsed external entities
				if(m_eType == ENTITY || m_eType == ENTITIES)
				{
					if(testNmToken(false, true/*Name*/, value, parser))
					{
						testEntity(value, parser);
					}
				}

				if(!isTokenized() || !tokenizer.hasMoreTokens())
				{
					break;
				}
			}
		}
	}
	
	//
	// Validity Constraint: Fixed Attribute Default
	//
	if(m_eDefaultType == FIXED && attr.getValue() != m_defaultValue)
	{
		const String& errMsg = MessageFormatter::Format(
			System::GetSysMessage(sXML, EXML_ATTRFIXED,
			"attribute '{0}' with value '{1}' must equal the FIXED default value of '{2}'"),
			getName().getRawName(),
			attr.getValue(), m_defaultValue);

		parser.errorDetected(Parser::Error, errMsg, EXML_ATTRFIXED);
	}
}

//==============================================================================
// AttributeType::testNmTokens
//
//==============================================================================
bool AttributeType::testNmTokens(bool bDefault, bool bName, const String& value, ParserImpl& parser) const
{
	bool bRet = true;
	StringTokenizer tokenizer(value, sSpace);
	
	if(!tokenizer.hasMoreTokens())
	{
	}
	
	while(tokenizer.hasMoreTokens())
	{
		const String& token = tokenizer.nextToken();
		bRet = bRet && testNmToken(bDefault, bName, token, parser);
	}
	return bRet;
}

//==============================================================================
// AttributeType::testNmToken
//
//==============================================================================
bool AttributeType::testNmToken(bool /*bDefault*/, bool bName, const String& value, ParserImpl& parser) const
{
	if(!CharTypeFacet::IsValidName(value, bName))
	{
		static String sName = QC_T("Name");
		static String sNameTok = QC_T("Nmtoken");
		const String& type = bName ? sName : sNameTok;

		const String& errMsg = MessageFormatter::Format(
			System::GetSysMessage(sXML, EXML_ATTRNOTNMTOKEN,
			"value '{0}' for attribute '{1}' is not a valid {2}"),
			value,
			getName().getRawName(),
			type);

		parser.errorDetected(Parser::Error, errMsg, EXML_ATTRNOTNMTOKEN);

		return false;
	}
	else
		return true;
}

//==============================================================================
// AttributeType::isTokenized
//
// Return true if attributes of this type consist of a number of white-space
// separated tokens.
//==============================================================================
bool AttributeType::isTokenized() const
{
	return (m_eType == IDREFS || m_eType == ENTITIES || m_eType == NMTOKENS);
}

//==============================================================================
// AttributeType::isStringType
//
//==============================================================================
bool AttributeType::isStringType() const
{
	return (m_eType == CDATA); 
}

//==============================================================================
// AttributeType::GetTypeAsString
//
//==============================================================================
String AttributeType::GetTypeAsString(Type eType)
{
	String ret;

	switch (eType)
	{
	default:
	case UNKNOWN:
		ret=QC_T("UNKNOWN");  break;
	case CDATA:
		ret=QC_T("CDATA");    break;
	case ID:
		ret=QC_T("ID");       break;
	case IDREF:
		ret=QC_T("IDREF");    break;
	case IDREFS:
		ret=QC_T("IDREFS");   break;
	case ENTITY:
		ret=QC_T("ENTITY");   break;
	case ENTITIES:
		ret=QC_T("ENTITIES"); break;
	case NMTOKEN:
		ret=QC_T("NMTOKEN");  break;
	case NMTOKENS:
		ret=QC_T("NMTOKENS"); break;
	case NOTATION:
		ret=QC_T("NOTATION"); break; 
	case ENUMERATION:
		ret=QC_T("NMTOKEN");  break;
	}
	return ret;
}

//==============================================================================
// AttributeType::getEnumListAsString()
//
//==============================================================================
String AttributeType::getEnumListAsString() const
{
	String sRet;

	EnumSet::const_iterator enumIter;
	for(enumIter=m_enums.begin(); enumIter!=m_enums.end(); ++enumIter)
	{
		if(enumIter==m_enums.begin())
		{
			sRet = QC_T("(");
		}
		else
		{
			sRet += QC_T("|");
		}
		sRet += (*enumIter);
	}
	
	if(sRet.size())
	{
		sRet += QC_T(")");
	}

	return sRet;
}

//==============================================================================
// AttributeType::validate
//
// Perform post DTD validation.  Ie, validate information that is only
// available when the entire DTD has been processed.
//==============================================================================
void AttributeType::validate(ParserImpl& parser) const
{
	if(m_eType == NOTATION)
	{
		EnumSet::const_iterator enumIter;
		for(enumIter=m_enums.begin(); enumIter!=m_enums.end(); ++enumIter)
		{
			if(!parser.isNotationDeclared(*enumIter))
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_UNDECLNOTN,
					"undeclared notation '{0}' referenced in attribute '{1}'"),
					(*enumIter),
					getName().getRawName());
				parser.errorDetected(Parser::Error, errMsg, EXML_UNDECLNOTN);
			}
		}
	}
	else if(!m_defaultValue.empty() && (m_eType == ENTITIES || m_eType == ENTITY))
	{
		//
		// Use a StringTokenizer if multiple values can be provided
		// in the default value
		//
		StringTokenizer tokenizer(m_defaultValue);
		while(true)
		{
			const String& value = isTokenized() ? tokenizer.nextToken()
				                                : m_defaultValue;
			testEntity(value, parser);
			if(!isTokenized() || !tokenizer.hasMoreTokens())
			{
				break;
			}
		}
	}
}

//==============================================================================
// AttributeType::containsEnum
//
// Determine of the enum value has already been specified for this 
// AttributeType.
//
//==============================================================================
bool AttributeType::containsEnum(const String& value) const
{
	EnumSet::const_iterator iter = m_enums.find(value);
	return (iter != m_enums.end());
}

//==============================================================================
// AttributeType::testEntity
//
// For ENTITY and ENTITIES type attributes, ensure that the value refers
// to an un-parsed external entity.
//==============================================================================
bool AttributeType::testEntity(const String& value, ParserImpl& parser) const
{
	//
	// Validity Constraint: Entity Name
	//
	AutoPtr<Entity> rpEntity = parser.getEntity(value);
	if(rpEntity.isNull() || rpEntity->isParsed())
	{
		const String& errMsg = MessageFormatter::Format(
			System::GetSysMessage(sXML, EXML_ENTITYNOTUNPARSED,
			"entity '{0}' is not an unparsed entity declared in the DTD"),
			value);

		parser.errorDetected(Parser::Error, errMsg, EXML_ENTITYNOTUNPARSED);

		return false;
	}
	return true;
}

//==============================================================================
// AttributeType::getTypeAsString
//
//==============================================================================
String AttributeType::getTypeAsString() const
{
	return GetTypeAsString(m_eType);
}

QC_XML_NAMESPACE_END


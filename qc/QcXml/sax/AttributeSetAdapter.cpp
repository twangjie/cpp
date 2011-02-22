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

#include "AttributeSetAdapter.h"
#include "QcXml/xml/Attribute.h"

QC_SAX_NAMESPACE_BEGIN

using xml::Attribute;

const String sNull;

AttributeSetAdapter::AttributeSetAdapter(const AttributeSet& theSet) :
	m_theSet(theSet)
{
}


//==============================================================================
// AttributeSetAdapter::getIndex
//
// Look up the index of an attribute by XML 1.0 qualified name. 
//==============================================================================
int AttributeSetAdapter::getIndex(const String& qName) const
{
	AutoPtr<Attribute> rpAddr = m_theSet.getAttribute(qName);
	if(rpAddr)
		return rpAddr->getIndex();
	else
		return -1;
}


//==============================================================================
// AttributeSetAdapter::getIndex
//
// Look up the index of an attribute by Namespace name. 
//==============================================================================
int AttributeSetAdapter::getIndex(const String& uri, const String& localPart) const
{
	AutoPtr<Attribute> rpAddr = m_theSet.getAttribute(uri, localPart);
	if(rpAddr)
		return rpAddr->getIndex();
	else
		return -1;
}


//==============================================================================
// AttributeSetAdapter::getLength
//
// Return the number of AttributesImpl in the list. 
//==============================================================================
size_t AttributeSetAdapter::getLength() const
{
	return m_theSet.size();
}


//==============================================================================
// AttributeSetAdapter::getLocalName
//
// Look up an attribute's local name by index. 
//==============================================================================
String AttributeSetAdapter::getLocalName(size_t index) const
{
	AutoPtr<Attribute> rpAttr = m_theSet.getAttribute(index);
	if(rpAttr)
	{
		return rpAttr->getName().getLocalName();
	}
	else
	{
		return sNull;
	}
}

//==============================================================================
// AttributeSetAdapter::getQName
//
// Look up an attribute's XML 1.0 qualified name by index. 
//==============================================================================
String AttributeSetAdapter::getQName(size_t index) const
{
	AutoPtr<Attribute> rpAttr = m_theSet.getAttribute(index);
	if(rpAttr)
	{
		return rpAttr->getName().getRawName();
	}
	else
	{
		return sNull;
	}
}

//==============================================================================
// AttributeSetAdapter::getType
//
// Look up an attribute's type by index. 
//==============================================================================
String AttributeSetAdapter::getType(size_t index) const
{
	AutoPtr<Attribute> rpAttr = m_theSet.getAttribute(index);
	if(rpAttr)
	{
		return rpAttr->getType();
	}
	else
	{
		return sNull;
	}
}
  

//==============================================================================
// AttributeSetAdapter::getType
//
// Look up an attribute's type by XML 1.0 qualified name. 
//==============================================================================
String AttributeSetAdapter::getType(const String& qName) const
{
	AutoPtr<Attribute> rpAttr = m_theSet.getAttribute(qName);
	if(rpAttr)
	{
		return rpAttr->getType();
	}
	else
	{
		return sNull;
	}
}


//==============================================================================
// AttributeSetAdapter::getType
//
// Look up an attribute's type by Namespace name. 
//==============================================================================
String AttributeSetAdapter::getType(const String& uri, const String& localName) const
{
	AutoPtr<Attribute> rpAttr = m_theSet.getAttribute(uri, localName);
	if(rpAttr)
	{
		return rpAttr->getType();
	}
	else
	{
		return sNull;
	}
}


//==============================================================================
// AttributeSetAdapter::getURI
//
// Look up an attribute's Namespace URI by index. 
//============================================================================== 
String AttributeSetAdapter::getURI(size_t index) const
{
	AutoPtr<Attribute> rpAttr = m_theSet.getAttribute(index);
	if(rpAttr)
	{
		return rpAttr->getName().getNamespaceURI();
	}
	else
	{
		return sNull;
	}
}


//==============================================================================
// AttributeSetAdapter::getValue
//
// Look up an attribute's value by index. 
//==============================================================================
String AttributeSetAdapter::getValue(size_t index) const
{
	AutoPtr<Attribute> rpAttr = m_theSet.getAttribute(index);
	if(rpAttr)
	{
		return rpAttr->getValue();
	}
	else
	{
		return sNull;
	}
}


//==============================================================================
// AttributeSetAdapter::getValue
//
// Look up an attribute's value by XML 1.0 qualified name. 
//==============================================================================
String AttributeSetAdapter::getValue(const String& qName) const
{
	AutoPtr<Attribute> rpAttr = m_theSet.getAttribute(qName);
	if(rpAttr)
	{
		return rpAttr->getValue();
	}
	else
	{
		return sNull;
	}
}

//==============================================================================
// AttributeSetAdapter::getValue
//
// Look up an attribute's value by Namespace name. 
//==============================================================================
String AttributeSetAdapter::getValue(const String& uri, const String& localName) const
{
	AutoPtr<Attribute> rpAttr = m_theSet.getAttribute(uri, localName);
	if(rpAttr)
	{
		return rpAttr->getValue();
	}
	else
	{
		return sNull;
	}
}

QC_SAX_NAMESPACE_END

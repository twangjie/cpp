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
// Class AttributesImpl
//
/**
	@class qc::sax::AttributesImpl
	
	@brief Default implementation of the SAX2 Attributes interface with
	the addition of manipulators so that the list can be modified or reused.

	There are two typical uses of this class:-
	-# to take a persistent snapshot of an Attributes object in a
	   startElement event, or;
	-# to construct or modify an Attributes object
	   in a SAX2 driver or filter.

	This class differs slightly from the official SAX 2.0 distribution in that 
	it omits certain methods or parameters that either deal with both local names
	and qualified names or that attempt to set the value of a local name in
	isolation.  Local names are not well defined in SAX 2.0, which is confused 
	further in this class because an AttributesImpl exists outside the scope of
	an XMLReader.  For this reason the QuickCPP designers decided to simplify matters
	by always avoiding conflicts between local and qualified names - which QuickCPP
	treats as the same thing when XML namespace processing is disabled.  

	@since 1.1
*/
//==============================================================================

#include "AttributesImpl.h"

#include "QcCore/base/IllegalArgumentException.h"
#include "QcXml/xml/QName.h"
#include "QcXml/xml/Attribute.h"

QC_SAX_NAMESPACE_BEGIN

using xml::Attribute;
using xml::QName;

//==============================================================================
// AttributesImpl::AttributesImpl
//
/**
   Default constructor.
*/
//==============================================================================
AttributesImpl::AttributesImpl()
{
}

//==============================================================================
// AttributesImpl::AttributesImpl
//
/**
   Copies an existing Attributes object
*/
//==============================================================================
AttributesImpl::AttributesImpl(const Attributes& atts)
{
	setAttributes(atts);
}

//==============================================================================
// AttributesImpl::addAttribute
//
/**
   Add a new attribute to the end of the collection.

   For the sake of speed, this method does no checking for 
   well-formedness: such checks are the responsibility of the application.

   <p>Note, for convenience to QuickCPP users, this method differs from
   the standard SAX 2 distribution in two ways:-</p>

   -# The method does not accepts a localName.  The qName parameter
      should be used for the attribute name, regardless of whether or 
      not namespace processing is enabled.
   -# An IllegalArgumentException is thrown if an attribute with the same name
      already exists in the collection.

   @param uri the namespace URI, or the empty string if none is
          available or namespace processing is not being performed.
   @param qName the qualified (prefixed) name of the attribute.  The name
          is not required to contain a prefix, but if it does the @c uri 
          parameter should also be provided when namespaces are in effect.
   @param type the attribute type as a string
   @param value the attribute value
   @throws IllegalArgumentException if the attribute already exists in the
           collection
*/
//==============================================================================
void AttributesImpl::addAttribute(const String& uri,
                                  const String& qName,
                                  const String& type,
                                  const String& value)
{
	const QName theQName(qName, uri);

	// Use a AutoPtr<> to ensure no memory leaks if we fail to add the Attribute
	AutoPtr<Attribute> rpAttr = new Attribute(theQName, value, type);
	if(!m_attributes.addAttribute(rpAttr.get()))
	{
		throw IllegalArgumentException(QC_T("attribute already exists"));
	}
}

//==============================================================================
// AttributesImpl::clear
//
/**
   Clear the attributes collection for reuse.
*/
//==============================================================================
void AttributesImpl::clear()
{
	m_attributes.removeAll();
}

//==============================================================================
// AttributesImpl::removeAttribute
//
/**
   Removes an attribute from the collection.

   @param index the 0-based index of the attribute to be removed
   @throws IllegalArgumentException if index specifies
           an attribute which is not in the collection
*/
//==============================================================================
void AttributesImpl::removeAttribute(size_t index)
{
	if(index < m_attributes.size())
		m_attributes.removeAttribute(index);
	else
		throw IllegalArgumentException(QC_T("index out of bounds"));
}

//==============================================================================
// AttributesImpl::setAttribute
//
/**
   Sets an attribute in the collection.

   <p>For the sake of speed, this method does no checking for name
   conflicts or well-formedness: such checks are
   the responsibility of the application.</p>

   <p>Note, for convenience to QuickCPP users, this method differs from
   the standard SAX 2 distribution in that it does not accepts a localName.
   The qName parameter should be used for the attribute name, regardless of
   whether or not namespace processing is enabled.</p>

   @param index the 0-based index of the item to be modified
   @param uri the namespace URI, or the empty string if none is
          available or namespace processing is not being performed.
   @param qName the qualified (prefixed) name of the attribute.  The name
          is not required to contain a prefix, but if it does the @c uri 
          parameter should also be provided when namespaces are in effect.
   @param type the attribute type as a string
   @param value the attribute value
   @throws IllegalArgumentException if index is larger than the number of entries
           in the collection.
*/
//==============================================================================
void AttributesImpl::setAttribute(size_t index,
                                  const String& uri,
                                  const String& qName,
                                  const String& type,
                                  const String& value)
{
	const AutoPtr<Attribute>& rpAttr = m_attributes.getAttribute(index);
	if(!rpAttr)
	{
		throw IllegalArgumentException(QC_T("index out of bounds"));
	}
	else
	{
		rpAttr->setName(QName(qName, uri));
		rpAttr->setType(type);
		rpAttr->setValue(value);
	}
}

//==============================================================================
// AttributesImpl::setAttributes
//
/**
   Copies an Attributes collection.

   @param atts the attributes to copy
   @throws IllegalArgumentException if the passed collection contains a 
           duplicate attribute.
*/
//==============================================================================
void AttributesImpl::setAttributes(const Attributes& atts)
{
	//
	// First, remove all entries from this collection
	//
	clear();

	//
	// Then iterate through the passed Attributes collection, creating
	// a new Attribute for each one and adding it to our collection
	//
	for(size_t i = 0; i < atts.getLength(); ++i)
	{
		AutoPtr<Attribute> rpAttr = new Attribute(atts.getQName(i),
		                                         atts.getValue(i),
		                                         atts.getType(i));

		if(!m_attributes.addAttribute(rpAttr.get()))
		{
			throw IllegalArgumentException(QC_T("attribute already exists"));
		}
	}
}

//==============================================================================
// AttributesImpl::setQName
//
/**
   Sets the qualified name of a specified attribute.

   @param index the 0-based index of the attribute to be modified
   @param qName the new qualified name for the attribute
   @throws IllegalArgumentException if index is larger than the number of entries
           in the collection.
*/
//==============================================================================
void AttributesImpl::setQName(size_t index, const String& qName)
{
	const AutoPtr<Attribute>& rpAttr = m_attributes.getAttribute(index);
	if(!rpAttr)
	{
		throw IllegalArgumentException(QC_T("index out of bounds"));
	}
	rpAttr->setName(qName);
}

//==============================================================================
// AttributesImpl::setType
//
/**
   Sets the type of a specified attribute.

   @param index the 0-based index of the attribute to be modified
   @param type the new type for the attribute
   @throws IllegalArgumentException if index is larger than the number of entries
           in the collection.
*/
//==============================================================================
void AttributesImpl::setType(size_t index, const String& type)
{
	const AutoPtr<Attribute>& rpAttr = m_attributes.getAttribute(index);
	if(!rpAttr)
	{
		throw IllegalArgumentException(QC_T("index out of bounds"));
	}
	rpAttr->setType(type);
}

//==============================================================================
// AttributesImpl::setURI
//
/**
   Sets the namespace URI of a specified attribute.

   @param index the 0-based index of the attribute to be modified
   @param uri the new uri for the attribute
   @throws IllegalArgumentException if index is larger than the number of entries
           in the collection.
*/
//==============================================================================
void AttributesImpl::setURI(size_t index, const String& uri)
{
	const AutoPtr<Attribute>& rpAttr = m_attributes.getAttribute(index);
	if(!rpAttr)
	{
		throw IllegalArgumentException(QC_T("index out of bounds"));
	}
	const QName newqName(rpAttr->getName().getRawName(), uri);
	rpAttr->setName(newqName);
}

//==============================================================================
// AttributesImpl::setValue
//
/**
   Sets the value of a specified attribute.

   @param index the 0-based index of the attribute to be modified
   @param value the new value for the attribute
   @throws IllegalArgumentException if index is larger than the number of entries
           in the collection.
*/
//==============================================================================
void AttributesImpl::setValue(size_t index, const String& value)
{
	const AutoPtr<Attribute>& rpAttr = m_attributes.getAttribute(index);
	if(!rpAttr)
	{
		throw IllegalArgumentException(QC_T("index out of bounds"));
	}
	rpAttr->setValue(value);
}

//==============================================================================
// AttributesImpl::getIndex
//
/**
   Looks up an attribute's index by qualified (prefixed) name.

   @param qName the qualified name to look for
   @returns the attribute's index in the collection, or -1 if the qName
            was not found
*/
//==============================================================================
int AttributesImpl::getIndex(const String& qName) const
{
	//
	// Because xml::AttributeSet does not return an attribute's index position,
	// we are forced to iterate through the collection looking for a match.
	//
	const AutoPtr<Attribute>& rpAttr = m_attributes.getAttribute(qName);

	if(rpAttr)
	{
		for(size_t i=0; i<m_attributes.size(); ++i)
		{
			const AutoPtr<Attribute>& rpIndexedAttr = m_attributes.getAttribute(i);
			if(rpIndexedAttr == rpAttr)
			{
				return i;
			}
		}
	}
	return -1;
}

//==============================================================================
// AttributesImpl::getIndex
//
/**
   Looks up an attribute's index by namespace uri and local name.

   @param uri the uri for the attribute to search for
   @param localName the local name for the attribute to search for
   @returns the attribute's index in the collection, or -1 if no
            matching attribute could be found
*/
//==============================================================================
int AttributesImpl::getIndex(const String& uri, const String& localName) const
{
	//
	// Because xml::AttributeSet does not return an attribute's index position,
	// we are forced to iterate through the collection looking for a match.
	//
	const AutoPtr<Attribute>& rpAttr = m_attributes.getAttribute(uri, localName);

	if(rpAttr)
	{
		for(size_t i=0; i<m_attributes.size(); ++i)
		{
			const AutoPtr<Attribute>& rpIndexedAttr = m_attributes.getAttribute(i);
			if(rpIndexedAttr == rpAttr)
			{
				return i;
			}
		}
	}
	return -1;
}

//==============================================================================
// AttributesImpl::getLength
//
/**
   Returns the number of attributes in the collection.
*/
//==============================================================================
size_t AttributesImpl::getLength() const
{
	return m_attributes.size();
}

//==============================================================================
// AttributesImpl::getLocalName
//
/**
   Returns the local name for the specified attribute.

   @param index the 0-based index of the attribute whose local name is
          required
   @returns the local name of the specified attribute or the empty
            string if none is available or if the index is out of range
*/
//==============================================================================
String AttributesImpl::getLocalName(size_t index) const
{
	const AutoPtr<Attribute>& rpAttr = m_attributes.getAttribute(index);
	if(rpAttr)
	{
		return rpAttr->getName().getLocalName();
	}
	else
	{
		return String();
	}
}

//==============================================================================
// AttributesImpl::getQName
//
/**
   Returns the qualified (prefixed) name for the specified attribute.

   @param index the 0-based index of the attribute whose qualified name is
          required
   @returns the qualified name of the specified attribute or the empty
            string if none is available or if the index is out of range
*/
//==============================================================================
String AttributesImpl::getQName(size_t index) const
{
	const AutoPtr<Attribute>& rpAttr = m_attributes.getAttribute(index);
	if(rpAttr)
	{
		return rpAttr->getName().getRawName();
	}
	else
	{
		return String();
	}
}

//==============================================================================
// AttributesImpl::getType
//
/**
   Returns the type for the specified attribute.

   @param index the 0-based index of the attribute whose type is
          required
   @returns the type of the specified attribute, "CDATA"
            if the type is unknown or the empty string if the index is
            out of range
*/
//==============================================================================
String AttributesImpl::getType(size_t index) const
{
	const AutoPtr<Attribute>& rpAttr = m_attributes.getAttribute(index);
	if(rpAttr)
	{
		String retStr = rpAttr->getType();
		if(retStr.empty())
		{
			return QC_T("CDATA");
		}
		else
		{
			return retStr;
		}
	}
	else
	{
		return String();
	}
}

//==============================================================================
// AttributesImpl::getType
//
/**
   Returns the type for the specified attribute.

   @param qName the qualified name of the attribute whose type is
          required
   @returns the type of the specified attribute, "CDATA"
            if the type is unknown of the empty string if the attribute could
            not be found.
*/
//==============================================================================
String AttributesImpl::getType(const String& qName) const
{
	const AutoPtr<Attribute>& rpAttr = m_attributes.getAttribute(qName);
	if(rpAttr)
	{
		String retStr = rpAttr->getType();
		if(retStr.empty())
		{
			return QC_T("CDATA");
		}
		else
		{
			return retStr;
		}
	}
	else
	{
		return String();
	}
}

//==============================================================================
// AttributesImpl::getType
//
/**
   Returns the type for the specified attribute.

   @param uri The attribute's Namespace URI, or the empty string if none
          is available.
   @param localName The attribute's local name
   @returns the type of the specified attribute, "CDATA"
            if the type is unknown of the empty string if the attribute could
            not be found.
*/
//==============================================================================
String AttributesImpl::getType(const String& uri, const String& localName) const
{
	const AutoPtr<Attribute>& rpAttr = m_attributes.getAttribute(uri, localName);
	if(rpAttr)
	{
		String retStr = rpAttr->getType();
		if(retStr.empty())
		{
			return QC_T("CDATA");
		}
		else
		{
			return retStr;
		}
	}
	else
	{
		return String();
	}
}

//==============================================================================
// AttributesImpl::getURI
//
/**
   Returns the namespace URI for the specified attribute.

   @param index the 0-based index of the attribute whose URI is
          required
   @returns the URI of the specified attribute, or the empty string if
            none is available or if the index is out of range
*/
//==============================================================================
String AttributesImpl::getURI(size_t index) const
{
	const AutoPtr<Attribute>& rpAttr = m_attributes.getAttribute(index);
	if(rpAttr)
	{
		return rpAttr->getName().getNamespaceURI();
	}
	else
	{
		return String();
	}
}

//==============================================================================
// AttributesImpl::getValue
//
/**
   Returns the value for the specified attribute.

   @param index the 0-based index of the attribute whose value is
          required
   @returns the value of the specified attribute, or the empty string if
            none is available or if the index is out of range
*/
//==============================================================================
String AttributesImpl::getValue(size_t index) const
{
	const AutoPtr<Attribute>& rpAttr = m_attributes.getAttribute(index);
	if(rpAttr)
	{
		return rpAttr->getValue();
	}
	else
	{
		return String();
	}
}

//==============================================================================
// AttributesImpl::getValue
//
/**
   Returns the value for the specified attribute.

   @param qName the qualified name of the attribute whose value is
          required
   @returns the value of the specified attribute, or the empty string if
            none is available or if the attribute could not be found
*/
//==============================================================================
String AttributesImpl::getValue(const String& qName) const
{
	const AutoPtr<Attribute>& rpAttr = m_attributes.getAttribute(qName);
	if(rpAttr)
	{
		return rpAttr->getValue();
	}
	else
	{
		return String();
	}
}

//==============================================================================
// AttributesImpl::getValue
//
/**
   Returns the value for the specified attribute.

   @param uri The attribute's Namespace URI, or the empty string if none
          is available.
   @param localName The attribute's local name
   @returns the value of the specified attribute or the empty string
            if the attribute could not be found.
*/
//==============================================================================
String AttributesImpl::getValue(const String& uri, const String& localName) const
{
	const AutoPtr<Attribute>& rpAttr = m_attributes.getAttribute(uri, localName);
	if(rpAttr)
	{
		return rpAttr->getValue();
	}
	else
	{
		return String();
	}
}

QC_SAX_NAMESPACE_END

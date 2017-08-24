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

#include "AttributeSetImpl.h"

#include "QcCore/base/debug.h"
#include "QcCore/base/NullPointerException.h"

QC_XML_NAMESPACE_BEGIN

AutoPtr<Attribute> AttributeSetImpl::getAttribute(const String& uri, const String& localName) const
{
	for(AttributeVector::const_iterator iter = m_attributes.begin();
	    iter != m_attributes.end(); ++iter)
	{
		const QName& name = (*iter)->getName();
		if(uri == name.getNamespaceURI() && localName == name.getLocalName())
		{
			return (*iter);
		}
	}
	return 0;
}

AutoPtr<Attribute> AttributeSetImpl::getAttribute(const String& rawName) const
{
	for(AttributeVector::const_iterator iter = m_attributes.begin();
	    iter != m_attributes.end(); ++iter)
	{
		const QName& name = (*iter)->getName();
		if(rawName == name.getRawName())
		{
			return (*iter);
		}
	}
	return 0;
}

AutoPtr<Attribute> AttributeSetImpl::getAttribute(size_t index) const
{
	if(index >= m_attributes.size())
	{
		return 0;
	}
	else
	{
		return m_attributes[index];
	}
}

bool AttributeSetImpl::empty() const
{
	return (m_attributes.size() == 0);
}

size_t AttributeSetImpl::size() const
{
	return m_attributes.size();
}

bool AttributeSetImpl::addAttribute(Attribute* pAttribute)
{
	if(!pAttribute) throw NullPointerException();

	const String& qName = pAttribute->getName().getRawName();
	if(!getAttribute(qName))
	{
		m_attributes.push_back(pAttribute);
		return true;
	}
	else
	{
		return false;
	}
}

void AttributeSetImpl::removeAttribute(size_t index)
{
	if(index < m_attributes.size())
	{
		AttributeVector::iterator i = m_attributes.begin() + index;
		m_attributes.erase(i);
	}
}

void AttributeSetImpl::removeAll()
{
	m_attributes.clear();
}

QC_XML_NAMESPACE_END

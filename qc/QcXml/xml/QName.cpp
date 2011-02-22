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

#include "QName.h"

QC_XML_NAMESPACE_BEGIN

const String sEmpty;


bool QName::operator==(const QName& rhs) const
{
	return (m_rawName == rhs.m_rawName);
}

bool QName::operator!=(const QName& rhs) const
{
	return (m_rawName != rhs.m_rawName);
}

//==============================================================================
// QName::::operator<()
//
// Used for sorting QNames lexicographically.
// The XML Canonicalization recommendation suggests that attributes
// short be sorted, using the namespace uri as the primary key and the local
// name as the secondary key.
//==============================================================================
bool QName::operator<(const QName& rhs) const
{
	return (m_namespaceURI < rhs.m_namespaceURI ||
	       (m_namespaceURI == rhs.m_namespaceURI && getLocalName() < rhs.getLocalName()));
}

const String& QName::getRawName() const
{
	return m_rawName;
}

String QName::getPrefix() const
{
	if(m_delimPosition != String::npos)
	{
		return m_rawName.substr(0, m_delimPosition);
	}
	else
	{
		return sEmpty;
	}
}

String QName::getLocalName() const
{
	if(m_delimPosition != String::npos)
		return m_rawName.substr(m_delimPosition+1, (m_rawName.size()-(m_delimPosition+1)));
	else
		return m_rawName;
}

const String& QName::getNamespaceURI() const
{
	return m_namespaceURI;
}

size_t QName::getDelimPosition() const
{
	return m_delimPosition;
}

void QName::setNamespaceURI(const String& namespaceURI)
{
	m_namespaceURI = namespaceURI;
}

void QName::setRawName(const String& rawName)
{
	m_rawName = rawName;
	setDelimPosition();
}

//==============================================================================
// QName::getUniversalName
//
// This is an attempt at creating a "universal name" ie a name representing
// the namespace and localname as a single name.  No standard exists for how
// this should be represented so we adopt the convention used by John Cowan's
// Namespace XML Filter which uses a a caret (^) to seperate the namespace URI
// from the local name.
//
// To deal with special cases like xml:space, when the URI is empty we revert to
// using the prefix ":" localname notation.
//==============================================================================
String QName::getUniversalName() const
{
	if(!m_namespaceURI.empty())
		return m_namespaceURI + String(QC_T("^")) + getLocalName();
	else
		return m_rawName;
}

QC_XML_NAMESPACE_END


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
// Class: QName
// 
// Overview
// --------
// A class to encapsulate a qualified XML name.
// 
// Qualified names are not part of the XML 1.0 standard - they were introduced
// as part of the XML Namespaces recommendation.
//
// Namespace processing is optional within the XML parser.  However, the QName
// class can still be used to represent raw XML 1.0 names.  In the case where
// namespace processing is switched off, the rawName element can be used.
// 
//==============================================================================

#ifndef QC_XML_QName_h
#define QC_XML_QName_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

QC_XML_NAMESPACE_BEGIN

class QC_XML_PKG QName
{
public:
	
	QName(const String& rawName,
	      const String& namespaceURI);

	QName(const String& rawName);
	
	QName(const QName& rhs);

	QName();

	bool operator==(const QName& rhs) const;
	bool operator!=(const QName& rhs) const;
	bool operator<(const QName& rhs) const; // used for sorting lexicographically

	const String& getRawName() const;
	String getPrefix() const;
	String getLocalName() const;
	const String& getNamespaceURI() const;
	size_t getDelimPosition() const;

	String getUniversalName() const;

	void setRawName(const String& rawName);
	void setNamespaceURI(const String& namespaceURI);

protected:
	void setDelimPosition();

private:
	String m_rawName;
	String m_namespaceURI;
	size_t m_delimPosition;
};

inline QName::QName() : m_delimPosition(String::npos)
{
}

inline QName::QName(const String& rawName, const String& namespaceURI) :
	m_rawName(rawName),
	m_namespaceURI(namespaceURI)
{
	setDelimPosition();
}

inline QName::QName(const String& rawName) :
	m_rawName(rawName)
{
	setDelimPosition();
}

inline QName::QName(const QName& rhs) :
	m_rawName(rhs.m_rawName),
	m_namespaceURI(rhs.m_namespaceURI),
	m_delimPosition(rhs.m_delimPosition)
{
}

inline void QName::setDelimPosition()
{
	m_delimPosition = m_rawName.find(':');
}

QC_XML_NAMESPACE_END

#endif //QC_XML_QName_h


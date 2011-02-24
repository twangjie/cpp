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
// Class: Attribute
// 
// Overview
// --------
// A class to encapsulate and store information about an attribute from an
// element start tag.
//
//==============================================================================

#ifndef QC_XML_Attribute_h
#define QC_XML_Attribute_h

#include "defs.h"
#include "QName.h"

QC_XML_NAMESPACE_BEGIN

class QC_XML_PKG Attribute : public virtual QCObject
{
	friend class ParserImpl;

public:

	// Create a parsed attribute
	Attribute(const QName& name,
			  const String& type,
			  int nIndex);

	// Create a "defaulted" attribute
	Attribute(const QName& name,
	          const String& value,
			  const String& type);

	// Default constructor - should never be used but std::map may require it's definition
	Attribute();

	Attribute(const Attribute& rhs);

	int getIndex() const;
	const QName& getName() const;
	const String& getValue() const;
	bool isDefaulted() const;
	const String& getType() const;

	void setName(const QName& name);
	void setValue(const String& value);
	void setType(const String& type);

private:
	QName m_name;
	String m_value;
	String m_type;
	int m_nIndex;
};

QC_XML_NAMESPACE_END

#endif //QC_XML_Attribute_h


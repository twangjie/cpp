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
// Class: AttributeSet
// 
// Overview
// --------
// A virtual base (interface) class to encapsulate and provide convenient
// access to the set of Attributes from an element start tag.
//
// Note: We elected to expose the Attribute Class rather than returning
// String values for two reasons:
// 1) Implementations may want to create their own Attribute derivative
// 2) It allows us to retun null when no matching attribute exists.
//
//
// Implementations of this interface have a difficult size/speed trade-off
// to make.  The SAX interface requires direct access to an attribute
// via an index, indicating the use of a std::vector.  However, it also 
// requires direct access via a name, or a name/uri pair indicating use of
// a map.
//
// Some applications also expect attributes to be sorted, and well-formedness
// requires that attributes are unique.
//
// For all these reasons the collection of attributes has been encapsulated
// behind this general-purpose interface.
//
//==============================================================================

#ifndef QC_XML_AttributeSet_h
#define QC_XML_AttributeSet_h

#include "Attribute.h"

QC_XML_NAMESPACE_BEGIN

class QC_XML_PKG AttributeSet
{
public:
	virtual ~AttributeSet() {};

	virtual AutoPtr<Attribute> getAttribute(const String& uri, const String& localName) const = 0;
	virtual AutoPtr<Attribute> getAttribute(const String& qName) const = 0;
	virtual AutoPtr<Attribute> getAttribute(size_t index) const = 0;
	virtual bool addAttribute(Attribute* pAttribute) = 0;
	virtual void removeAttribute(size_t index) = 0;
	virtual void removeAll() = 0;
	
	virtual bool empty() const = 0;
	virtual size_t size() const = 0;
};

QC_XML_NAMESPACE_END

#endif //QC_XML_AttributeSet_h


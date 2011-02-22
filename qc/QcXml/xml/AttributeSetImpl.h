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
// Class: AttributeSetImpl
// 
// Overview
// --------
// An implementation of the AttributeSet interface.
//
//==============================================================================

#ifndef QC_XML_AttributeSetImpl_h
#define QC_XML_AttributeSetImpl_h

#include "AttributeSet.h"
#include "Attribute.h"
#include <vector>

QC_XML_NAMESPACE_BEGIN

class QC_XML_PKG AttributeSetImpl : public AttributeSet
{
public:

	// AttributeSet methods
	virtual AutoPtr<Attribute> getAttribute(const String& uri, const String& localName) const;
	virtual AutoPtr<Attribute> getAttribute(const String& qName) const;
	virtual AutoPtr<Attribute> getAttribute(size_t index) const;
	virtual bool addAttribute(Attribute* pAttribute);
	virtual void removeAttribute(size_t index);
	virtual void removeAll();
	
	virtual bool empty() const;
	virtual size_t size() const;

private:
	typedef std::vector<AutoPtr<Attribute> > AttributeVector;
	AttributeVector m_attributes;
};

QC_XML_NAMESPACE_END

#endif //QC_XML_AttributeSetImpl_h


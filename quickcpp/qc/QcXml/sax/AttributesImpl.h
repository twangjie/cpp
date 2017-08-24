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

#ifndef QC_SAX_AttributesImpl_h
#define QC_SAX_AttributesImpl_h

#include "defs.h"
#include "Attributes.h"

#include "QcXml/xml/AttributeSetImpl.h"

QC_SAX_NAMESPACE_BEGIN

using xml::AttributeSetImpl;

class QC_SAX_PKG AttributesImpl : public Attributes
{
public:
	AttributesImpl();
	AttributesImpl(const Attributes& atts);

	void addAttribute(const String& uri,
	                  const String& qName,
	                  const String& type,
	                  const String& value);

	void clear();

	void removeAttribute(size_t index);
	void setAttribute(size_t index, const String& uri,
	                  const String& qName,
	                  const String& type, const String& value);

	void setAttributes(const Attributes& atts);
	void setQName(size_t index, const String& qName);
	void setType(size_t index, const String& type);
	void setURI(size_t index, const String& uri);
	void setValue(size_t index, const String& value);


	// implementation of Attributes interface
	virtual int getIndex(const String& qName) const;
	virtual int getIndex(const String& uri,
	                     const String& localName) const;

	virtual size_t getLength() const;
	virtual String getLocalName(size_t index) const;
	virtual String getQName(size_t index) const;
	virtual String getType(size_t index) const;
	virtual String getType(const String& qName) const;
	virtual String getType(const String& uri,
	                       const String& localName) const;

	virtual String getURI(size_t index) const;
	virtual String getValue(size_t index) const;
	virtual String getValue(const String& qName) const;
	virtual String getValue(const String& uri,
	                        const String& localName) const;

private:
	AttributeSetImpl m_attributes;
};

QC_SAX_NAMESPACE_END

#endif //QC_SAX_AttributesImpl_h

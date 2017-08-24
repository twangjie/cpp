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

#ifndef QC_UTIL_AttributeListParser_h
#define QC_UTIL_AttributeListParser_h

#ifndef QC_UTIL_DEFS_h
#include "defs.h"
#endif //QC_UTIL_DEFS_h

#include <deque>

QC_UTIL_NAMESPACE_BEGIN

class QC_UTIL_PKG AttributeListParser
{
public:

	bool parseString(const String& in);

	bool containsAttribute(const String& name) const;
	String getAttributeValue(const String& name) const;
	String getAttributeValueICase(const String& name) const;
	String getAttributeName(size_t index) const;
	String getAttributeValue(size_t index) const;
	size_t getAttributeCount() const;
	void removeAttribute(const String& name);

private:
	typedef std::deque<std::pair<String, String> > AttributeList;
	AttributeList m_list;
};

QC_UTIL_NAMESPACE_END

#endif //QC_UTIL_AttributeListParser_h


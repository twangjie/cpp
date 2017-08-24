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

#ifndef QC_SAX_NamespaceSupport_h
#define QC_SAX_NamespaceSupport_h

#include "defs.h"

#include <list>
#include <deque> // reqd for gcc 2.96
#include <map>
#include <stack>

QC_SAX_NAMESPACE_BEGIN 

class QC_SAX_PKG NamespaceSupport : public QCObject
{
public:

	NamespaceSupport();

	typedef std::list<String> PrefixList;
 
	bool declarePrefix(const String& prefix, const String& uri);
	PrefixList getDeclaredPrefixes() const;
	String getPrefix(const String& uri) const;
	PrefixList getPrefixes() const;
	PrefixList getPrefixes(const String& uri) const;
	String getURI(const String& prefix) const;
	void popContext();
	bool processName(const String& qName, bool bAttribute,
	                 String& retUri, String& retLocal) const;
	void pushContext();
	void reset();

	static const String XMLNS;

private:

	void init();

private:

	struct NamespaceContext
	{
		typedef std::map<String, String, std::less<String> > PrefixMap;

		String m_defaultURI;
		PrefixMap m_prefixMap;
		PrefixList m_deltaPrefixList;
	};

	typedef std::stack<NamespaceContext, std::deque<NamespaceContext> > NamespaceContextStack;
	NamespaceContextStack m_namespaceContextStack;
};

QC_SAX_NAMESPACE_END

#endif //QC_SAX_NamespaceSupport_h

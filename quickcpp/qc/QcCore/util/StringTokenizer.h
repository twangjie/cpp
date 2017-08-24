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

#ifndef QC_UTIL_StringTokenizer_h
#define QC_UTIL_StringTokenizer_h

#ifndef QC_UTIL_DEFS_h
#include "defs.h"
#endif //QC_UTIL_DEFS_h

#include <vector>

QC_UTIL_NAMESPACE_BEGIN

class QC_UTIL_PKG StringTokenizer
{
public:
	
	StringTokenizer(const String& str);
	StringTokenizer(const String& str, const String& delim,
	                bool bReturnDelims=false, bool bReturnContiguousDelims=true);
	String nextToken();
	String peekNextToken() const;
	bool hasMoreTokens() const;
	std::vector<String> toVector();

private:
	String::size_type locateNextToken(bool bDelim, String::size_type pos) const;

private:
	bool m_bReturnDelims;
	bool m_bReturnContiguousDelims;
	bool m_bContainsMultiCharSequence;
	String m_delim;
	String::size_type m_nextPos;
	String m_str;
};

QC_UTIL_NAMESPACE_END

#endif //QC_UTIL_StringTokenizer_h


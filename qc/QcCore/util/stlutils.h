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
// QC/STL utility functions
//
//==============================================================================

#ifndef QC_UTIL_stlutils_h
#define QC_UTIL_stlutils_h

#ifndef QC_UTIL_DEFS_h
#include "defs.h"
#endif //QC_UTIL_DEFS_h

#include <algorithm>

QC_UTIL_NAMESPACE_BEGIN

template <typename Iter>
inline
void deleteAll(Iter first, Iter last)
{
	while(first != last)
	{
		delete (*first);
		++first;
	}
}

template<class Iter1, class Iter2>
inline
Iter1 find_first_not_of(Iter1 i1, Iter1 end1, Iter2 i2, Iter2 end2)
{
	for (; i1 != end1; ++i1)
		if(std::find(i2, end2, *i1) == end2)
			return (i1);
	return (end1);
}

QC_UTIL_NAMESPACE_END

#endif //QC_UTIL_stlutils_h

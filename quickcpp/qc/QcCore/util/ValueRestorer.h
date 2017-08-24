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
// Class: ValueRestorer
//
// This is a simple class that takes a copy of a variable and restores its
// value on destruction.
//
//==============================================================================

#ifndef QC_UTIL_ValueRestorer_h
#define QC_UTIL_ValueRestorer_h

#ifndef QC_UTIL_DEFS_h
#include "defs.h"
#endif //QC_UTIL_DEFS_h

QC_UTIL_NAMESPACE_BEGIN

template <typename T>
class ValueRestorer
{
public:

	ValueRestorer(T& ref) :
		m_ref(ref),
		m_oldVal(ref),
		m_bRestoreOnDestruct(true)
	{
	}

	ValueRestorer(T& ref, const T& newVal) :
		m_ref(ref),
		m_oldVal(ref),
		m_bRestoreOnDestruct(true)
	{
		ref = newVal;
	}

	~ValueRestorer()
	{
		if(m_bRestoreOnDestruct)
			m_ref = m_oldVal;
	}

	void release()
	{
		m_bRestoreOnDestruct = false;
	}

private:
	T& m_ref;
	const T m_oldVal;
	bool m_bRestoreOnDestruct;
};

QC_UTIL_NAMESPACE_END

#endif //QC_UTIL_ValueRestorer_h


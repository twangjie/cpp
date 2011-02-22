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
// This class is based upon CountedPtr from the book 
// "The C++ Standard Library" by Nicolai M Josuttis
//
// It implements a non-invasive reference counting scheme
// where the count is maintained seperately from the object
// that is reference counted.
//
//==============================================================================

#ifndef QC_UTIL_RefCountedPtr_h
#define QC_UTIL_RefCountedPtr_h

#ifndef QC_UTIL_DEFS_h
#include "defs.h"
#endif //QC_UTIL_DEFS_h

QC_UTIL_NAMESPACE_BEGIN

template <typename T>
class RefCountedPtr
{
public:
	// constructor taking value returned from new()
	explicit RefCountedPtr(T* ptr=NULL) :
		m_pObject(ptr), m_pCount(new long(1)) {}

	// copy constructor - one more owner
	RefCountedPtr(const RefCountedPtr<T>& rhs) :
		m_pObject(rhs.m_pObject), m_pCount(rhs.m_pCount)
	{
		++*m_pCount;
	}

	// destructor - delete valuer (and count) if this is the last owner
	~RefCountedPtr()
	{
		dispose();
	}

	// assignment (release old and share new value)
	RefCountedPtr<T>& operator=(const RefCountedPtr<T>& rhs)
	{
		if (this != &rhs)
		{
			dispose();
			m_pObject = rhs.m_pObject;
			m_pCount = rhs.m_pCount;
			++*m_pCount;
		}
		return *this;
	}

	// accessors
	T& operator*() const
	{
		return *m_pObject;
	}

	T* operator->() const
	{
		return m_pObject;
	}

	T* get() const
	{
		return m_pObject;
	}

private:
	void dispose()
	{
		if(--*m_pCount == 0)
		{
			delete m_pCount;
			delete m_pObject;
		}
	}

	T* m_pObject;
	long* m_pCount;
};

QC_UTIL_NAMESPACE_END

#endif //QC_UTIL_RefCountedPtr_h


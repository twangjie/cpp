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

#ifndef QC_BASE_ArrayAutoPtr_h
#define QC_BASE_ArrayAutoPtr_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

QC_BASE_NAMESPACE_BEGIN

	template<typename T>
class ArrayAutoPtr
{
public:
	typedef T element_type;
	explicit ArrayAutoPtr(T *ptr = 0);
	ArrayAutoPtr(const ArrayAutoPtr<T>& rhs);
	ArrayAutoPtr<T>& operator=(const ArrayAutoPtr<T>& rhs);
	~ArrayAutoPtr();
	T& operator*() const;
	T& operator[](size_t n) const;
	T *get() const;
	T *release() const;

private:
	mutable bool m_bOwner;
	T *m_ptr;
};

//==============================================================================
/**
Creates an ArrayAutoPtr which takes ownership of the array 
starting at @c ptr.
*/
//==============================================================================
template<typename T>
inline ArrayAutoPtr<T>::ArrayAutoPtr(T* ptr) :
m_bOwner(ptr != 0), m_ptr(ptr)
{}

//==============================================================================
/**
Copy constructor which takes ownership of the array currently
owned by @c rhs.

Note that @c rhs is modified by this operation
even though it is marked as @c const.
*/
//==============================================================================
template<typename T>
inline ArrayAutoPtr<T>::ArrayAutoPtr(const ArrayAutoPtr<T>& rhs) :
m_bOwner(rhs.m_bOwner), m_ptr(rhs.release())
{
}

//==============================================================================
/**
Assignment operator which takes ownership of the array currently
owned by @c rhs.  If this ArrayAutoPtr already owns an
array pointer it is freed.

Note that @c rhs is modified by this operation
even though it is marked as @c const.
*/
//==============================================================================
template<typename T>
inline ArrayAutoPtr<T>& ArrayAutoPtr<T>::operator=(const ArrayAutoPtr<T>& rhs)
{
	if (this != &rhs)
	{
		if (m_ptr != rhs.get())
		{
			if (m_bOwner)
				delete [] m_ptr;
			m_bOwner = rhs.m_bOwner;
		}
		else if (rhs.m_bOwner)
			m_bOwner = true;
		m_ptr = rhs.release();
	}
	return (*this);
}

//==============================================================================
/**
Destructor which deletes the contained array pointer if it is 'owned'
by this object.
*/
//==============================================================================
template<typename T>
inline ArrayAutoPtr<T>::~ArrayAutoPtr()
{
	if (m_bOwner)
		delete [] m_ptr;
}

//==============================================================================
/**
Dereference operator which returns a reference to the first element of the
contained array.
*/
//==============================================================================
template<typename T>
inline T& ArrayAutoPtr<T>::operator*() const 
{
	return (*get());
}

//==============================================================================
/**
Dereference operator which returns a reference to the nth element of the
contained array.
*/
//==============================================================================
template<typename T>
inline T& ArrayAutoPtr<T>::operator[](size_t n) const 
{
	return (*(m_ptr+n));
}

//==============================================================================
/**
Returns the contained array pointer.
*/
//==============================================================================
template<typename T>
inline T* ArrayAutoPtr<T>::get() const 
{
	return (m_ptr);
}

//==============================================================================
/**
Returns the contained pointer and revokes 'ownership', so that the
destructor or assignment will not delete the contained pointer.
*/
//==============================================================================
template<typename T>
inline T* ArrayAutoPtr<T>::release() const 
{
	// handle lack of mutable support
	((ArrayAutoPtr<T>*)this)->m_bOwner = false;
	return (m_ptr);
}

QC_BASE_NAMESPACE_END

#endif //QC_BASE_ArrayAutoPtr_h

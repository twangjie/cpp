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
// Class: AutoPtr<T>
// 
//==============================================================================

#ifndef QC_BASE_AutoPtr_h
#define QC_BASE_AutoPtr_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include "debug.h"

QC_BASE_NAMESPACE_BEGIN

	template<typename T>
class AutoPtr
{
public:
	~AutoPtr();
	AutoPtr();

	AutoPtr(const AutoPtr<T>& rhs);
	AutoPtr(T* ptr);

	AutoPtr<T>& operator=(const AutoPtr<T>& rhs);
	AutoPtr<T>& operator=(T* ptr);

	bool operator<(const AutoPtr<T>& rhs) const;
	bool operator==(const AutoPtr<T>& rhs) const;
	bool operator==(const T* rhs) const;
	bool operator!=(const AutoPtr<T>& rhs) const;
	bool operator!=(const T* rhs) const;

	bool isNull() const;

	//operator T*() const; // not defined to prevent silent conversion
	operator bool() const;
	T* operator->() const;
	T& operator*() const;
	T* get() const;

	void release();

private:
	T* m_ptr;
};

//==============================================================================
// AutoPtr::AutoPtr
//
/**
Default constructor.

The internal object pointer is initialized to null.
*/
//==============================================================================
template<typename T>
inline
	AutoPtr<T>::AutoPtr() : m_ptr(0)
{
}

//==============================================================================
// AutoPtr::AutoPtr
//
/**
Constructs a AutoPtr with a pointer to T.

If @c ptr is not null, addRef() is called to increment the reference count.

@param ptr pointer to an object of type T, which is usually a class
derived from QCObject.
*/
//==============================================================================
template<typename T>
inline
	AutoPtr<T>::AutoPtr(T* ptr) : m_ptr(ptr)
{
	if(m_ptr)
		m_ptr->addRef();
}

//==============================================================================
// AutoPtr::AutoPtr
//
/**
Constructs a AutoPtr from another AutoPtr with the same object type.

If @c rhs contains a non-null object pointer, addRef() is called 
to increment the reference count.

@param rhs the AutoPtr<T> to copy.
*/
//==============================================================================
template<typename T>
inline
	AutoPtr<T>::AutoPtr(const AutoPtr& rhs) : m_ptr(rhs.m_ptr)
{
	if(m_ptr)
		m_ptr->addRef();
}

//==============================================================================
// AutoPtr::operator=
//
/**
Assigns one AutoPtr to another.

If @c rhs contains a non-null object pointer, addRef() is called 
to increment the reference count.

@param rhs the AutoPtr<T> to copy.
*/
//==============================================================================
template<typename T>
inline
	AutoPtr<T>& AutoPtr<T>::operator=(const AutoPtr<T>& rhs)
{
	if(m_ptr != rhs.get())
	{
		// do not release the current ptr until we have addrefed
		// the new value - just in case the new value relies on
		// the existing value for its continued existence
		T* pOld = m_ptr;

		m_ptr = rhs.get();
		if(m_ptr)
		{
			m_ptr->addRef();
		}
		if(pOld)
		{
			pOld->release();
		}

	}
	return *this;
}

//==============================================================================
// AutoPtr::operator=
//
/**
Assigns a pointer.

If @c ptr is not null, addRef() is called 
to increment the reference count.  If this AutoPtr previously referenced
an object, release() is called on that object.

@param ptr pointer to an object of type T
*/
//==============================================================================
template<typename T>
inline
	AutoPtr<T>& AutoPtr<T>::operator=(T* ptr)
{
	if(m_ptr != ptr)
	{
		// do not release the current ptr until we have addrefed
		// the new value - just in case the new value relies on
		// the existing value for its continued existence
		T* pOld = m_ptr;
		m_ptr = ptr;
		if(m_ptr)
		{
			m_ptr->addRef();
		}
		if(pOld)
		{
			pOld->release();
		}
	}
	return *this;
}

//==============================================================================
// AutoPtr::operator<
//
/**
Less than operator.

Returns true if this AutoPtr should be ordered before @c rhs.
If this and @c rhs both refer to null, @c false is returned.
@param rhs the AutoPtr to compare.
@since 1.3
*/
//==============================================================================
template<typename T>
inline
	bool AutoPtr<T>::operator<(const AutoPtr& rhs) const
{
	return (m_ptr < rhs.m_ptr);
}

//==============================================================================
// AutoPtr::operator==
//
/**
Equality operator.

Returns true if this AutoPtr refers to the same object as @c rhs.
If this and @c rhs both refer to null, @c true is returned.
@param rhs the AutoPtr to compare.
*/
//==============================================================================
template<typename T>
inline
	bool AutoPtr<T>::operator==(const AutoPtr& rhs) const
{
	return (m_ptr == rhs.m_ptr);
}

//==============================================================================
// AutoPtr::operator==
//
/**
Equality operator.

Returns true if this AutoPtr refers to the same object as @c rhs.
If this and @c rhs both refer to null, true is returned.
@param rhs a pointer to an object of type @c T which will be compared
with the contained pointer
*/
//==============================================================================
template<typename T>
inline
	bool AutoPtr<T>::operator==(const T* rhs) const
{
	return (m_ptr == rhs);
}

//==============================================================================
// AutoPtr::operator!=
//
/**
Inequality operator.

Returns true if this AutoPtr refers to a different object than @c rhs.
If this and @c rhs both refer to null, @c false is returned.

@param rhs the AutoPtr to compare.
*/
//==============================================================================
template<typename T>
inline
	bool AutoPtr<T>::operator!=(const AutoPtr& rhs) const
{
	return !(*this == rhs);
}

//==============================================================================
// AutoPtr::operator!=
//
/**
Inequality operator.

Returns true if this AutoPtr refers to a different object than @c rhs.
If this and @c rhs both refer to null, @c false is returned.

@param rhs a pointer to an object of type @c T which will be compared
against the contained pointer
*/
//==============================================================================
template<typename T>
inline
	bool AutoPtr<T>::operator!=(const T* rhs) const
{
	return (m_ptr != rhs);
}

//==============================================================================
// AutoPtr::operator bool
//
/**
Conversion operator.

@returns @c true if this AutoPtr is not equal to null.
See the class description for further information about
using the conversion operator.
*/
//==============================================================================
template<typename T>
inline 
	AutoPtr<T>::operator bool() const
{
	return (m_ptr!=0);
}

//==============================================================================
// AutoPtr::isNull
//
/**
Tests if the contained object pointer is null

@returns @c true if the object pointer is null; @c false otherwise
*/
//==============================================================================
template<typename T>
inline
	bool AutoPtr<T>::isNull() const
{
	return (m_ptr == 0);
}

//==============================================================================
// AutoPtr::get
//
/**
Returns the contained object pointer.

Neither the AutoPtr nor the reference count of its object are changed.
*/
//==============================================================================
template<typename T>
inline 
	T* AutoPtr<T>::get() const
{
	return m_ptr;
}

//==============================================================================
// AutoPtr::operator->
//
/**
A dereferencing operator.  Returns the contained object pointer.
This operator allows AutoPtrs to be used as if they were raw C++ pointers.
*/
//==============================================================================
template<typename T>
inline 
	T* AutoPtr<T>::operator->() const
{
	QC_DBG_ASSERT(m_ptr!=0);
	return m_ptr;
}

//==============================================================================
// AutoPtr::operator*
//
/**
A dereferencing operator.  Returns the contained object.
This operator allows AutoPtrs to be used as if they were raw C++ pointers.
*/
//==============================================================================
template<typename T>
inline 
	T& AutoPtr<T>::operator*() const
{
	QC_DBG_ASSERT(m_ptr!=0);
	return *m_ptr;
}

//==============================================================================
// AutoPtr::~AutoPtr
//
/**
Destructor.  Decrements the reference count of the contained object pointer
if it is not null.
*/
//==============================================================================
template<typename T>
inline 
	AutoPtr<T>::~AutoPtr()
{
	release();
}

//==============================================================================
// AutoPtr::release
//
/**
Decrements the reference count of the contained object pointer
if it is not null, before setting it to null.
*/
//==============================================================================
template<typename T>
inline 
	void AutoPtr<T>::release()
{
	if(m_ptr)
	{
		m_ptr->release();
		m_ptr = 0;
	}
}

QC_BASE_NAMESPACE_END

#endif //QC_BASE_AutoPtr_h


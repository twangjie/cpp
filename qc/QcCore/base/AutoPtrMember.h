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
// Class: AutoPtrMember<T>
// 
//==============================================================================

#ifndef QC_BASE_AutoPtrMember_h
#define QC_BASE_AutoPtrMember_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include "AutoPtr.h"

QC_BASE_NAMESPACE_BEGIN

class ManagedObject;

template<typename T>
class AutoPtrMember
{
public:
	~AutoPtrMember();
	AutoPtrMember(ManagedObject* pParent);
	AutoPtrMember(ManagedObject* pParent, T* ptr);
	AutoPtrMember(ManagedObject* pParent, const AutoPtrMember<T>& rhs);
	AutoPtrMember(ManagedObject* pParent, const AutoPtr<T>& rhs);

	AutoPtrMember<T>& operator=(const AutoPtrMember<T>& rhs);
	AutoPtrMember<T>& operator=(const AutoPtr<T>& rhs);
	AutoPtrMember<T>& operator=(T* ptr);

	bool operator<(const AutoPtrMember<T>& rhs) const;
	bool operator==(const AutoPtrMember<T>& rhs) const;
	bool operator!=(const AutoPtrMember<T>& rhs) const;
	bool operator==(const T* rhs) const;
	bool operator!=(const T* rhs) const;

	bool isNull() const;

	// operator T* is defined because AutoPtrMembers are not used as
	// return values from functions
	operator T*() const;
	T* operator->() const;
	T& operator*() const;
	T* get() const;

	void release();

private:
	T* m_ptr;
	ManagedObject* m_pParent;
};

//==============================================================================
// AutoPtrMember::AutoPtrMember
//
/**
Standard constructor, requires a pointer to a ManagedObject which is the
parent object containing this AutoPtrMember as a member.

The internal object pointer is initialized to null.
@param pParent the parent ManagedObject containing this AutoPtrMember as a member
*/
//==============================================================================
template<typename T>
inline
	AutoPtrMember<T>::AutoPtrMember(ManagedObject* pParent) :
m_ptr(0),
	m_pParent(pParent)
{
	if(!pParent) throw NullPointerException();
}

//==============================================================================
// AutoPtrMember::AutoPtrMember
//
/**
Constructs a AutoPtrMember with a pointer to T.

If @c ptr is not null and is not a reference to the parent object, addRef() is called
to increment the reference count.

@param pParent the parent ManagedObject containing this AutoPtrMember as a member
@param ptr pointer to an object of type T, which must be a class
derived from ManagedObject.
*/
//==============================================================================
template<typename T>
inline
	AutoPtrMember<T>::AutoPtrMember(ManagedObject* pParent, T* ptr) :
m_ptr(ptr),
	m_pParent(pParent)
{
	if(m_ptr && (static_cast<ManagedObject*>(m_ptr) != m_pParent))
		m_ptr->addRef();
}

//==============================================================================
// AutoPtrMember::AutoPtrMember
//
/**
Constructs a AutoPtrMember with another AutoPtrMember.

If @c rhs is not null and is not a reference to the parent object, addRef() is called
to increment the reference count.

@param pParent the parent ManagedObject containing this AutoPtrMember as a member
@param rhs a AutoPtrMember pointing to an object of type T, which must be a class
derived from ManagedObject.
*/
//==============================================================================
template<typename T>
inline
	AutoPtrMember<T>::AutoPtrMember(ManagedObject* pParent, const AutoPtrMember<T>& rhs) :
m_ptr(rhs.get()),
	m_pParent(pParent)
{
	if(m_ptr && (static_cast<ManagedObject*>(m_ptr) != m_pParent))
		m_ptr->addRef();
}

//==============================================================================
// AutoPtrMember::AutoPtrMember
//
/**
Constructs a AutoPtrMember from a AutoPtr for the same type.

If @c rhs is not null and is not a reference to the parent object, addRef() is called
to increment the reference count.

@param pParent the parent ManagedObject containing this AutoPtrMember as a member
@param rhs a AutoPtr pointing to an object of type T, which must be a class
derived from ManagedObject.
*/
// This has been added to give symmetry to the case where a AutoPtr can be created from
// a AutoPtrMember because of the AutoPtrMember conversion operator.  AutoPtr
// does not have a conversion operator, so we need this.
//==============================================================================
template<typename T>
inline
	AutoPtrMember<T>::AutoPtrMember(ManagedObject* pParent, const AutoPtr<T>& rhs) :
m_ptr(rhs.get()),
	m_pParent(pParent)
{
	if(m_ptr && (static_cast<ManagedObject*>(m_ptr) != m_pParent))
		m_ptr->addRef();
}

//==============================================================================
// AutoPtrMember::operator=
//
/**
Assigns one AutoPtrMember to another.

If @c rhs contains a non-null object pointer, and is not a reference to
the parent, addRef() is called to increment the reference count.

@param rhs the AutoPtrMember<T> to copy.
*/
//==============================================================================
template<typename T>
inline
	AutoPtrMember<T>& AutoPtrMember<T>::operator=(const AutoPtrMember<T>& rhs)
{
	if(m_ptr != rhs.get())
	{
		// do not release the current ptr until we have addrefed
		// the new value - just in case the new value relies on
		// the existing value for its continued existence
		T* pOld = m_ptr;

		m_ptr = rhs.get();
		if(m_ptr && (static_cast<ManagedObject*>(m_ptr) != m_pParent))
		{
			m_ptr->addRef();
		}
		if(pOld && (static_cast<ManagedObject*>(pOld) != m_pParent))
		{
			pOld->release();
		}
	}
	return *this;
}

//==============================================================================
// AutoPtrMember::operator=
//
/**
Assigns a AutoPtr to a AutoPtrMember.

If @c rhs contains a non-null object pointer which is not a reference to
the parent, addRef() is called to increment the reference count.

@param rhs the AutoPtr<T> to copy.
*/
//==============================================================================
template<typename T>
inline
	AutoPtrMember<T>& AutoPtrMember<T>::operator=(const AutoPtr<T>& rhs)
{
	return (*this)=rhs.get();
}

//==============================================================================
// AutoPtrMember::operator=
//
/**
Assigns a pointer.

If @c ptr is not null, and does not refer to the parent object, addRef() is called 
to increment the reference count.  If this AutoPtrMember previously referenced
an object, release() is called on that object.

@param ptr pointer to an object of type T, which must be a class
derived from ManagedObject.
*/
//==============================================================================
template<typename T>
inline
	AutoPtrMember<T>& AutoPtrMember<T>::operator=(T* ptr)
{
	if(m_ptr != ptr)
	{
		// do not release the current ptr until we have addrefed
		// the new value - just in case the new value relies on
		// the existing value for its continued existence
		T* pOld = m_ptr;
		m_ptr = ptr;
		if(m_ptr && (static_cast<ManagedObject*>(m_ptr) != m_pParent))
		{
			m_ptr->addRef();
		}
		if(pOld && (static_cast<ManagedObject*>(pOld) != m_pParent))
		{
			pOld->release();
		}
	}
	return *this;
}

//==============================================================================
// AutoPtrMember::operator<
//
/**
Less than operator.

Returns true if this AutoPtrMember should be ordered before @c rhs.
If this and @c rhs both refer to null, @c false is returned.
@param rhs the AutoPtr to compare.
@since 1.3
*/
//==============================================================================
template<typename T>
inline
	bool AutoPtrMember<T>::operator<(const AutoPtrMember& rhs) const
{
	return (m_ptr < rhs.m_ptr);
}

//==============================================================================
// AutoPtrMember::operator==
//
/**
Equality operator.

Returns true if this AutoPtrMember refers to the same object as @c rhs.
If this and @c rhs both refer to null, true is returned.
@param rhs the AutoPtrMember to compare.
*/
//==============================================================================
template<typename T>
inline
	bool AutoPtrMember<T>::operator==(const AutoPtrMember& rhs) const
{
	return (m_ptr == rhs.m_ptr);
}

//==============================================================================
// AutoPtrMember::operator==
//
/**
Equality operator.

Returns true if this AutoPtrMember refers to the same object as @c rhs.
If this and @c rhs both refer to null, true is returned.
@param rhs the pointer to compare.
*/
//==============================================================================
template<typename T>
inline
	bool AutoPtrMember<T>::operator==(const T* rhs) const
{
	return (m_ptr == rhs);
}

//==============================================================================
// AutoPtrMember::operator!=
//
/**
Inequality operator.

Returns true if this AutoPtrMember refers to a different object than @c rhs.
If this and @c rhs both refer to null, false is returned.

@param rhs the AutoPtrMember to compare.
*/
//==============================================================================
template<typename T>
inline
	bool AutoPtrMember<T>::operator!=(const AutoPtrMember& rhs) const
{
	return !(*this == rhs);
}

//==============================================================================
// AutoPtrMember::operator!=
//
/**
Inequality operator.

Returns true if this AutoPtrMember refers to a different object than @c rhs.
If this and @c rhs both refer to null, false is returned.

@param rhs the pointer to compare.
*/
//==============================================================================
template<typename T>
inline
	bool AutoPtrMember<T>::operator!=(const T* rhs) const
{
	return (m_ptr != rhs);
}

//==============================================================================
// AutoPtrMember::operator T*
//
/**
Conversion operator.

Silently converts this AutoPtrMember<T> into a T*.
*/
//==============================================================================
template<typename T>
inline 
	AutoPtrMember<T>::operator T*() const
{
	return m_ptr;
}

//==============================================================================
// AutoPtrMember::isNull
//
/**
Tests if the contained object pointer is null

@returns true if the object pointer is null; false otherwise
*/
//==============================================================================
template<typename T>
inline
	bool AutoPtrMember<T>::isNull() const
{
	return (m_ptr == 0);
}

//==============================================================================
// AutoPtrMember::get
//
/**
Returns the contained object pointer.

Neither the AutoPtrMember nor the reference count of its object are changed.
*/
//==============================================================================
template<typename T>
inline 
	T* AutoPtrMember<T>::get() const
{
	return m_ptr;
}

//==============================================================================
// AutoPtrMember::operator->
//
/**
A dereferencing operator.  Returns the contained object pointer.
This operator allows AutoPtrMembers to be used as if they were raw C++ pointers.
*/
//==============================================================================
template<typename T>
inline 
	T* AutoPtrMember<T>::operator->() const
{
	QC_DBG_ASSERT(m_ptr!=0);
	return m_ptr;
}

//==============================================================================
// AutoPtrMember::operator*
//
/**
A dereferencing operator.  Returns the contained object.
This operator allows AutoPtrMembers to be used as if they were raw C++ pointers.
*/
//==============================================================================
template<typename T>
inline 
	T& AutoPtrMember<T>::operator*() const
{
	QC_DBG_ASSERT(m_ptr!=0);
	return *m_ptr;
}

//==============================================================================
// AutoPtrMember::~AutoPtrMember
//
/**
Destructor.  Decrements the reference count of the contained object pointer
if it is not null and is not the same as the parent object.
*/
//==============================================================================
template<typename T>
inline 
	AutoPtrMember<T>::~AutoPtrMember()
{
	release();
}

//==============================================================================
// AutoPtrMember::release
//
/**
Decrements the reference count of the contained object pointer
if it is not null and is not the same as the parent object.
The contained object pointer is then set equal to null.
*/
//==============================================================================
template<typename T>
inline 
	void AutoPtrMember<T>::release()
{
	if(m_ptr && (static_cast<ManagedObject*>(m_ptr) != m_pParent))
	{
		m_ptr->release();
	}
	m_ptr = 0;
}

QC_BASE_NAMESPACE_END

#endif //QC_BASE_AutoPtrMember_h


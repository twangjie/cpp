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
// Class: AutoLock
// 
//==============================================================================

#ifndef QC_BASE_AutoLock_h
#define QC_BASE_AutoLock_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h
#include "NullPointerException.h"

QC_BASE_NAMESPACE_BEGIN

template<typename T>
class AutoLock
{
public:
	typedef T LockType;

	AutoLock(T& lock);
	AutoLock(T& lock, bool bInitialLock);
	~AutoLock();

	void lock();
	void unlock();

private: // not implemented
	AutoLock(const AutoLock<T>& rhs);            // cannot be copied
	AutoLock& operator=(const AutoLock<T>& rhs); // nor assigned

private:
	T& m_lock;
	bool m_bLocked;
};

//==============================================================================
// AutoLock<T>::AutoLock(T& lock)
//
/**
Constructor which takes a reference to @c lock and locks it.
@param lock a reference to the synchronization object that will be managed
by this AutoLock object.
@throws NullPointerException if @c lock is a reference to a null pointer
*/
//==============================================================================
template<typename T>
inline
	AutoLock<T>::AutoLock(T& lock) :
m_lock(lock),
	m_bLocked(false)
{
	if(&lock == NULL) throw NullPointerException();

	m_lock.lock();
	m_bLocked = true;
}

//==============================================================================
// AutoLock<T>::AutoLock
//
/**
Constructor which takes a reference to @c lock and locks it
if @c bInitialLock is true.
@param lock a reference to the synchronization object that will be managed
by this AutoLock object.
@param bInitialLock if true, the lock is acquired by the constructor
@throws NullPointerException if @c lock is a reference to a null pointer
*/
//==============================================================================
template<typename T>
inline
	AutoLock<T>::AutoLock(T& lock, bool bInitialLock) :
m_lock(lock),
	m_bLocked(false)
{
	if(&lock == NULL) throw NullPointerException();

	if(bInitialLock)
	{
		m_lock.lock();
		m_bLocked = true;
	}
}

//==============================================================================
// AutoLock<T>::~AutoLock
//
/**
Destructor which releases the lock if it is still being held.
*/
//==============================================================================
template<typename T>
inline
	AutoLock<T>::~AutoLock()
{
	if(m_bLocked)
	{
		m_lock.unlock();
	}
}

//==============================================================================
// AutoLock<T>::unlock
//
/**
Releases the lock.  If the lock is not currently being held, this method
has no effect.
*/
//==============================================================================
template<typename T>
inline
	void AutoLock<T>::unlock()
{
	if(m_bLocked)
	{
		m_lock.unlock();
		m_bLocked = false;
	}
}

//==============================================================================
// AutoLock<T>::lock
//
/**
Acquires the lock.  If the lock is already held this method has no effect.
*/
//==============================================================================
template<typename T>
inline
	void AutoLock<T>::lock()
{
	if(!m_bLocked)
	{
		m_lock.lock();
		m_bLocked = true;
	}
}

QC_BASE_NAMESPACE_END

#endif //QC_BASE_AutoLock_h


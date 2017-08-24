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
// Class: AutoUnlock
// 
//==============================================================================

#ifndef QC_BASE_AutoUnlock_h
#define QC_BASE_AutoUnlock_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

QC_BASE_NAMESPACE_BEGIN

	template<typename T>
class AutoUnlock
{
public:

	AutoUnlock(T& lock);
	~AutoUnlock();

	void lock();
	void unlock();

private: // not implemented
	AutoUnlock(const AutoUnlock<T>& rhs);            // cannot be copied
	AutoUnlock& operator=(const AutoUnlock<T>& rhs); // nor assigned

private:
	T& m_lock;
	bool m_bUnlocked;
};

//==============================================================================
// AutoUnlock<T>::AutoUnlock(T& lock)
//
/**
Constructor which takes a reference to @c lock and unlocks it.
@param lock a reference to the synchronization object that will be managed
by this AutoUnlock object.
*/
//==============================================================================
template<typename T>
inline
	AutoUnlock<T>::AutoUnlock(T& lock) :
m_lock(lock),
	m_bUnlocked(false)
{
	m_lock.unlock();
	m_bUnlocked = true;
}

//==============================================================================
// AutoUnlock<T>::~AutoUnlock
//
/**
Destructor which re-acquires the lock if it is not being held.
*/
//==============================================================================
template<typename T>
inline
	AutoUnlock<T>::~AutoUnlock()
{
	if(m_bUnlocked)
	{
		m_lock.lock();
	}
}

//==============================================================================
// AutoUnlock<T>::lock
//
/**
Re-acquires the lock if it is not being held; has no effect otherwise.
*/
//==============================================================================
template<typename T>
inline
	void AutoUnlock<T>::lock()
{
	if(m_bUnlocked)
	{
		m_lock.lock();
		m_bUnlocked = false;
	}
}

//==============================================================================
// AutoUnlock<T>::unlock
//
/**
Releases the lock if it is being held; has no effect otherwise.
*/
//==============================================================================
template<typename T>
inline
	void AutoUnlock<T>::unlock()
{
	if(!m_bUnlocked)
	{
		m_lock.unlock();
		m_bUnlocked = true;
	}
}

QC_BASE_NAMESPACE_END

#endif //QC_BASE_AutoUnlock_h


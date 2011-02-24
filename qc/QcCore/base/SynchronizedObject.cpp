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
// Class: SynchronizedObject
// 
/**
	@class qc::SynchronizedObject
	
	@brief A Base class facilitating synchronized concurrent access
	       from multiple threads.

    The SynchronizedObject enables thread-safe access by the use of a contained
	RecursiveMutex object.  Locking and unlocking of the mutex is performed by
	a friend class with the typedef name SynchronizedObject::Lock.  This class is
	designed to be used as a scoped lock: the mutex lock is acquired by the
	constructor and released by the destructor.  In this way, the mutex lock is 
	guaranteed to be released even if an exception is thrown by the intervening code.

    @code
    int SafeObject::getSafeId()
    {
        // acquire our mutex lock
        SynchronizedObject::Lock lock(*this);
        return m_id;
        // mutex lock is automatically released at end of scope
    }
    @endcode

	The SynchronizedObject::Lock constructor takes a non-const reference
	to a SynchronizedObject.  To use SynchronizedObject::Lock from a const
	function or with a const object reference it is necessary to "cast away the
	constness".  To avoid having to litter your const-correct code with casts,
	the @c QC_SYNCHRONIZED macro may be used.  For example: 

    @code
    int SafeObject::getSafeConstId() const
    {
        QC_SYNCHRONIZED
        return m_id;
    }
    @endcode

	The @c QC_SYNCHRONIZED macro has the additional advantage that it is #defined
	to do nothing in the single-threaded version of the library.

	The SynchronizedObject class is derived from QCObject so that
	standard @QuickCPP reference-counted object references can be created and stored.

    If you wish to synchronize against another SynchronizedObject, there
	are two other macros to choose from:-
	- @c QC_SYNCHRONIZED_PTR if the foreign object is guaranteed to exist
	  for the lifetime of the scoped lock; or
	- @c QC_SYNCHRONIZED_PTR_ADD which increments the reference count of
	  the SynchronizedObject for the duration of the scoped lock.

    @code
    void someSynchronizedFunction()
    {
        QC_SYNCHRONIZED_PTR_ADD(getSyncObject())

        // Do something useful here
    }
    @endcode
*/
//==============================================================================

#include "SynchronizedObject.h"

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// SynchronizedObject::lock
//
/**
   Locks the mutex.  If the mutex is already locked by another thread, the
   current thread blocks until the mutex becomes free.

   While a thread has ownership of a SynchronizedObject, it can specify the
   same object in additional lock() calls without blocking its execution.
   This prevents a thread from deadlocking itself while waiting for a 
   SynchronizedObject that it already owns. However, to release its ownership,
   the thread must call unlock() once for each time that the SynchronizedObject
   was locked.. 
*/
//==============================================================================
void SynchronizedObject::lock()
{
#ifdef QC_MT
	m_mutex.lock();
#endif //QC_MT
}

//==============================================================================
// SynchronizedObject::unlock
//
/**
   Unlocks the mutex.

   Decrements the use count of the internal mutex.  If the use count is decremented
   to zero, the mutex is released and may then be acquired by another thread.
*/
//==============================================================================
void SynchronizedObject::unlock()
{
#ifdef QC_MT
	m_mutex.unlock();
#endif //QC_MT
}

QC_BASE_NAMESPACE_END

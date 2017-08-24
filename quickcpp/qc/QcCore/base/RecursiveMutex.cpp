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
// Class: RecursiveMutex
// 
/**
	@class qc::RecursiveMutex
	
	@brief Variety of mutex that can be locked multiple times
	       by the same thread without blocking.

    A mutex can be acquired (locked) by only one thread at a time.  A RecursiveMutex
	can be locked multiple times by the same thread without blocking.  It contains
	an internal usage count which is incremented by lock() and decremented by unlock().
	When the internal usage count is decremented to zero the mutex is released and may
	be acquired by another thread.
	
    While a thread 'owns' the mutex (i.e. has it locked), it can safely use
	the protected resource, which may be a variable, a region of memory or 
	anything else, in the knowledge that no other thread will be accessing it.
	However, this only holds true if all threads obey the rules and acquire 
	the mutex before attempting to access the protected resource.
	This is where the 'mutual' part comes in - all programs that access the
	resource must acquire the mutex first.
	
	@sa Mutex
	@sa FastMutex
*/
//==============================================================================

#include "RecursiveMutex.h"
#include "IllegalMonitorStateException.h"
#include "Thread.h"

#ifdef QC_MT

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// RecursiveMutex::RecursiveMutex
//
/**
   Default constructor.
*/
//==============================================================================
RecursiveMutex::RecursiveMutex() :
	m_recursionCount(0)
{
}

//==============================================================================
// Mutex::lock
//
/**
   Acquires the mutex.     If the current thread already owns the mutex then the
   lock count is incremented.  In this case unlock() must be called an equal number
   of times before the mutex will become released.
   
   If the mutex is currently held by another thread
   this call blocks and waits until the mutex becomes available.
*/
//==============================================================================
void RecursiveMutex::lock()
{
	const ThreadId current = Thread::CurrentThreadId();

	if(current == m_owningThreadId && m_recursionCount > 0)
	{
		m_recursionCount++;
		return;
	};

	//
	// If we were to use an internal mutex as a memory barrier,
	// then it would have to be released here, otherwise
	// another thread that owns the real mutex will not be able to
	// release it!
	//
	//internalLock.unlock();

	m_mutex.lock();

	//
	// Once we have the mutex, we are at liberty to update
	// our internal state
	//
	m_owningThreadId = current;
	m_recursionCount = 1;
}

//==============================================================================
// RecursiveMutex::trylock
//
/**
   Attempts to acquire the mutex without blocking.  This function differs
   from lock() because it returns immediately even if the mutex is held
   by another thread.

   If the current thread already owns the mutex then the
   lock count is incremented.  In this case unlock() must be called an equal number
   of times before the mutex will become released.

   @returns @c true if the mutex was successfully acquired; @c false otherwise.
   @sa lock()
   @sa unlock()
*/
//==============================================================================
bool RecursiveMutex::tryLock()
{
	const ThreadId current = Thread::CurrentThreadId();

	if(current == m_owningThreadId && m_recursionCount > 0)
	{
		m_recursionCount++;
		return true;
	};

	//
	// If we were to use an internal mutex as a memory barrier,
	// then it would have to be released here, otherwise
	// another thread that owns the real mutex will not be able to
	// release it!
	//
	//internalLock.unlock();

	if(m_mutex.tryLock())
	{
		//
		// Once we have the mutex, we are at liberty to update
		// our internal state
		//
		m_owningThreadId = current;
		m_recursionCount = 1;
		return true;
	}
	else
	{
		return false;
	}
}

//==============================================================================
// RecursiveMutex::unlock
//
/**
   Releases the mutex.  unlock() must be called an equal number of times
   that lock() is called before the mutex is released and made available to
   another thread.

   @throws IllegalMonitorStateException if the current thread does not own the
           mutex (i.e. does not have it locked)
*/
//==============================================================================
void RecursiveMutex::unlock()
{
	const ThreadId current = Thread::CurrentThreadId();

	if(m_owningThreadId != current || m_recursionCount ==  0)
	{
		throw IllegalMonitorStateException();
	}
	
	if(m_recursionCount > 1)
	{
		--m_recursionCount;
	}
	else
	{
		QC_DBG_ASSERT(m_recursionCount==1);
		m_recursionCount=0;
		m_owningThreadId=0;
		m_mutex.unlock();
	}
}

//==============================================================================
// RecursiveMutex::isLocked
//
/**
   Tests if the mutex is currently owned by the current thread.

   @returns true if the current thread has the mutex locked; false otherwise
*/
//==============================================================================
bool RecursiveMutex::isLocked()
{
	const ThreadId current = Thread::CurrentThreadId();
	return (m_owningThreadId == current && m_recursionCount > 0);
}

//==============================================================================
// RecursiveMutex::getRecursionCount
//
//==============================================================================
int RecursiveMutex::getRecursionCount() const
{
	return m_recursionCount;
}

//==============================================================================
// RecursiveMutex::postWait
//
// Used by ConditionVariable to re-establish the recursion count and owner.
//==============================================================================
void RecursiveMutex::postWait(int recursionCount)
{
	m_owningThreadId = Thread::CurrentThreadId();
	m_recursionCount = recursionCount;
}

//==============================================================================
// RecursiveMutex::preWait
//
// Used by ConditionVariable to mark the mutex as no-longer belonging to the
// current thread.
//==============================================================================
void RecursiveMutex::preWait()
{
	m_owningThreadId = 0;
}

QC_BASE_NAMESPACE_END

#endif //QC_MT

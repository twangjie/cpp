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
// Class: FastMutex
// 
/**
	@class qc::FastMutex
	
	@brief Variety of mutex that is optimized and may offer
	       better performance on some platforms.

	A mutex can be acquired (locked) by only one thread at a time.  A thread
	should not attempt to acquire a FastMutex that it already owns (see
	RecursiveMutex if you require this).
	
    While a thread 'owns' the mutex (i.e. has it locked), it can safely use
	the protected resource, which may be a variable, a region of memory or 
	anything else, in the knowledge that no other thread will be accessing it.
	However, this only holds true if all threads obey the rules and acquire 
	the mutex before attempting to access the protected resource.

	@sa Mutex
	@sa RecursiveMutex
*/
//==============================================================================

#include "FastMutex.h"
#include "OSException.h"

#include <errno.h>

#ifdef QC_MT

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// FastMutex::FastMutex
//
/**
   Default constructor.  Initializes the mutex for use.
*/
//==============================================================================
FastMutex::FastMutex()
{
#if defined(QC_WIN32_THREADS)

	::InitializeCriticalSection(&m_mutex);

#elif defined(QC_POSIX_THREADS)

	::pthread_mutex_init(&m_mutex, 0);

#endif
}

//==============================================================================
// FastMutex::~FastMutex
//
/**
   Destructor.

   Frees any resources held by the mutex.
*/
//==============================================================================
FastMutex::~FastMutex()
{
#if defined(QC_WIN32_THREADS)

	::DeleteCriticalSection(&m_mutex);

#elif defined(QC_POSIX_THREADS)

	::pthread_mutex_destroy(&m_mutex);

#endif
}

//==============================================================================
// FastMutex::lock
//
/**
   Acquires the mutex.
   
   If the mutex is currently held by another thread
   this call blocks and waits until the mutex becomes available.

   A thread should not attempt to lock a FastMutex that it already owns.
   This behaviour is undefined for the FastMutex class but is supported by
   RecursiveMutex.

   @sa tryLock
   @sa unlock
*/
//==============================================================================
void FastMutex::lock()
{
#if defined(QC_WIN32_THREADS)

	::EnterCriticalSection(&m_mutex);

#elif defined(QC_POSIX_THREADS)

	int status = ::pthread_mutex_lock(&m_mutex);
	if(status != 0) throw OSException(status, QC_T("pthread_mutex_lock"));

#endif
}

//==============================================================================
// FastMutex::tryLock
//
/**
   Attempts to acquire the mutex without blocking.  This function differs
   from lock() because it returns immediately even if the mutex is held
   by another thread.

   @returns @c true if the mutex was successfully acquired; @c false otherwise.
   @sa lock()
   @sa unlock()
*/
//==============================================================================
bool FastMutex::tryLock()
{
#if defined(QC_WIN32_THREADS)

	return (::TryEnterCriticalSection(&m_mutex)!=0);

#elif defined(QC_POSIX_THREADS)

	int status = ::pthread_mutex_trylock(&m_mutex);
	switch(status)
	{
	case 0:
		return true;
	case EBUSY:
		return false;
	default:
		throw OSException(status, QC_T("pthread_mutex_trylock"));
	}

#endif
}

//==============================================================================
// FastMutex::unlock
//
/**
   Releases the mutex.
*/
//==============================================================================
void FastMutex::unlock()
{
#if defined(QC_WIN32_THREADS)

	::LeaveCriticalSection(&m_mutex);

#elif defined(QC_POSIX_THREADS)

	int status = ::pthread_mutex_unlock(&m_mutex);
	if(status != 0) throw OSException(status, QC_T("pthread_mutex_unlock"));

#endif
}

QC_BASE_NAMESPACE_END
#endif // QC_MT


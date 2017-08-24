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
// Class: Mutex
// 
/**
	@class qc::Mutex
	
	@brief Multi-threaded synchronization object that enables mutually exclusive
	       access to a protected resource.

	A mutex can be acquired (locked) by only one thread at a time.  A thread
	should not attempt to acquire a Mutex that it already owns (see
	RecursiveMutex if you require this).
	
    While a thread 'owns' the mutex (i.e. has it locked), it can safely use
	the protected resource, which may be a variable, a region of memory or 
	anything else, in the knowledge that no other thread will be accessing it.
	However, this only holds true if all threads obey the rules and acquire 
	the mutex before attempting to access the protected resource.
	This is where the 'mutual' part comes in - all programs that access the
	resource must acquire the mutex first.

	@sa FastMutex
	@sa RecursiveMutex
*/
//==============================================================================

#include "Mutex.h"
#include "OSException.h"

#include <errno.h>

#if defined(WIN32)
	#include "Win32Exception.h"
#endif //WIN32

#ifdef QC_MT

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// Mutex::Mutex
//
/**
   Default constructor.  Initializes the mutex for use.
*/
//==============================================================================
Mutex::Mutex()
{
#if defined(QC_WIN32_THREADS)

	m_hMutex = ::CreateMutex(0, FALSE, 0);
	if(m_hMutex == 0) throw Win32Exception(::GetLastError(), QC_T("CreateMutex"));

#elif defined(QC_POSIX_THREADS)

	::pthread_mutex_init(&m_mutex, 0);

#endif
}

//==============================================================================
// Mutex::~Mutex
//
/**
   Destructor.

   Frees any resources held by the mutex.
*/
//==============================================================================
Mutex::~Mutex()
{
#if defined(QC_WIN32_THREADS)

	::CloseHandle(m_hMutex);

#elif defined(QC_POSIX_THREADS)

	::pthread_mutex_destroy(&m_mutex);

#endif
}

//==============================================================================
// Mutex::lock
//
/**
   Acquires the mutex.
   
   If the mutex is currently held by another thread this call blocks and waits
   until the mutex becomes available.  A thread should not attempt to lock a
   Mutex that it already owns.  This behaviour is undefined for the Mutex class
   but is supported by RecursiveMutex.

   @sa tryLock
   @sa unlock
*/
//==============================================================================
void Mutex::lock()
{
#if defined(QC_WIN32_THREADS)

	DWORD result = ::WaitForSingleObject(m_hMutex, INFINITE);
	if(result == WAIT_FAILED) throw Win32Exception(::GetLastError(), QC_T("WFSO"));

#elif defined(QC_POSIX_THREADS)

	int status = ::pthread_mutex_lock(&m_mutex);
	if(status != 0) throw OSException(status, QC_T("pthread_mutex_lock"));

#endif
}

//==============================================================================
// Mutex::tryLock
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
bool Mutex::tryLock()
{
#if defined(QC_WIN32_THREADS)

	DWORD result = ::WaitForSingleObject(m_hMutex, 0);
	switch (result)
	{
	case WAIT_OBJECT_0:
		return true;
	case WAIT_TIMEOUT:
		return false;
	default:
		throw Win32Exception(::GetLastError(), QC_T("WFSO"));
	}

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
// Mutex::unlock
//
/**
   Releases the mutex.
*/
//==============================================================================
void Mutex::unlock()
{
#if defined(QC_WIN32_THREADS)

	if(!::ReleaseMutex(m_hMutex))
		throw Win32Exception(::GetLastError(), QC_T("ReleaseMutex"));

#elif defined(QC_POSIX_THREADS)

	int status = ::pthread_mutex_unlock(&m_mutex);
	if(status != 0) throw OSException(status, QC_T("pthread_mutex_unlock"));

#endif
}

QC_BASE_NAMESPACE_END

#endif //QC_MT

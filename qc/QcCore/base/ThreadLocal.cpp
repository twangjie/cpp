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
// Class: ThreadLocal
//
/**
	@class qc::ThreadLocal
	
	@brief Provides a class interface to native thread-local storage which
	provides each thread with its own copy of a variable.
	
	Each thread's copy of a ThreadLocal variable is initially set to zero.

    The variable type is a @c void pointer that can be used to hold a pointer
	or it can be cast to a long integer value.

    In order for a ThreadLocal variable to be available to all threads, it is
	often convenient for instances of ThreadLocal to be created statically
	during program initialization.  In this way the variable exists for the lifetime
	of the application, and any thread can access its individual value at any time.

    In the following example, a static ThreadLocal variable is used to hold
	a pointer to an instance of an imaginary class @c WorkerThreadInfo.
	In this case, if the current thread does not have an instance of @c WorkerThreadInfo
	then it creates one and stores the pointer for subsequent use.

    @code
    static ThreadLocal ThreadInfoPointer;

    WorkerThreadInfo* getThreadInfo()
    {
        WorkerThreadInfo* pInfo = ThreadInfoPointer.get();
        if(!pInfo)
        {
            pInfo = new WorkerThreadInfo(Thread::CurrentThreadId());
            ThreadInfoPointer.set(pInfo);
        }
        return pInfo;
    }
    @endcode

    Note that the above example contains a potential memory leak. When a thread terminates,
	the ThreadLocal variable for that particular thread becomes inaccessible.  If
	the variable contains a pointer to allocated memory then the memory is @em not
	automatically freed.  For this reason it is a good idea to put any necessary clean-up
	code into the Runnable::run() method.
*/
//==============================================================================

#include "ThreadLocal.h"
#include "OSException.h"

#ifdef WIN32
#include "Win32Exception.h"
#endif //WIN32

#ifdef QC_MT

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// ThreadLocal::ThreadLocal
//
/**
   Default constructor.  Uses the operating-system's threading library to 
   allocate a new thread-local variable.

   The value of the variable is automatically initialized to zero for every
   thread.
*/
//==============================================================================
ThreadLocal::ThreadLocal()
{
#if defined(QC_WIN32_THREADS)

	m_key = ::TlsAlloc();
	if(m_key == (DWORD)-1)
	{
		throw Win32Exception(::GetLastError());
	}

#elif defined(QC_POSIX_THREADS)

	int status = ::pthread_key_create(&m_key, 0);
	if(status != 0)
	{
		throw OSException(status, QC_T("pthread_key_create"));
	}

#endif
}

//==============================================================================
// ThreadLocal::~ThreadLocal
//
/**
   Destructor.  Frees the thread-local variable, making it inaccessible from
   any thread.
*/
//==============================================================================
ThreadLocal::~ThreadLocal()
{
#if defined(QC_WIN32_THREADS)
	::TlsFree(m_key);
#elif defined(QC_POSIX_THREADS)
	::pthread_key_delete(m_key);
#endif
}
	
//==============================================================================
// ThreadLocal::set
//
/**
   Sets the value of this ThreadLocal for the currently executing thread.
   @param value the value to set this ThreadLocal variable to for the current thread
   @sa get()
*/
//==============================================================================
void ThreadLocal::set(void* value) const
{
#if defined(QC_WIN32_THREADS)

	::TlsSetValue(m_key, value);

#elif defined(QC_POSIX_THREADS)

	::pthread_setspecific(m_key, value);

#endif
}

//==============================================================================
// ThreadLocal::get
//
/**
   Returns the current value of this ThreadLocal variable for the current thread.
   @sa set()
*/
//==============================================================================
void* ThreadLocal::get() const
{
#if defined(QC_WIN32_THREADS)
	
	return ::TlsGetValue(m_key);

#elif defined(QC_POSIX_THREADS)

	return ::pthread_getspecific(m_key);

#endif
}

QC_BASE_NAMESPACE_END

#endif //QC_MT

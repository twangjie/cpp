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
// Class: ConditionVariable
// 
/**
	@class qc::ConditionVariable

    @brief Provides a means for a thread to be signalled when a condition
	       becomes true.

    When a thread has acquired a mutex on some shared data, but then finds
	that the data is not in the correct state for it to do some work, then 
	it should release the mutex and wait for another thread to signal it that
	the shared data has changed.  This is the basic function of a POSIX-style
	condition variable.

    A condition variable is always used in conjunction with a mutex.  The mutex
	is used to synchronize access to the shared data, and the condition variable
	is used to signal to waiting threads that a condition or @em predicate has become
	true.

    A program cannot examine the shared data without having the mutex locked.  But if
	the mutex is locked, no other thread will be able to change the state, so the
	wait operation @em atomically releases the mutex and blocks the thread pending
	a signal from another thread.  When the waiting thread is signalled, the mutex is
	re-acquired before the wait() method returns to the caller.

    Here is a simple example of a thread that pulls a single item from a queue.  If the
	queue is empty, it blocks until an item becomes available:
	
    @code
    WorkItem Worker::getNextWorkItem()
    {
        // acquire the queue's mutex
        RecursiveMutex::Lock lock(m_queueMutex);
        while(m_queue.empty())
        {
            try
            {
                // mutex is released for the wait...
                m_queueCV.wait();
                // ...and re-acquired on return
            }
            catch(InterruptedException& e) {}
        }
        // pop the first item from the work queue
        WorkItem ret = m_queue.front();
        m_queue.pop_front();
        return ret;
        // mutex lock is automatically released on exit
    }
    @endcode

    Note the structure of this function, where the @c predicate is tested in a
	@c while loop.  This is a standard and recommended technique which is used
	to make the function robust in the presence of <i>spurious wake-ups</i>.

    Following on from the previous example, here is a function
	that adds an item to the queue and signals the waiting thread 
	that there is some work to be done:

    @code
    void Worker::queueWorkItem(const WorkItem& item)
    {
        // acquire the queue's mutex
        RecursiveMutex::Lock lock(m_queueMutex);
        m_queue.push_back(item);
        // inform a waiting thread that a work item is now available
        m_queueCV.signal();
        // mutex is automatically released on exit from the scope
    }
    @endcode

    In the above example it is not a requirement that the mutex lock be held while
	the @c signal() operation is performed, but it is generally better to do so.  If the
	function added multiple items to the queue, then @c broadcast() would be a better
	choice than @c signal() if multiple worker threads could be waiting on the queue.

    A single condition variable may be associated with more than one mutex.  Likewise
	a single mutex may be associated with more than one condition variable.

	On POSIX-compliant systems this class is a simple wrapper around
	a POSIX threads condition variable.  On other systems, a condition
	variable is emulated using the available synchronization primitives.
*/
//==============================================================================

#include "ConditionVariable.h"
#include "AutoUnlock.h"
#include "IllegalMonitorStateException.h"
#include "InterruptedException.h"
#include "Mutex.h"
#include "RecursiveMutex.h"
#include "OSException.h"
#include "Thread.h"

#if defined(WIN32)
	#include "Win32Exception.h"

	// This is the APC callback routine function for signalling a thread
	VOID WINAPI DummyAPC(DWORD /*dwData*/)
	{
		// intentionally does nothing
	}

	// Get the Windows version.
	static const DWORD dwVersion = ::GetVersion();
	static const DWORD dwMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
	static const bool bIsNT4orLater =  (dwVersion < 0x80000000) // Windows NT/2K
		                            && (dwMajorVersion >= 4);
	static const bool bIsNT3 =  (dwVersion < 0x80000000 && dwMajorVersion < 4);


#else // !WIN32
	#include <sys/time.h>
	#include <errno.h>
#endif //WIN32

#include <algorithm>

#ifdef QC_MT

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// ConditionVariable::ConditionVariable
//
/**
   Constructs a ConditionVariable.
*/
//==============================================================================
ConditionVariable::ConditionVariable()
{
#if defined(QC_WIN32_THREADS)

	m_hBroadcastEvent = ::CreateEvent(0,     // no security
	                                  TRUE,  // manual-reset event
	                                  FALSE, // non-signalled initially
	                                  0);    // unnamed

	if(m_hBroadcastEvent == NULL)
		throw Win32Exception(::GetLastError(), QC_T("CreateEvent"));

#elif defined(QC_POSIX_THREADS)

	int status = ::pthread_cond_init(&m_cv, 0);
	if(status != 0) throw OSException(status, QC_T("pthread_cond_init"));

#endif
}

//==============================================================================
// ConditionVariable::~ConditionVariable
//
/**
   Releases system resources associated with this ConditionVariable.
*/
//==============================================================================
ConditionVariable::~ConditionVariable()
{
#if defined(QC_WIN32_THREADS)

	::CloseHandle(m_hBroadcastEvent);

#elif defined(QC_POSIX_THREADS)

	::pthread_cond_destroy(&m_cv);

#endif
}

//==============================================================================
// ConditionVariable::wait
//
/**
   Waits and blocks the calling thread until the ConditionVariable becomes
   signalled.

   This method allows a thread to wait  for  a condition  and  atomically release 
   the associated mutex that it needs to hold to check the condition.  The thread
   waits for another thread to make the condition true and that
   thread's resulting call to @c signal() or @c broadcast().
 
   Upon completion, the mutex is locked and owned by the calling thread.

   On most occasions, control is not returned to the caller until another thread
   has called signal() or broadcast() on this ConditionVariable.  However, it is
   possible for <i>spurious wake-ups</i> to occur.  For this reason, the caller
   should always check the condition in a @c while loop.

   @param mutex the mutex object associated with the condition.

   @sa signal()
   @sa broadcast()
*/
//==============================================================================
void ConditionVariable::wait(RecursiveMutex& mutex)
{
	if(!mutex.isLocked()) throw IllegalMonitorStateException();

#if defined(QC_WIN32_THREADS)

	waitImplWin32(mutex, INFINITE);

#elif defined(QC_POSIX_THREADS)

	int recursionCount = mutex.getRecursionCount();
	mutex.preWait();
	int status = ::pthread_cond_wait(&m_cv, &mutex.m_mutex.m_mutex);
	mutex.postWait(recursionCount);
	if(status != 0) throw OSException(status, QC_T("pthread_cond_wait"));

#endif
}

//==============================================================================
// ConditionVariable::wait
//
/**
   Waits and blocks the calling thread until the ConditionVariable becomes
   signalled or the timeout value expires.

   This method allows a thread to wait  for  a condition  and  atomically release 
   the associated mutex that it needs to hold to check the condition.  The thread
   waits for another thread to make the condition true and that
   thread's resulting call to @c signal() or @c broadcast().
 
   @par
   Upon completion, the mutex is locked and owned by the calling thread.

   @param mutex the mutex object associated with the condition.
   @param milliseconds the number of milliseconds to wait before timing-out.

   @returns true if the ConditionVariable was signalled; false if timed-out
   @throws InterruptedException if the current thread is interrupted.  The mutex
           is re-acquired before throwing this exception.
   @sa wait()
   @sa signal()
   @sa broadcast()
   @sa Thread::interrupt()
*/
//==============================================================================
bool ConditionVariable::wait(RecursiveMutex& mutex, unsigned long milliseconds)
{
	if(!mutex.isLocked()) throw IllegalMonitorStateException();

#if defined(QC_WIN32_THREADS)

	return waitImplWin32(mutex, milliseconds);

#elif defined(QC_POSIX_THREADS)

	//
	// Compute the absolute time that the wait should time-out
	//

	struct timespec abstime;
	struct timeval tv;
	static const long NANOSECSINONE = 1000000000;

	::gettimeofday(&tv, 0);

	//
	// The UNIX timeval represents the number of seconds since the epoch
	// and the number of micro-seconds elapsed in that second.
	//
	// The POSIX timespec uses seconds and nanoseconds, but with the
	// same epoch.
	//
	abstime.tv_sec = tv.tv_sec + (milliseconds / 1000);
	abstime.tv_nsec = (tv.tv_usec * 1000) + ((milliseconds % 1000) * 1000000);
	// Nanoseconds must not be greater than one second on Solaris
    if (abstime.tv_nsec > NANOSECSINONE)
    {
		++abstime.tv_sec;
		abstime.tv_nsec -= NANOSECSINONE;
    }

	int recursionCount = mutex.getRecursionCount();
	mutex.preWait();
	int status = ::pthread_cond_timedwait(&m_cv, &mutex.m_mutex.m_mutex, &abstime);
	mutex.postWait(recursionCount);

	bool bRet = true;
	switch(status)
	{
	case ETIMEDOUT: // timeout expired
		bRet = false;
	case 0:         // cv signalled
		break;
	case EINTR:     // interrupted by a signal
		throw InterruptedException();
	default:
		throw OSException(status, QC_T("pthread_cond_timedwait"));
	}
	return bRet;
#endif
}

//==============================================================================
// ConditionVariable::signal
//
/**
   Awakens one of the threads that are waiting on this condition variable.
   
   If no threads are waiting on this condition variable,  nothing happens.
   If several threads are waiting, exactly one is woken, but it is not
   specified which one.

   When a thread is awakened, it re-acquires the mutex that was released
   by the wait() call before returning to the application.

   @sa wait()
*/
//==============================================================================
void ConditionVariable::signal()
{
#if defined(QC_WIN32_THREADS)
	
	HANDLE hSignalThread = 0;

	m_waitersLock.lock();
	if(!m_waitersQueue.empty())
	{
		hSignalThread = m_waitersQueue.front();
		m_waitersQueue.pop_front();
	}
	m_waitersLock.unlock();

	if(hSignalThread)
	{
		signalImplWin32(hSignalThread);
	}

#elif defined(QC_POSIX_THREADS)

	int status = ::pthread_cond_signal(&m_cv);
	if(status != 0) throw OSException(status, QC_T("pthread_cond_signal"));

#endif
}

//==============================================================================
// ConditionVariable::broadcast
//
/**
   Awakens all threads that are waiting on this condition variable.

   Every awakened thread will compete to re-acquire the mutex that it had locked
   prior to entering wait().
*/
//==============================================================================
void ConditionVariable::broadcast()
{
#if defined(QC_WIN32_THREADS)

	//
	// See the function waitImplWin32() for a complete description of what
	// needs to be done for each OS version.
	// In short, the preferred broadcast method is unreliable on Win95/98/NT3
	// so we have to signal every thread in the waiters queue.
	// For Windows 95/98 we queue a UserAPC to the thread, for NT3 we
	// signal the event object that has kindly been created for the purpose.
	//
	if(bIsNT4orLater)
	{
		m_waitersLock.lock();
		bool bHaveWaiters = !m_waitersQueue.empty();
		m_waitersLock.unlock();

		if(bHaveWaiters)
		{
			if(!::PulseEvent(m_hBroadcastEvent))
				throw Win32Exception(::GetLastError(), QC_T("PulseEvent"));
		}
	}
	else // <NT4.0
	{
		// Lock the queue for the duration
		FastMutex::Lock lock(m_waitersLock);
		while(!m_waitersQueue.empty())
		{
			HANDLE hSignalThread = m_waitersQueue.front();
			m_waitersQueue.pop_front();
			signalImplWin32(hSignalThread);
		}
	}

#elif defined(QC_POSIX_THREADS)

	int status = ::pthread_cond_broadcast(&m_cv);
	if(status != 0) throw OSException(status, QC_T("pthread_cond_broadcast"));

#endif
}

#if defined(QC_WIN32_THREADS)
//==============================================================================
// ConditionVariable::waitImplWin32
//
// Win32 implementation of wait()
//
// A discussion regarding thread priority and signalling
// -----------------------------------------------------
// Most os implementations of signal() will wake up high prioirity threads
// before lower priority ones.  This implementation uses a FIFO dispatching
// without regard to thread priority.  This may change in the future, but
// here is a brief explanation of the design decisions:
// 
// One approach would be to make the waiters list ordered in priority/fifo
// order instead of fifo order.  This would involve either changing the queue
// entries to include the waiting thread's priority or obtaining the priority
// for each existing handle whenever a new entry is added.  The former is likely
// to be more efficient.  Each insertion to the queue would then be placed
// in front of the first entry with a lower priority than the current thread.
// This places an overhead on each waiter to do an ordered insert into the queue.
//
// The other approach is for the signal() function to traverse the queue looking
// for the thread with the highest priority.  Like above, this could be achieved
// by adding priority into the queue entry, or by obtaining it dynamically.  The
// former is likely to be more efficient, with the trade-of that dynamically changing
// the priority will not be reflected in the thread's wait priority if it is already
// waiting.
// 
// So, no matter which way we do it, posting in priority sequence adds overhead
// to an already overweight implementation.  One possible down-side of
// priority-based signalling is that lower-priority threads may never get
// woken until a broadcast occurs.  This could lead to priority-induced deadlock,
// but that is really an application design fault and does not effect the decision
// not to implement prioirty-based signalling.
//
// @returns true if signalled; false if timeout expires
//==============================================================================
bool ConditionVariable::waitImplWin32(RecursiveMutex& mutex, DWORD timeout)
{
	//
	// The Windows operating systems have slowly evolved to the point where
	// they can provide adequate synchronization primitives to support
	// condition variable semantics.  However, versions prior to NT 4.0
	// are sadly lacking in various respects.
	//
	// Windows 95 & NT3.x do not support QueueUserAPC()
	// Windoes 95/98 & NT3.x do not support SignalObjectAndWait()
	//
	// A correct and efficient implementation requires both of these calls, so our
	// ideal solution (which still isn't ideal - it would benefit from
	// SignalObjectandWaitMultipe - but it's okay) is only available for NT4.0 and
	// above.
	//
	// So what is the ideal solution you say?  I'll explain...
	//
	// NT4.0 and Above
	// ===============
	// We have an atomic "signal and wait" but it can only wait on one event.
	// This is okay for broadcast or signal but not both.  We have the choice of
	// only implementing one, or seeking another method for the other.  The
	// solution is to use a broadcast event which is waited upon, and QueueUserAPC
	// for signalling an individual thread.  How does signal() know which threads are
	// waiting? Simple, the waiter adds his handle to a queue, which is done atomically
	// because it is covered by the mutex which is held prior to SignalObjectAndWait.
	//
	// Windows 95/98
	// =============
	// Windows 95/98 does not have SignalObjectAndWait, so we don't have a reliable
	// broadcast mechanism. signal() will still work okay because the user APC will
	// be queued until the WaitForSingleObjectEx is executed.
	// 
	// So, the cure for this is quite straight-forward: the broadcast event doesn't
	// need to ever be signalled, but broadcast() for Win95/98 should iterate over the
	// waiters list, signalling each one (via QueueUserAPC), instead of pulsing the
	// broadcast event.
	//
	// Windows NT 3.x
	// ==============
	// Poor old Windows NT3.x.  Not only no SignalObjectAndWait, but no 
	// QueueUserAPC either.
	// The solution here is a little extreme.  Rather than populating the waiters
	// list with thread handles, we populate it with newly created Event handles.
	// Each waiter gets its own event - which is rather unattractive but at least
	// it doesn't suffer from any race conditions (that I'm aware of).
	//
	
	HANDLE hWaiter; // The waiter's handle.  Normally a thread handle except for
	                // NT3 when it is an auto-reset event handle

	bool bHandleCloseReqd = false;

	if(bIsNT3)
	{
		// Create an Event object for this old-fashioned operating system
		hWaiter = ::CreateEvent(0, FALSE, FALSE, 0);
		if(hWaiter == NULL)
		{
			throw Win32Exception(::GetLastError(), QC_T("CreateEvent"));
		}

		bHandleCloseReqd = true;
	}
	else // Win98, NT4 etc
	{
		AutoPtr<Thread> rpThread = Thread::CurrentThread();
		if(rpThread)
		{
			hWaiter = rpThread->getWin32Handle();
		}
		else
		{
			// Obtain a useable thread handle
			// for this (unmanaged) thread
			if(!::DuplicateHandle(
				::GetCurrentProcess(),
				::GetCurrentThread(),
				::GetCurrentProcess(),
				&hWaiter,
				THREAD_SET_CONTEXT,
				FALSE,
				0))
			{
				throw Win32Exception(::GetLastError(), QC_T("DuplicateHandle"));
			}
			bHandleCloseReqd = true;
		}
	}

	//
	// Add our thread's handle to the waiters queue
	//
	m_waitersLock.lock();
	m_waitersQueue.push_back(hWaiter);
	m_waitersLock.unlock();

	bool bInterrupted = false;
	bool bSignalled = false;

	//
	// Prepare the RecursiveMutex for the wait
	//
	int recursionCount = mutex.getRecursionCount();
	mutex.preWait();

	//
	// Atomically unlock the external mutex, and wait on the broadcast event
	//
	DWORD waitResult;

	//
	// SignalObjectAndWait() is not available on Win9x or WinNT < 4.0
	// so we need to check before calling it.
	//
	#if(_WIN32_WINNT >= 0x0400)

	// Get the Windows version.

	if(bIsNT4orLater)
	{
		waitResult = ::SignalObjectAndWait(mutex.m_mutex.m_hMutex, m_hBroadcastEvent, timeout, TRUE);
	}
	else

	#endif //(_WIN32_WINNT >= 0x0400)

	{
		//
		// The Mutex release and wait are not atomic, but that is catered
		// for by using the events in the waiters queue
		//
		if(!::ReleaseMutex(mutex.m_mutex.m_hMutex))
			throw Win32Exception(::GetLastError(), QC_T("ReleaseMutex"));

		waitResult = ::WaitForSingleObjectEx(hWaiter, timeout, TRUE);
	}

	switch(waitResult)
	{
	case WAIT_OBJECT_0:      // broadcast
	case WAIT_IO_COMPLETION: // signal or interrupt (we'll find out which later)
		bSignalled = true;
		break;
	case WAIT_ABANDONED_0:   // owner terminated
	case WAIT_TIMEOUT:       // timeout value exceeded
		break;
	default:
		throw Win32Exception(::GetLastError(), QC_T("SignalObjectAndWait"));
	}

	// re-acquire the mutex
	if(::WaitForSingleObject(mutex.m_mutex.m_hMutex, INFINITE) == WAIT_FAILED)
		throw Win32Exception(::GetLastError(), QC_T("WFSO"));

	// reset the RecursiveMutex state members
	mutex.postWait(recursionCount);

	//
	// Remove our HANDLE from the waiters list
	// If we were signalled then it will have been removed for us
	// Otherwise, a WAIT_IO_COMPLETION result indicates that we were interrupted
	// for some other reason.
	//
	m_waitersLock.lock();
	WaitersQueue::iterator i = std::find(m_waitersQueue.begin(), m_waitersQueue.end(), hWaiter);
	if(i != m_waitersQueue.end())
	{
		if(waitResult == WAIT_IO_COMPLETION)
			bInterrupted = true;
		else
			m_waitersQueue.erase(i);
	}
	m_waitersLock.unlock();

	//
	// If we created the handle, then close it now
	//
	if(bHandleCloseReqd)
	{
		::CloseHandle(hWaiter);
	}

	//
	// Only after re-acquiring the mutex and releasing resources
	// are we able to throw an InterruptedExeption if required.
	//
	if(bInterrupted)
	{
		throw InterruptedException();
	}

	return bSignalled;
}

//==============================================================================
// ConditionVariable::signalImplWin32
//
// Private helper function to signal an individual thread.
// 
// The hWaiter HANDLE is either a thread handle (Win95/98/NT4 and above)
// or an event handle (NT3.x).
//==============================================================================
void ConditionVariable::signalImplWin32(HANDLE hWaiter)
{
	if(bIsNT3)
	{
		if(!::SetEvent(hWaiter))
			throw Win32Exception(::GetLastError(), QC_T("SetEvent"));
	}
	else
	{
		DWORD result = ::QueueUserAPC(&DummyAPC, hWaiter, 0);
		// Apparently there is no ::GetLastError() info for this call
		if(!result) throw RuntimeException(QC_T("QueueUserAPC failed"));
	}
}

#endif //QC_WIN32_THREADS

QC_BASE_NAMESPACE_END

#endif //QC_MT

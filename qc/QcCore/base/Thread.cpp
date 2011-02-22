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
// Class: Thread
//
/**
	@class qc::Thread
	
	@brief Class to start and control @em threads of execution.

	@QuickCPP creates a simple abstraction for the concept of threads,
	providing wrapper classes to provide common functionality on
	disparate operating environments.

	An application may contain many threads, each one executing
	concurrently to perform a certain task.  Every thread has a priority.
	Where the underlying threading system supports it, threads with
	a higher priority are executed in preference to threads with a 
	lower priority.  The priority of a thread is initially set equal
	to the thread that created it.

    Not all threads will have a Thread object associated with them;
	only those created using an instance of the Thread class.
	For example, the thread that executes the @c main() function
	when an application starts is created by the operating system
	and does not have a Thread object.  Calls to CurrentThread()
	will therefore yield a null reference for this thread.

	When an application terminates by returning from @c main(), 
	the operating system terminates any threads that are still
	executing.  The Thread class creates two types of thread: @em daemon
	threads and @em user threads.  The only distinction between the two
	is that user threads are required to terminate before the application
	terminates whereas daemon threads may be terminated automatically
	by the operating system.
	
	@em Applications should include an instance of SystemMonitor in their 
	@c main() function to ensure System::Terminate() is called at exit.
	In addition to its other tasks, System::Terminate() checks that
	all @em user threads have terminated before proceeding.
	
    There are two ways to create a new thread of execution. One is to 
    declare a class derived from @c Thread and override the virtual
	@c run() method.  An instance of this class can then be created
	and the @c start() method called.  For example, here is a function
	that uses a specialized thread to simply print @c "Hello World!"
	to the console:
	
    @code
    class HelloThread : public Thread {
    public:
        virtual void run() {
            Console::cout() << QC_T("Hello World!");
        }
    };

    void hello() {
        AutoPtr<Thread> rpThread = new HelloThread;
        rpThread->start();
        rpThread->join(); // wait for thread termination
    }
    @endcode
	  
	The other way is to declare a class derived from
	@c Runnable which has a single virtual @c run() method that
	should be overridden to perform the required task.  An instance
	of this class can then be passed to a Thread's constructor
	and the thread will call the @c run() method for that class. See Runnable
	for an example of this method.

	Due to its nature, a Thread instance is likely to be accessed from more
	than one thread.  For this reason, all access to member state information
	is synchronized so that it may safely be accessed by multiple concurrent
	threads.

	<hr><h4>Reference Counting and Thread objects</h4>
	When a Thread is started, a AutoPtr<Thread> for the Thread is added
	to a list of active threads.  On termination this entry is removed from
	the list, with the result that the Thread's reference count will be 
	decremented.  One effect of this is that a Thread instance is
	guaranteed to remain valid for as long as the OS thread is active - which
	you must agree makes perfect sense.  Another effect will be that the Thread
	instance will be destroyed automatically when the Thread terminates unless
	the application	has incremented its reference count (for example by storing
	the Thread's address in a AutoPtr<Thread> when the instance was created).
	
	A Thread object should never be allocated on the stack as stack-based 
	objects are not consistent with the semantics of reference-counting.
*/
//==============================================================================

#include "Thread.h"
#include "Tracer.h"
#include "IllegalArgumentException.h"
#include "IllegalThreadStateException.h"
#include "InterruptedException.h"
#include "NumUtils.h"
#include "OSException.h"
#include "UnsupportedOperationException.h"

#ifdef WIN32
	#include "Win32Exception.h"
	#include <process.h>
#else // !WIN32
	#include <sched.h>
	#include <unistd.h>
#endif //WIN32

#include <signal.h>
#include <errno.h>
#include <time.h>

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// Thread::Sleep
//
/**
   Suspends execution of the currently executing thread for @c millis milliseconds
   or until the current thread is interrupted.

   This is a static function that can be called from any thread, even from those
   threads that are not created and controlled by a Thread object (such as the
   application's main thread).

   This function is also available in single-threaded versions of @QuickCPP.

   @param millis the number of milliseconds to sleep for
   @throws InterruptedException if another thread interrupted the current thread
   @sa interrupt()
*/
//==============================================================================
void Thread::Sleep(long millis)
{
	Thread::Sleep(millis, 0);
}

//==============================================================================
// Thread::Sleep
//
/**
   Suspends execution of the currently executing thread for a
   specified duration.

   There is no guarantee that the current thread will sleep for exactly
   the specified duration.  The Sleep() function may return earlier
   or later than the specified duration.

   This is a static function that can be called from any thread, even from those
   threads that are not created and controlled by a Thread object (such as the
   application's main thread).

   This function is also available in single-threaded versions of @QuickCPP.

   @param millis the number of milliseconds to sleep for
   @param nanos the number of nanoseconds to sleep for.  If the operating
          system does not support a nanosecond resolution, the number
		  of milliseconds is rounded up when appropriate.
   @throws InterruptedException if another thread interrupted the current thread
   @sa interrupt()
*/
//==============================================================================
void Thread::Sleep(long millis, long nanos)
{
	
#if defined(WIN32)
	
	if(nanos > 500000)
		millis++;

	if(::SleepEx(millis, TRUE) == WAIT_IO_COMPLETION)
		throw InterruptedException();
	
#elif defined(HAVE_NANOSLEEP)
    
	struct timespec tv;
	tv.tv_sec = millis/1000;
	tv.tv_nsec = (millis% 1000)*1000000 + nanos;
	if(::nanosleep(&tv, 0) < 0)
	{
		if(errno == EINTR)
			throw InterruptedException();
		else
			throw OSException(errno, QC_T("nanosleep"));
	}

#else
	//
	// if nothing else is available then sleep() will have to do
	// note that this returns the number of seconds left to go
	// when interrupted by a signal
	//

	if(::sleep(millis/1000) != 0)
		throw InterruptedException();

#endif
}



//==============================================================================
//
// MULTI-THREADED LIBRARY FEATURES ONLY FOLLOWING THIS POINT
//
//==============================================================================
#ifdef QC_MT

FastMutex          ThreadListMutex;
Thread::ThreadList Thread::s_activeThreadList;
AtomicCounter      Thread::s_nextThreadNumber;
ThreadLocal        Thread::s_thisPointer;
int                Thread::s_interruptSignal = 0;

#ifndef QC_DOCUMENTATION_ONLY
//==============================================================================
/**
   Thread start-up function used by the @QuickCPP Thread class.

   This is the function that is executed when an OS thread starts executing.
   It is the equivalent of main() in a non-threaded program.

   User code should never call this function directly.
*/
//==============================================================================
void* QuickCPPThreadFunc(void* pArg)
{
    //
    // pArg is a pointer to the Thread object
    //
	QC_DBG_ASSERT(pArg!=0);

	//
	// Hoild a reference to the Thread ptr forthe duration of
	// the thread.
	// Note, this reference is explicity released in cancel().
	//
    AutoPtr<Thread> rpThread = (Thread*)pArg;
	if(rpThread)
	{
		rpThread->doRun();
	}

	return 0;
}
#endif //QC_DOCUMENTATION_ONLY

//==============================================================================
// Thread::doRun
//
// Private helper function.
//==============================================================================
void Thread::doRun()
{
	// Guaranteed not to throw exceptions...
	QC_DBG_ASSERT(m_state == Active);

	if(Tracer::IsEnabled())
	{
		String traceMsg = QC_T("starting thread: ");
		traceMsg += getName();
		Tracer::Trace(Tracer::Base, Tracer::High, traceMsg);
	}

	try
	{
		s_thisPointer.set(this);
		run();
	}
	catch(Exception& e)
	{
		Tracer::Trace(Tracer::Base, Tracer::Exceptions, e.toString());
	}
	catch(...)
	{
		Tracer::Trace(Tracer::Base, Tracer::Exceptions, QC_T("Untrapped system exception"));
	}

	if(Tracer::IsEnabled())
	{
		String traceMsg = QC_T("stopping thread: ");
		traceMsg += getName();
		Tracer::Trace(Tracer::Base, Tracer::High, traceMsg);
	}

	setState(Terminated);

	//
	// The thread is no longer active, so it must be removed from
	// the active thread list.  This also has the effect of decrementing the
	// reference count, but never below 1 because the threadFunc is still holding
	// a reference to this object
	//
	{	// create scope for lock
		AutoLock<FastMutex> threadListLock(ThreadListMutex);
		s_activeThreadList.remove(this);
	}
}

//==============================================================================
// Thread::Thread
//
/**
   Constructs a Thread using this as the Runnable object and with an
   automatically generated name.

   This constructor is intended to be used from derived classes which
   have overridden the run() method to do something useful.

   Automatically generated names are of the form "Thread-"+n, where n is an
   integer. 

   @sa run()
*/
//==============================================================================
Thread::Thread() :
	m_state(Initial),
	m_name(GenerateName())
{
	init();
}

//==============================================================================
// Thread::Thread
//
/**
   Constructs a Thread with the specified name, using this as the Runnable
   object.

   This constructor is intended to be used from derived classes which
   have overridden the run() method to do something useful.

   @param name the name to be given to the thread
   @sa run()
*/
//==============================================================================
Thread::Thread(const String& name) :
	m_state(Initial),
	m_name(name)
{
	init();
}

//==============================================================================
// Thread::Thread
//
/**
   Constructs a thread with @c pTarget as the Runnable object and with an
   automatically generated name.

   @param pTarget the Runnable object whose run() method will be called
   in the context of the new thread, when the Thread is started.

   @sa start()
   @sa Runnable::run()
*/
//==============================================================================
Thread::Thread(Runnable* pTarget) :
	m_state(Initial),
	m_name(GenerateName()),
	m_rpRunnable(pTarget)
{
	init();
}

//==============================================================================
// Thread::Thread
//
/**
   Constructs a Thread with the specified name and Runnable object.

   @param pTarget the Runnable object whose run() method will be called
   in the context of the new thread, when the Thread is started.
   @param name the name for the new Thread

   @sa start()
   @sa Runnable::run()
*/
//==============================================================================
Thread::Thread(Runnable* pTarget, const String& name) :
	m_state(Initial),
	m_name(name),
	m_rpRunnable(pTarget)
{
	init();
}

//==============================================================================
// Thread::~Thread
//
/**
   Releases resources associated with the Thread.
*/
// The POSIX pthread_t or the WIN32 HANDLE for the thread are kept for as
// long as the Thread object exists (or until join() for POSIX).  This is 
// necessary if the thread needs to be controlled during its execution
// (e.g. changing its priority or whatever).  However, these represent
// valuable system resources, so they are freed here in the destructor.
//
//==============================================================================
Thread::~Thread()
{
	// Resources are only acquired when the thread is started, so a test
	// is made to check that the thread isn't still in its initial state
	// (ie has never been started)

	if(m_state != Initial)
	{
#if defined(QC_WIN32_THREADS)

		// handles created with _beginthreadex must be explicitly closed
		::CloseHandle(m_hThread);

#elif defined(QC_POSIX_THREADS)

		if(!m_bJoined)
		{
			::pthread_detach(m_threadId.getNativeId());
		}

#endif
	}
}

//==============================================================================
// Thread::init
//
// Private helper function to set a Thread's priority and daemon status
//==============================================================================
void Thread::init()
{
	QC_DBG_ASSERT(m_state == Initial);

#if defined(QC_POSIX_THREADS)
	m_bJoined = false;
#endif //QC_POSIX_THREADS

	AutoPtr<Thread> rpCurrent = CurrentThread();
	if(rpCurrent)
	{
		m_bDaemon = rpCurrent->isDaemon();
		m_priority = rpCurrent->getPriority();
	}
	else
	{
		m_bDaemon = false;
		m_priority = NormPriority;
	}
}

//==============================================================================
// Thread::run
//
/**
	If this thread was constructed using a separate Runnable object, then
	that Runnable object's @c run() method is called; otherwise, this method does
	nothing and returns. 

    Derived classes of Thread should override this method to do something useful.

    This method runs in the context of a newly created thread of execution.  The 
	thread of execution is terminated when this method returns.
*/
//==============================================================================
void Thread::run()
{
	QC_DBG_ASSERT(m_rpRunnable);
	QC_DBG_ASSERT(CurrentThread().get() == this);
	if(m_rpRunnable)
	{
		m_rpRunnable->run();
	}
}

//==============================================================================
// Thread::join
//
/**
   Waits for this Thread to terminate.

   @throws InterruptedException if the waiting thread is interrupted
*/
//==============================================================================
void Thread::join()
{
	join(0);
}

//==============================================================================
// Thread::join
//
/**
   Waits up to @c millis milliseconds for this Thread to terminate.

   @param millis the number of milliseconds to wait.  A value of zero
          means wait @em forever
   @throws InterruptedException if the waiting thread is interrupted
*/
//==============================================================================
void Thread::join(long millis)
{
#if defined(QC_WIN32_THREADS)

	DWORD timeout = millis;
	if(0 == timeout) timeout = INFINITE;
	DWORD result = ::WaitForSingleObjectEx(m_hThread, timeout, TRUE);
	switch (result)
	{
    case WAIT_IO_COMPLETION:
		throw InterruptedException();
	case WAIT_TIMEOUT:
	case WAIT_OBJECT_0:
		break;
	default:
		throw Win32Exception(::GetLastError(), QC_T("unable to join thread"));
	}

#elif defined(QC_POSIX_THREADS)

	// POSIX threads do not allow a timeout on pthread_join, so we have to 
	// use something else instead.  This is why Thread is a Monitor,
	// so that we can wait on a change of state.
	QC_SYNCHRONIZED

	while(m_state != Terminated)
	{
		if(millis)
		{
			wait(millis);
			break;
		}
		else
		{
			wait();
		}
	}

	//
	if(m_state == Terminated)
	{
		::pthread_join(m_threadId.getNativeId(), 0);
		m_bJoined = true;
	}

#endif
}

//==============================================================================
// Thread::GenerateName
//
/**
   Generate a unique name for a Thread

   @returns a new name in the form "Thread-" + n
*/
//==============================================================================
String Thread::GenerateName()
{
	long threadNumber = ++s_nextThreadNumber;
	return String(QC_T("Thread-")) + NumUtils::ToString(threadNumber);
}

//==============================================================================
// Thread::getName
//
/**
   Returns the name of this Thread.

   @returns the name of this Thread.
*/
//==============================================================================
String Thread::getName() const
{
	QC_SYNCHRONIZED

	return m_name;
}

//==============================================================================
// Thread::isActive
//
/**
   Tests to see if this Thread is active.

   A Thread is active from the point in time that start() is called until
   after the run() method returns.

   @returns true if the Thread is active; false otherwise
*/
//==============================================================================
bool Thread::isActive() const
{
	QC_SYNCHRONIZED

	return (m_state == Active);
}

//==============================================================================
// Thread::isDaemon
//
/**
   Tests whether this Thread is a daemon thread.

   @returns true if this Thread is a daemon thread; false otherwise
   @sa setDaemon()
*/
//==============================================================================
bool Thread::isDaemon() const
{
	return m_bDaemon;
}

//==============================================================================
// Thread::setDaemon
//
/**
   Marks this thread as either a daemon thread or a user thread.

   This method must be called before the thread is started.

   An application exits when the only threads left executing are daemon threads.
   This policy is enforced by the System::Terminate() function, which is called
   by SystemMonitor's destructor.

   Some care should be taken when deciding whether or not to employ daemon threads.
   The advantage of using a daemon thread is that the application doesn't need to
   worry about terminating it, but the flip-side is that the thread could be terminated
   at any point - even when it is doing important work.  Another draw-back with using
   daemon threads is that memory resources used by the thread may not be freed prior
   to system termination - which could result in memory diagnostic tools indicating
   a memory leak in the application.

   @throws IllegalThreadStateException if this method is called after the Thread
   has been started.
   @sa isDaemon()
   @sa System::Terminate()
*/
//==============================================================================
void Thread::setDaemon(bool bDaemon)
{
	QC_SYNCHRONIZED

	if(m_state != Initial) throw IllegalThreadStateException();
	m_bDaemon = bDaemon;
}

//==============================================================================
// Thread::setState
//
// Private helper function.
//==============================================================================
void Thread::setState(State state)
{
	QC_SYNCHRONIZED

	m_state = state;
	//
	// It's possible that joiners will be waiting for a notification
	// that we have terminated.  If so, let them know.
	//
	if(state == Terminated)
	{
		notifyAll();
	}
}

//==============================================================================
// Thread::setName
//
/**
   Sets the name of this Thread.

   @sa getName()
*/
//==============================================================================
void Thread::setName(const String& name)
{
	QC_SYNCHRONIZED

	m_name = name;
}

//==============================================================================
// Thread::start
//
/**
   
   Causes the Thread to begin execution.
   
   A native thread is created using the underlying threading mechanism (e.g. Win32
   threads or POSIX threads), which is then directed to execute this Thread's
   run() method.

   @throws IllegalThreadStateException if the Thread has been started previously.
*/
//==============================================================================
void Thread::start()
{
	//
	// This is synchronized for a number of reasons:
	// i) It must check the state to ensure that it is not started multiple times
	// ii) It must set the threadId, which though imutable after the thread is started,
	//     could be obtained by the child thread before this thread regains control.
	//
	QC_SYNCHRONIZED

	if(m_state != Initial) throw IllegalThreadStateException();

	//
	// To prevent this Thread being destroyed before QuickCPPThreadFunc gains control
	// and to make this thread available to GetActiveThreads() we add a 
	// reference to the static list.
	// 
	// The reference will be removed (and thus decremented) on termination
	//
	// Create a scope for the global thread mutex lock
	// [beware: we will have two mutexes locked, ensure access to the global
	//  mutex always follows access to the synchronized mutex]
	{
		AutoLock<FastMutex> lock(ThreadListMutex);
		s_activeThreadList.push_back(this);
	}

	m_state = Active;

#if defined(QC_WIN32_THREADS)

	//
	// Under Win32, we create the thread initially suspended.  This is so that we can set
	// the thread HANDLE, and not have to synchromize access to the handle when the
	// thread starts to execute.
	//
	unsigned id;
	m_hThread = (HANDLE) ::_beginthreadex(0, 0, (unsigned (__stdcall*)(void*))QuickCPPThreadFunc, this, CREATE_SUSPENDED, &id);
	m_threadId = (DWORD)id;
	if(m_hThread == INVALID_HANDLE_VALUE)
	{
		throw OSException(errno, QC_T("beginthreadex"));
	}
	if(::ResumeThread(m_hThread) == (DWORD)-1)
	{
		throw Win32Exception(::GetLastError(), QC_T("ResumeThread"));
	}

#elif defined(QC_POSIX_THREADS)

	pthread_t id;
	int status = ::pthread_create(&id, 0, QuickCPPThreadFunc, this);
	if(status != 0)
	{
		throw OSException(status, QC_T("pthread_create"));
	}
	m_threadId = id;
#endif

	//
	// Finally, set the execution priority of the spawned thread
	//
	setExecutionPriority();
}

//==============================================================================
// Thread::Yield
//
/**
   Yields any remaining execution time-slice to another thread.

   This is a static function that can be called from any thread, even from those
   threads that are not created and controlled by a Thread object (such as the
   application's main thread).

   @note For historical reasons, Microsoft have created a
   macro in <winbase.h> called @c Yield.  If you are writing programs
   for the Microsoft Windows platform you will have to 
   <code>\#undef Yield</code>
   in order to call this function.
*/
//==============================================================================
void Thread::Yield()
{
#if defined(QC_WIN32_THREADS)
	
	::Sleep(0);

#elif defined(QC_POSIX_THREADS)
	
	//
	// according to the linux man page,
	// sched_yield is only available on those systems
	// with _POSIX_PRIORITY_SCHEDULING defined in <unistd.h>
	//
	#if defined(_POSIX_PRIORITY_SCHEDULING)

	int status = ::sched_yield();
	if(status != 0)
	{
		throw OSException(status, QC_T("sched_yield"));
	}

	#endif //_POSIX_PRIORITY_SCHEDULING

#endif
}

//==============================================================================
// Thread::CurrentThreadId
//
/**
   Returns a ThreadId representing the currently executing thread.
*/
//==============================================================================
ThreadId Thread::CurrentThreadId()
{
#if defined(QC_WIN32_THREADS)

	return ::GetCurrentThreadId();

#elif defined(QC_POSIX_THREADS)

	return ::pthread_self();

#else
	#error Unsupported configuration
#endif
}

//==============================================================================
// Thread::CurrentThread
//
/**
   Returns a reference to the Thread object for the currently executing thread.

   If the currently executing thread was not created using a Thread object
   (such as the thread executing the application's main() function), then
   a null reference is returned.

   @returns a reference to the currently executing Thread object, or a
            null reference if the executing thread was not created
            using a Thread object.
*/
//==============================================================================
AutoPtr<Thread> Thread::CurrentThread()
{
	Thread* pThis = static_cast<Thread*>(s_thisPointer.get());
	return pThis;
}

//==============================================================================
// Thread::operator
//
/**
   Equality operator.

   Returns true if @c rhs represents the same thread as this; false otherwise
*/
//==============================================================================
bool Thread::operator==(const Thread& rhs) const
{
	return (m_threadId == rhs.m_threadId);
}

//==============================================================================
// Thread::operator
//
/**
   Inequality operator.

   Returns true if @c rhs represents a different thread to this; false otherwise
*/
//==============================================================================
bool Thread::operator!=(const Thread& rhs) const
{
	return !(*this == rhs);
}

//==============================================================================
// Thread::GetActiveThreads()
//
/**
   Returns a list containing references to all the active Thread objects
   in the system.

   A Thread becomes active during the call to start() and ceases to be
   active after the run() method has completed and shortly before it is 
   terminated.

   @sa GetActiveCount()
*/
//==============================================================================
Thread::ThreadList Thread::GetActiveThreads()
{
	AutoLock<FastMutex> lock(ThreadListMutex);
	return s_activeThreadList;
}

//==============================================================================
// Thread::GetActiveCount
//
/**
   Returns the number of active Threads.

   This method simply returns the number of references in the active thread list
   returned by GetActiveThreads().  The return value may not equate
   with the number of threads running within the current process.
   
   A Thread becomes active during the call to start() and ceases to be
   active after the run() method has completed and shortly before it is 
   terminated by the operating system.

   @sa GetActiveThreads()
*/
//==============================================================================
size_t Thread::GetActiveCount()
{
	AutoLock<FastMutex> lock(ThreadListMutex);
	return s_activeThreadList.size();
}

//==============================================================================
// Thread::getId
//
/**
   Returns the ThreadId for this Thread.
*/
//==============================================================================
ThreadId Thread::getId() const
{
	// Even though the thread id is imutable, there is a possible race condition
	// between the creator thread setting the id and this thread executing.
	// By synchronizing, we ensure that we obtain the correct value as set by
	// Thread::start()
	//
	QC_SYNCHRONIZED

	return m_threadId;
}

//==============================================================================
// Thread::getPriority
//
/**
   Returns this Thread's priority.
   @sa setPriority()
*/
//==============================================================================
unsigned int Thread::getPriority() const
{
	QC_SYNCHRONIZED

	return m_priority;
}

//==============================================================================
// Thread::setPriority
//
/**
   Sets the Thread's execution priority.

   The priority is specified using an unsigned integer in the range
   Thread::MinPriority to Thread::MaxPriority.

   A thread's execution priority may be altered both before and during
   its execution.

   Not all operating systems support the setting of a thread's execution
   priority.  In this case calling this function does nothing.

   @throws IllegalArgumentException if @c priority is less than 
           Thread::MinPriority or greater than Thread::MaxPriority.
*/
//==============================================================================
void Thread::setPriority(unsigned priority)
{
	if(priority < MinPriority || priority > MaxPriority)
		throw IllegalArgumentException(QC_T("illegal priority value"));
	
	QC_SYNCHRONIZED

	if(m_priority != priority)
	{
		m_priority = priority;
		if(m_state == Active)
		{
			setExecutionPriority();
		}
	}
}

//==============================================================================
// Thread::setExecutionPriority
//
// Private helper function to set the priority of an existing thread
//==============================================================================
void Thread::setExecutionPriority()
{
#if defined(QC_WIN32_THREADS)

	//
	// Translate the priority into something meaningful for Win32
	//
	static int PriorityTable[10] =
	{ THREAD_PRIORITY_IDLE,
	  THREAD_PRIORITY_LOWEST,
	  THREAD_PRIORITY_BELOW_NORMAL,
	  THREAD_PRIORITY_BELOW_NORMAL,
	  THREAD_PRIORITY_NORMAL,
	  THREAD_PRIORITY_ABOVE_NORMAL,
	  THREAD_PRIORITY_ABOVE_NORMAL,
	  THREAD_PRIORITY_ABOVE_NORMAL,
	  THREAD_PRIORITY_HIGHEST,
	  THREAD_PRIORITY_TIME_CRITICAL
	};

	if(!::SetThreadPriority(m_hThread, PriorityTable[m_priority-1]))
	{
		throw Win32Exception(::GetLastError());
	}

// TODO: posix priority
#endif
}

#ifdef QC_WIN32_THREADS
//==============================================================================
// Thread::getThreadHandle
//
/**
   Returns a process-wide Win32 HANDLE for this Thread.
*/
//==============================================================================
HANDLE Thread::getWin32Handle() const
{
	// The handle is imutable and set before the thread is scheduled, so no need to
	// synchronize.
	return m_hThread;
}
#endif //QC_WIN32_THREADS

void Thread::WaitAllUserThreads()
{
	//
	// Wait for all user threads to terminate
	//
	bool bDone = false;
	while(!bDone)
	{
		bDone = true;
		Thread::ThreadList activeThreads = Thread::GetActiveThreads();
		for(Thread::ThreadList::iterator i=activeThreads.begin(); i!=activeThreads.end(); ++i)
		{
			if(!(*i)->isDaemon())
			{
				bDone = false;
				Thread::Sleep(100);
				break;
			}
		}
	}
}

void Thread::TerminateAllDaemonThreads()
{
	ThreadList activeList = GetActiveThreads();
	for(ThreadList::iterator i=activeList.begin(); i!=activeList.end(); ++i)
	{
		if((*i)->isDaemon())
		{
			((*i)->cancel());
		}
	}
}

//==============================================================================
// Thread::cancel
//
// Private helper method
//==============================================================================
void Thread::cancel()
{
	//
	// It is illegal to cancel ourself
	//
	if(getId() == CurrentThreadId())
	{
		throw IllegalThreadStateException();
	}

#ifdef QC_WIN32_THREADS

	::TerminateThread(m_hThread, 1);

#elif defined(QC_POSIX_THREADS)

	::pthread_cancel(m_threadId.getNativeId());

	// Wait for thread to terminate.  
	// Note: if the thread does not enter a cancellation point
	// this could wait indefinitely.  In fact, preliminary
	// tests have shown that a thread in pthread_cond_wait was not
	// cancelled!
	::pthread_join(m_threadId.getNativeId(), 0);

#else
	#error Unsupported configuration
#endif

	setState(Terminated);

	//
	// Finally, the reference held by QuickCPPThreadFunc will not be released
	// so we should do that here...

	release();

	// ..and as we are being forcably cancelled, we will not
	// get a chance to remove our reference from the active list
	// so we'll do it now.
	//
	AutoLock<FastMutex> threadListLock(ThreadListMutex);
	s_activeThreadList.remove(this);
}

#ifdef QC_WIN32_THREADS
	// This is the APC callback routine function for interrupting a thread
	VOID WINAPI InterruptAPC(DWORD /*dwData*/)
	{
		// intentionally does nothing
	}
#endif //QC_WIN32_THREADS

//==============================================================================
// Thread::interrupt
//
/**
   Interrupts this thread.

   If this thread is waiting in Sleep() or waiting on a ConditionVariable
   or Monitor, then it will be woken and receive an InterruptedException.
   
   On most UNIX platforms, blocking i/o calls can be interrupted, but this is not
   portable behaviour.
   
   Interruption is implemented differently on the various supported platforms. 

   <ul>
   <li>On Microsoft Windows platforms, interruption is implemented by queuing a 
   user APC request to the thread.  As APC de-queuing is an atomic operation
   performed during certain Win32 system calls, interruption is implemented
   reliably on Windows platforms.  
   <li>On UNIX platforms, interruption is achieved by sending a signal to the
   target thread.  As signals cannot be blocked and unblocked atomically with
   other system calls, a race condition can occur resulting in the signal being
   lost.
   </ul>
   
   Another complication with interrupting a UNIX thread is that certain i/o
   operations may be interrupted, with the result that they may throw some
   form of IOException.  These exceptions are rarely recoverable, so 
   interrupt() may only be useful when asking a thread to terminate.

   If your application is going to call interrupt() in the UNIX environment,
   you must choose which signal number to assign and call 
   SetInterruptSignal() during initialization.

   @note interrupt() is not supported on Windows NT 3.x or Mac OS X
   @throws UnsupportedOperationException when running Windows NT 3.x or
           a UNIX-like operating system which does not support pthread_kill()
   @throws RuntimeException when running under UNIX and SetInterruptSignal() has
           not been called to assign an interrupt signal number
   @sa interruptSupported()
   @sa SetInterruptSignal()
*/
//==============================================================================
void Thread::interrupt()
{
	//
	// Obtain synchronized access to our state variable.  Only if this Thread 
	// is active should it be interrupted
	//
	QC_SYNCHRONIZED

	if(m_state != Active)
		return;

#if defined(QC_WIN32_THREADS)

	// Get the Windows version.
	static const DWORD dwVersion = ::GetVersion();
	static const DWORD dwMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
	static const bool bIsNT3 =  (dwVersion < 0x80000000 && dwMajorVersion < 4);

	if(bIsNT3)
	{
		throw UnsupportedOperationException(QC_T("no interrupt support on NT3.x"));
	}
	else
	{
		DWORD result = ::QueueUserAPC(&InterruptAPC, m_hThread, 0);
		// Apparently there is no ::GetLastError() info for this call
		if(!result) throw RuntimeException(QC_T("QueueUserAPC failed"));
	}

#elif defined(QC_POSIX_THREADS)

	if(!s_interruptSignal)
	{
		throw RuntimeException(QC_T("interrupt signal number not set"));
	}

	#ifdef HAVE_PTHREAD_KILL

		int status = ::pthread_kill(m_threadId.getNativeId(), s_interruptSignal);
		if(status != 0) throw OSException(status, QC_T("pthread_kill"));

	#else

		throw UnsupportedOperationException(QC_T("no interrupt support on this platform"));

	#endif //HAVE_PTHREAD_KILL

#endif
}

//==============================================================================
// Thread::interruptSupported
//
/**
   Tests whether the interrupt() method is supported on the current
   platform.

   @returns true if the interrupt() is supported; false otherwise
   @sa interrupt()
   @since 1.1
*/
//==============================================================================
bool Thread::interruptSupported() const
{
#if defined(QC_WIN32_THREADS)

	// Get the Windows version.
	static const DWORD dwVersion = ::GetVersion();
	static const DWORD dwMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
	static const bool bIsNT3 =  (dwVersion < 0x80000000 && dwMajorVersion < 4);

	if(bIsNT3)
	{
		return false;
	}
	else
	{
		return true;
	}

#elif defined(QC_POSIX_THREADS)

	#ifdef HAVE_PTHREAD_KILL

		return true;

	#else

		return false;

	#endif //HAVE_PTHREAD_KILL

#endif
}

//==============================================================================
// Signals are software interrupts.  We don't use them on Windows platforms because
// APCs are easier to deal with.  However, under UNIX we can't avoid them - and
// they are useful for interrupting a slow command.
//
// Unix signal handling and multi-threading doesn't go well together.  One reason
// for this is that signal actions (the action taken when a signal is raised)
// are process-wide.  Another complication is that most signals have a default
// action that terminates the process.
//
// In order to co-exist happily with the application (which may have its own
// signalling requirements), we allow the application to nominate a signal
// for @QuickCPP to use.  Once the signal has been nominated it is an error
// for the application to set an alternative action for the signal.
//==============================================================================
#if !defined(WIN32) && !defined(QC_DOCUMENTATION_ONLY)
extern "C"
void CelInterruptSignalHandler(int /*sig*/)
{
	// do nothing!
}
#endif // !WIN32

//==============================================================================
// Thread::SetInterruptSignal
//
/**
   Specifies the signal number to use for interrupt processing in the UNIX
   environment.

   On UNIX/Linux platforms, @QuickCPP uses signalling to implement
   thread interruption.  This method installs a dummy signal handler for the
   specified signal.

   UNIX signal handling is process-wide and non-modular.  If one thread sets
   a signal handler, all threads get to use the same one.  For this reason
   signal handling is normally dealt with at the application level rather
   than by individual libraries.  This method gives applications the choice
   of which signal number to assign to @QuickCPP for thread interruption.
   
   If your application does not use the @c alarm() function then @c SIGALRM is 
   a good choice.  Note, though, that @c SIGALRM is not #defined in the
   Windows version of @c <signal.h>, so conditional code is required.
   For example:-

   @code
   #include "QcCore/base/SystemMonitor.h"
   #include "QcCore/base/Thread.h"
   #include <signal.h>
   using namespace qc;

   int main(int argc, char* argv[])
   {
        SystemMonitor monitor; // ensure correct termination
        // Assign an unused signal for UNIX thread interruption handling
        #ifdef SIGALRM
            Thread::SetInterruptSignal(SIGALRM);
        #endif
        ...
   }
   @endcode

   @param signo the signal number to be assigned to interrupt processing
   @sa interrupt()
*/
//==============================================================================
void Thread::SetInterruptSignal(int signo)
{
	if(!signo)
		throw IllegalArgumentException();

#ifndef WIN32

	struct sigaction mySigAction;
	mySigAction.sa_flags = 0;
	mySigAction.sa_handler = CelInterruptSignalHandler;
	sigemptyset(&mySigAction.sa_mask);
	sigaddset(&mySigAction.sa_mask, signo);
	if(::sigaction(signo, &mySigAction, 0) == -1)
		throw OSException(errno, QC_T("sigaction"));

#endif //WIN32

	s_interruptSignal = signo;
}

//==============================================================================
// Thread::GetInterruptSignal
//
/**
   Returns the signal number that @QuickCPP will use for thread interruption.

   @sa SetInterruptSignal()
   @sa interrupt()
*/
//==============================================================================
int Thread::GetInterruptSignal()
{
	return s_interruptSignal;
}

#endif //QC_MT

QC_BASE_NAMESPACE_END

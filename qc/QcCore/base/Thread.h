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
//==============================================================================

#ifndef QC_BASE_Thread_h
#define QC_BASE_Thread_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include "Runnable.h"
#include "AtomicCounter.h"
#include "ConditionVariable.h"
#include "FastMutex.h"
#include "AutoPtr.h"
#include "RecursiveMutex.h"
#include "String.h"
#include "Monitor.h"
#include "ThreadLocal.h"
#include "ThreadId.h"

#include <list>

QC_BASE_NAMESPACE_BEGIN

#ifndef QC_DOCUMENTATION_ONLY
extern "C" void* QuickCPPThreadFunc(void*);
#endif //QC_DOCUMENTATION_ONLY

class QC_BASE_PKG Thread : public Runnable, public Monitor
{
	friend class System;
	friend void* QuickCPPThreadFunc(void*);

public: // static functions available in single-threaded versions

	static void Sleep(long millis);
	static void Sleep(long millis, long nanos);

	//
	// Everything from here is only present in multi-threaded
	// versions of the library
	//
#ifdef QC_MT

protected: // constructors for use by derived classes

	Thread();
	Thread(const String& name);

public:
	typedef std::list< AutoPtr<Thread> > ThreadList;
	enum { MinPriority=1, NormPriority=5, MaxPriority=10 };

	Thread(Runnable* pTarget);
	Thread(Runnable* pTarget, const String& name);

	~Thread();

	ThreadId getId() const;
	String getName() const;
	unsigned getPriority() const;

	bool isActive() const;
	bool isDaemon() const;

	void join();
	void join(long millis);

	void setPriority(unsigned priority);
	void setDaemon(bool bDaemon);
	void setName(const String& name);
	void interrupt();
	bool interruptSupported() const;

	void start();
	
	bool operator==(const Thread& rhs) const;
	bool operator!=(const Thread& rhs) const;

	// From Runnable...
	virtual void run();

#if defined(QC_WIN32_THREADS)
	HANDLE getWin32Handle() const;
#endif

public: // static functions

	static void Yield();
	static AutoPtr<Thread> CurrentThread();
	static ThreadId CurrentThreadId();
	static ThreadList GetActiveThreads();
	static size_t GetActiveCount();
	static int GetInterruptSignal();
	static void SetInterruptSignal(int signo);

private: // static functions
	static String GenerateName();
	static void TerminateAllDaemonThreads();
	static void WaitAllUserThreads();

private:
	enum State {Initial, Active, Terminated};
	void setState(State state);
	void init();
	void setExecutionPriority();
	void cancel();
	void doRun();

private:
	State m_state;
	String m_name;
	bool m_bDaemon;
	const AutoPtr<Runnable> m_rpRunnable;
	ThreadId m_threadId;
	unsigned m_priority;

#if defined(QC_WIN32_THREADS)
	HANDLE m_hThread;
#elif defined(QC_POSIX_THREADS)
	bool m_bJoined;
#endif

	static AtomicCounter s_nextThreadNumber;
	static ThreadLocal s_thisPointer;
	static ThreadList s_activeThreadList;
	static int s_interruptSignal;

#endif //QC_MT

};

QC_BASE_NAMESPACE_END

#endif //QC_BASE_Thread_h

#include "QcCore/base/System.h"
#include "QcCore/io/Console.h"
#include "QcCore/base/Exception.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/util/AttributeListParser.h"

using namespace qc;
using namespace qc::io;
using namespace qc::util;

String getTestAttribute(const String& name);
void testMessage(const String& msg);
void testFailed(const String& test);
void testPassed(const String& test);
void goodCatch(const String& test, const String& eMsg);
void uncaughtException(const String& e, const String& test);


#include "QcCore/base/IllegalThreadStateException.h"
#include "QcCore/base/IllegalMonitorStateException.h"
#include "QcCore/base/InterruptedException.h"
#include "QcCore/base/Thread.h"
#include "QcCore/base/Runnable.h"
#include "QcCore/base/ConditionVariable.h"
#include "QcCore/base/RecursiveMutex.h"
#include <signal.h>

using namespace qc; 


#ifdef QC_MT

size_t GlobalCount = 0;
size_t Semaphore = 0;
ConditionVariable GlobalCV;
RecursiveMutex GlobalMutex;

//
// class: testInterrupt
//
// The basic idea behind this class is that it tests the Thread::interrupt
// functionality of QuickCPP.
//
// A thread is started, which then goes into a sleep.  The main thread interrupts
// this thread, so the sleep() should throw an InterruptedException.  To overcome
// the race condition where the main thread interrupts this thread before the sleep()
// starts, the sleep is performed within a loop.
//
class testInterrupt : public Runnable
{
	virtual void run()
	{
		// test yield
		try
	{
		Thread::Yield(); testPassed(QC_T("Yield"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("Yield"));
	}
		// sleep for a long while (10 secs) to give main line a chance to interrupt us
		bool bInterrupted = false;
		for(int i=0; i<2 && !bInterrupted; ++i)
		{
			try
			{
				Thread::Sleep(10000);
			}
			catch(InterruptedException& /*e*/)
			{
				bInterrupted = true;
			}
		}
		try
	{
		if(bInterrupted) {testPassed(QC_T("thread interrupted"));} else {testFailed(QC_T("thread interrupted"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("thread interrupted"));
	}
	}
};

class testWaiter : public Runnable
{
	virtual void run()
	{
		try
	{
		if(!GlobalMutex.isLocked()) {testPassed(QC_T("isLocked"));} else {testFailed(QC_T("isLocked"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("isLocked"));
	}
		try
	{
		GlobalCV.wait(GlobalMutex);
		testFailed(QC_T("badWait"));
	}
	catch(IllegalMonitorStateException& e)
	{
		goodCatch(QC_T("badWait"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("badWait"));
	}
		RecursiveMutex::Lock _lock1(GlobalMutex);
		try
	{
		if(GlobalMutex.isLocked()) {testPassed(QC_T("isLocked"));} else {testFailed(QC_T("isLocked"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("isLocked"));
	}
		{
			// create another level of recursion
			RecursiveMutex::Lock _lock2(GlobalMutex);
			GlobalCount++;
			while(!Semaphore)
			{
				try
	{
		GlobalCV.wait(GlobalMutex); testPassed(QC_T("wait"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("wait"));
	}
			}
			Semaphore--;
		}
	}
};

#endif //QC_MT

// Simple class to test use of QC_SYNCHRONIZED in 
// derived (client) classes
struct MySynchronizedObject : public SynchronizedObject
{
	bool doSynchronizedAction() const
	{
		QC_SYNCHRONIZED
		return true;
	}
};


void Thread_Tests()
{
	testMessage(QC_T("Starting tests for Thread"));

	try
	{
		; testPassed(QC_T("Thread::Sleep(10)"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("Thread::Sleep(10)"));
	}
	try
	{
		; testPassed(QC_T("Thread::Sleep(500,500)"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("Thread::Sleep(500,500)"));
	}

	// Synchronized object tests...
	AutoPtr<MySynchronizedObject> rpSync;
	try
	{
		rpSync = new MySynchronizedObject; testPassed(QC_T("new Synchronized"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("new Synchronized"));
	}
	try
	{
		if(rpSync->doSynchronizedAction()) {testPassed(QC_T("Synchronized"));} else {testFailed(QC_T("Synchronized"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("Synchronized"));
	}

#ifdef QC_MT

#ifndef WIN32
	try
	{
		Thread::SetInterruptSignal(SIGUSR1); testPassed(QC_T("SetInterruptSignal"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("SetInterruptSignal"));
	}
#endif //WIN32

	AutoPtr<Thread> rpThread = new Thread(new testInterrupt);
	// test the destruction of a thread before it has been started
	// by re-using the AutoPtr for another object
	rpThread = new Thread(new testInterrupt);

	//
	// If the interrupt() call is supported, then start a thread
	// and attempt to interrupt it from its sleep.  The interrupt()
	// is placed within a loop to help avoid interrupting the 
	// thread before it has entered its sleep state.
	//
	if(rpThread->interruptSupported())
	{
		try
	{
		rpThread->start(); testPassed(QC_T("start"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("start"));
	}
		try
	{
		if(rpThread->isActive()) {testPassed(QC_T("isActive1"));} else {testFailed(QC_T("isActive1"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("isActive1"));
	}
		try
	{
		if(!rpThread->isDaemon()) {testPassed(QC_T("isDaemon"));} else {testFailed(QC_T("isDaemon"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("isDaemon"));
	}
		try
	{
		if(Thread::GetActiveCount()==1) {testPassed(QC_T("GetActiveCount"));} else {testFailed(QC_T("GetActiveCount"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("GetActiveCount"));
	}
		for(int i=0; i<5 && rpThread->isActive(); ++i)
		{
			try
	{
		rpThread->interrupt(); testPassed(QC_T("interrupt"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("interrupt"));
	}
			try
	{
		Thread::Sleep(1000); testPassed(QC_T("main sleep"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("main sleep"));
	}
		}
		try
	{
		rpThread->join(1000); testPassed(QC_T("join"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("join"));
	}
		try
	{
		if(!rpThread->isActive()) {testPassed(QC_T("isActive2"));} else {testFailed(QC_T("isActive2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("isActive2"));
	}
		try
	{
		if(Thread::GetActiveCount()==0) {testPassed(QC_T("GetActiveCount0"));} else {testFailed(QC_T("GetActiveCount0"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("GetActiveCount0"));
	}
		try
	{
		rpThread->start();
		testFailed(QC_T("start2"));
	}
	catch(IllegalThreadStateException& e)
	{
		goodCatch(QC_T("start2"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("start2"));
	}
	}

	const size_t waiterThreads = 10;
	for(size_t i=0; i<waiterThreads; ++i)
	{
		(new Thread(new testWaiter))->start();
	}

	// give the threads a chance to start
	// It would be better to use some form of barrier
	for(size_t i1=0; i1<10; ++i1)
	{
		RecursiveMutex::Lock _lock(GlobalMutex);
		if(GlobalCount==waiterThreads)
		{
			break;
		}
		try
	{
		_lock.unlock(); testPassed(QC_T("unlock"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("unlock"));
	}
		try
	{
		Thread::Sleep(500); testPassed(QC_T("sleep"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("sleep"));
	}
	}

	try
	{
		if(Thread::GetActiveCount()==waiterThreads) {testPassed(QC_T("GetActiveCount10"));} else {testFailed(QC_T("GetActiveCount10"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("GetActiveCount10"));
	}

	//
	// Now its time to start waking the threads up.
	// To guard against spurious wake-ups, we maintain a count like a semaphore
	//
	{
		RecursiveMutex::Lock _lock(GlobalMutex);
		Semaphore++;
		try
	{
		GlobalCV.signal(); testPassed(QC_T("signal"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("signal"));
	}
	}

	//
	// Wait for a signalled thread to terminate
	//
	for(size_t i2=0; i2<10; ++i2)
	{
		if(Thread::GetActiveCount()==(waiterThreads-1))
			break;
		// give the chosen thread a chance to die
		try
	{
		Thread::Sleep(500); testPassed(QC_T("sleep2"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("sleep2"));
	}
	}
	try
	{
		if(Thread::GetActiveCount()==(waiterThreads-1)) {testPassed(QC_T("GetActiveCount9"));} else {testFailed(QC_T("GetActiveCount9"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("GetActiveCount9"));
	}

	//
	// Now wake up the rest
	//
	{
		RecursiveMutex::Lock _lock(GlobalMutex);
		Semaphore+=(waiterThreads-1);
		try
	{
		GlobalCV.broadcast(); testPassed(QC_T("broadcast"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("broadcast"));
	}
	}

	//
	// give all the threads a chance to die
	//
	for(size_t i3=0; i3<10; ++i3)
	{
		if(Thread::GetActiveCount()==0)
			break;
		try
	{
		Thread::Sleep(500); testPassed(QC_T("sleep3"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("sleep3"));
	}
	}
	try
	{
		if(Thread::GetActiveCount()==0) {testPassed(QC_T("GetActiveCount3"));} else {testFailed(QC_T("GetActiveCount3"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("GetActiveCount3"));
	}

#endif //QC_MT


	testMessage(QC_T("End of tests for Thread"));
}


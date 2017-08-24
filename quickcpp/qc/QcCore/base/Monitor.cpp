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
// Class: Monitor
// 
/**
	@class qc::Monitor

    @brief A base class providing both synchronization and notification.

    The Monitor class is based on the concept of a Mutex and a ConditionVariable.
	The Mutex is used to synchronize access to a shared object; the 
	ConditionVariable is used to wake up waiting threads when the shared object
	has changed.

    By combining a Mutex and ConditionVariable in this way, the Monitor class
	provides a simple and safe means for making a class thread-safe.  However,
	it should be noted that this gain in simplicity is achieved with some loss
	of flexibility and, possibly, efficiency.  In particular there is no requirement
	for a ConditionVariable to be associated with a single Mutex, nor is there a
	requirement for a Mutex to be associated with a single ConditionVariable.

    As an example, we show a simple class that implements a thread-safe counter.
	For the purposes of demonstration, we have decided that whenever the count
	changes, all waiting threads should be woken so that they can re-inspect the count.

    @code
    class ActiveCounter : public Monitor {
    public:
        ActiveCounter() : m_count(0) {}

        void updateBy(long diff) {
            QC_SYNCHRONIZED  // creates a locked scope
            m_count+=diff;
            notifyAll();      // notifies all waiting threads
        }

        long getValue() const {
            QC_SYNCHRONIZED  // creates a locked scope
            return m_count;
        }

    private:
        long m_count;
    };
    @endcode

    The @c QC_SYNCHRONIZED macro is described in the class description
	for SynchronizedObject.
*/
//==============================================================================


#include "Monitor.h"
#include "IllegalMonitorStateException.h"

#ifdef QC_MT

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// Monitor::notify
//
/**
   Wakes up one thread that is currently waiting on this Monitor.

   Before calling this function, the caller must own the lock on this
   SynchronizedObject.  On return from this function, the lock on this
   SynchronizedObject is still owned.  

   @throws IllegalMonitorStateException if the caller does not own the lock on
           this SynchronizedObject
   @sa notifyAll()
*/
//==============================================================================
void Monitor::notify()
{
	if(!m_mutex.isLocked())
		throw IllegalMonitorStateException();

	m_cv.signal();
}

//==============================================================================
// Monitor::notifyAll
//
/**
   Wakes up all threads that are currently waiting on this Monitor.

   Before calling this function, the caller must own the lock on this
   SynchronizedObject.  On return from this function, the lock on this
   SynchronizedObject is still owned.  

   @throws IllegalMonitorStateException if the caller does not own the lock on
           this SynchronizedObject
   @sa notify()
*/
//==============================================================================
void Monitor::notifyAll()
{
	if(!m_mutex.isLocked())
		throw IllegalMonitorStateException();

	m_cv.broadcast();
}

//==============================================================================
// Monitor::wait
//
/**
   Atomically releases the lock on this SynchronizedObject and waits for this
   Monitor to become @a notified for the current thread.

   Before calling this function, the caller must own the lock on this
   SynchronizedObject.  On return from this function, the lock on this
   SynchronizedObject is re-acquired.  
   
   @throws IllegalMonitorStateException if the caller does not own the lock on
           this SynchronizedObject
   @throws InterruptedException if the current thread is interrupted
   @sa notify()
   @sa notifyAll()
*/
//==============================================================================
void Monitor::wait()
{
	m_cv.wait(m_mutex);
}

//==============================================================================
// Monitor::wait
//
/**
   Atomically releases the lock on this SynchronizedObject and waits for up to
   @a millis milliseconds for this Monitor to become @a notified for the current
   thread.

   Before calling this function, the caller must own the lock on this
   SynchronizedObject.  On return from this function, the lock on this 
   SynchronizedObject is re-acquired.  
   
   @param millis the number of milliseconds to wait
   @throws IllegalMonitorStateException if the caller does not own the lock on
           this SynchronizedObject
   @throws InterruptedException if the current thread is interrupted
   @sa notify()
   @sa notifyAll()
*/
//==============================================================================
void Monitor::wait(unsigned long millis)
{
	m_cv.wait(m_mutex, millis);
}

QC_BASE_NAMESPACE_END
#endif //QC_MT


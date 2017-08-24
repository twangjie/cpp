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
// Class: AtomicCounter
// 
/**
	@class qc::AtomicCounter
	
	@brief Integer value that can be incremented and
	decremented.  In a multi-threaded environment, access to the counter is
	synchronized to prevent lost updates.

*/
//==============================================================================

#include "AtomicCounter.h"

#if defined(WIN32)
	#include "QcCore/base/winincl.h"
#endif

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// AtomicCounter::AtomicCounter
//
/**
   Default constructor.

   Initializes the counter to zero.
*/
//==============================================================================
AtomicCounter::AtomicCounter() : 
	m_count(0)
{
}

//==============================================================================
// AtomicCounter::AtomicCounter
//
/**
   Constructs an AtomicCounter with an initial value.

   @param n the initial value of the counter.
*/
//==============================================================================
AtomicCounter::AtomicCounter(long n) :
	m_count(n)
{
}

//==============================================================================
// AtomicCounter::operator
//
/**
   Assignment operator.

   Assigns a new value to the AtomicCounter.
   @param n the new value of the counter.
   @returns the new value
*/
//==============================================================================
unsigned long AtomicCounter::operator=(long n)
{
	return m_count = n;
}

//==============================================================================
// AtomicCounter::operator++
//
/**
   Prefix increment operator.

   Increments the counter by one and returns the new value as a single atomic
   operation.
*/
//==============================================================================
unsigned long AtomicCounter::operator++()
{
#if defined(WIN32)
	// Note: not guaranteed to return correct result on Win95/NT3.51
	return ::InterlockedIncrement((long*)&m_count);
#else
	#if defined(QC_MT)
	FastMutex::Lock lock(m_mutex);
	#endif //QC_MT
	return ++m_count;
#endif
}

//==============================================================================
// AtomicCounter::operator++(int)
//
/**
   Postfix increment operator.

   Increments the counter by one and returns the original value as a 
   synchronized atomic operation.
*/
//==============================================================================
unsigned long AtomicCounter::operator++(int)
{
#if defined(WIN32)
	// Note: not guaranteed to return correct result on Win95/NT3.51
	return ::InterlockedIncrement((long*)&m_count)-1;
#else
	#if defined(QC_MT)
	FastMutex::Lock lock(m_mutex);
	#endif //QC_MT
	return m_count++;
#endif
}

//==============================================================================
// AtomicCounter::operator--
//
/**
   Prefix decrement operator.

   Decrements the counter by one and returns the new value as a 
   synchronized atomic operation.
*/
//==============================================================================
unsigned long AtomicCounter::operator--()
{
#if defined(WIN32)
	// Note: not guaranteed to return correct result on Win95/NT3.51
	return ::InterlockedDecrement((long*)&m_count);
#else
	#if defined(QC_MT)
	FastMutex::Lock lock(m_mutex);
	#endif //QC_MT
	return --m_count;
#endif
}

//==============================================================================
// AtomicCounter::operator--(int)
//
/**
   Postfix decrement operator.

   Decrements the counter by one and returns the original value as a 
   synchronized atomic operation.
*/
//==============================================================================
unsigned long AtomicCounter::operator--(int)
{
#if defined(WIN32)
	// Note: not guaranteed to return correct result on Win95/NT3.51
	return ::InterlockedDecrement((long*)&m_count)+1;
#else
	#if defined(QC_MT)
	FastMutex::Lock lock(m_mutex);
	#endif //QC_MT
	return m_count--;
#endif
}

//==============================================================================
// AtomicCounter::operator long
//
/**
   Conversion operator.

   Returns the current value of the counter.
*/
//==============================================================================
AtomicCounter::operator unsigned long() const
{
	return m_count;
}

QC_BASE_NAMESPACE_END

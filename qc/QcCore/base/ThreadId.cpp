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
// Class: ThreadId
// 
/**
	@class qc::ThreadId
	
	@brief Represents an abstract thread identifier. 

	A ThreadId uniquely identifies a single Thread for the duration that the 
	Thread is active.  After a Thread has terminated its ThreadId may be reused
	by a new Thread. 
*/
//==============================================================================

#include "ThreadId.h"
#include "NumUtils.h"

#ifdef QC_MT

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// ThreadId::ThreadId
//
/**
   Default constructor.
*/
//==============================================================================
ThreadId::ThreadId() :
	m_nativeId(0)
{
}

//==============================================================================
// ThreadId::ThreadId
//
/**
   Copy constructor.
   @param rhs the ThreadId to copy
*/
//==============================================================================
ThreadId::ThreadId(const ThreadId& rhs) :
	m_nativeId(rhs.m_nativeId)
{
}

//==============================================================================
// ThreadId::ThreadId
//
/**
   Constructs a ThreadId from a native thread identifier as used by the underlying
   threading library.
   @param id the native thread identifier
*/
//==============================================================================
ThreadId::ThreadId(NativeId id) :
	m_nativeId(id)
{
}

//==============================================================================
// ThreadId::operator=
//
/**
   Assignment operator.  Sets this ThreadId equal to the native thread identifier
   @c id.
*/
//==============================================================================
ThreadId& ThreadId::operator=(NativeId id)
{
	m_nativeId = id;
	return *this;
}

//==============================================================================
// ThreadId::operator==
//
/**
   Equality operator.  Returns true if this ThreadId is equal to the
   ThreadId @c rhs.
   @returns true if the two ThreadId objects represent the same Thread; false
            otherwise
*/
//==============================================================================
bool ThreadId::operator==(const ThreadId& rhs) const
{
#if defined(QC_WIN32_THREADS)

	return (m_nativeId == rhs.m_nativeId);

#elif defined(QC_POSIX_THREADS)

	return (::pthread_equal(m_nativeId, rhs.m_nativeId) != 0);

#else
	#error Unsupported configuration
#endif
}

//==============================================================================
// ThreadId::operator!=
//
/**
   Inequality operator.  Returns true if this ThreadId is not equal to the
   ThreadId @c rhs.
   @returns false if the two ThreadId objects represent the same Thread; true
            otherwise
*/
//==============================================================================
bool ThreadId::operator!=(const ThreadId& rhs) const
{
	return !(*this==rhs);
}

//==============================================================================
// ThreadId::NativeId 
//
/**
   Returns the native thread identifier.
*/
//==============================================================================
ThreadId::NativeId ThreadId::getNativeId() const
{
	return m_nativeId;
}

//==============================================================================
// ThreadId::toString
//
/**
   Returns a string representation of the object.
*/
//==============================================================================
String ThreadId::toString() const
{
	return NumUtils::ToString((unsigned long)m_nativeId);
}

QC_BASE_NAMESPACE_END

#endif //QC_MT

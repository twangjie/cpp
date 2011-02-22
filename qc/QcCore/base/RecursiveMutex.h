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
// Class: RecursiveMutex
// 
//==============================================================================

#ifndef QC_BASE_RecursiveMutex_h
#define QC_BASE_RecursiveMutex_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include "Mutex.h"
#include "FastMutex.h"
#include "ThreadId.h"

#ifdef QC_MT

QC_BASE_NAMESPACE_BEGIN

class QC_BASE_PKG RecursiveMutex
{
	friend class ConditionVariable;

public:
	typedef AutoLock<RecursiveMutex> Lock;

	RecursiveMutex();

	void lock();
	bool tryLock();
	void unlock();
	bool isLocked();

private: // not implemented
	RecursiveMutex(const RecursiveMutex& rhs);            // cannot be copied
	RecursiveMutex& operator=(const RecursiveMutex& rhs); // nor assigned

private: // functions for use by ConditionVariable
	int getRecursionCount() const;
	void postWait(int recursionCount);
	void preWait();

private:
	Mutex m_mutex;
	//FastMutex m_internalMutex;
	ThreadId m_owningThreadId;
	int m_recursionCount;
};

QC_BASE_NAMESPACE_END

#endif //QC_MT
#endif //QC_BASE_RecursiveMutex_h

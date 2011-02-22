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
//==============================================================================

#ifndef QC_BASE_ConditionVariable_h
#define QC_BASE_ConditionVariable_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#ifdef QC_MT

#include "FastMutex.h"
#include <list>

QC_BASE_NAMESPACE_BEGIN

class RecursiveMutex;

class QC_BASE_PKG ConditionVariable
{
public:

	ConditionVariable();
	~ConditionVariable();

	void wait(RecursiveMutex& mutex);
	bool wait(RecursiveMutex& mutex, unsigned long milliseconds);

	void signal();
	void broadcast();

private: // not implemented
	ConditionVariable(const ConditionVariable& rhs);            // cannot be copied
	ConditionVariable& operator=(const ConditionVariable& rhs); // nor assigned

private:

#if defined(QC_WIN32_THREADS)

	bool waitImplWin32(RecursiveMutex& mutex, DWORD timeout);
	void signalImplWin32(HANDLE hWaiter);

	HANDLE m_hBroadcastEvent;
	typedef std::list<HANDLE> WaitersQueue;
	WaitersQueue m_waitersQueue;
	FastMutex m_waitersLock;

#elif defined(QC_POSIX_THREADS)

	pthread_cond_t m_cv;

#else
	#error Unsupported configuration
#endif

};

QC_BASE_NAMESPACE_END

#endif //QC_MT
#endif //QC_BASE_ConditionVariable_h


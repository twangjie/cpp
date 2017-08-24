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
// Class: Mutex
// 
//==============================================================================

#ifndef QC_BASE_Mutex_h
#define QC_BASE_Mutex_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include "AutoLock.h"

#ifdef QC_MT

QC_BASE_NAMESPACE_BEGIN

class QC_BASE_PKG Mutex
{
public:
	friend class ConditionVariable;

	typedef AutoLock<Mutex> Lock;

	Mutex();
	~Mutex();

	void lock();
	bool tryLock();
	void unlock();

private: // not implemented
	Mutex(const Mutex& rhs);            // cannot be copied
	Mutex& operator=(const Mutex& rhs); // nor assigned

private:
#if defined(QC_WIN32_THREADS)

	HANDLE m_hMutex;

#elif defined(QC_POSIX_THREADS)

	pthread_mutex_t m_mutex;

#else
	#error Unsupported configuration
#endif

};

QC_BASE_NAMESPACE_END

#endif //QC_MT
#endif //QC_BASE_Mutex_h

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
//==============================================================================

#ifndef QC_BASE_ThreadId_h
#define QC_BASE_ThreadId_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include "String.h"

#ifdef QC_MT

QC_BASE_NAMESPACE_BEGIN

class QC_BASE_PKG ThreadId
{
public:

#if defined(QC_DOCUMENTATION_ONLY)

	/** Set equal to the thread identifier type of the underlying
	    threading library
	*/
	typedef os_thread_id NativeId;

#elif defined(QC_WIN32_THREADS)

	typedef DWORD NativeId;

#elif defined(QC_POSIX_THREADS)

	typedef pthread_t NativeId;

#endif

	ThreadId();
	ThreadId(const ThreadId& rhs);
	ThreadId(NativeId id);

	ThreadId& operator=(NativeId id);

	bool operator==(const ThreadId& rhs) const;
	bool operator!=(const ThreadId& rhs) const;

	NativeId getNativeId() const;
	
	String toString() const;

private:
	NativeId m_nativeId;
};

QC_BASE_NAMESPACE_END

#endif //QC_MT
#endif //QC_BASE_ThreadId_h

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
// Class: ThreadLocal
// 
//==============================================================================

#ifndef QC_BASE_ThreadLocal_h
#define QC_BASE_ThreadLocal_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#ifdef QC_MT

QC_BASE_NAMESPACE_BEGIN

class QC_BASE_PKG ThreadLocal
{
public:

	ThreadLocal();
	~ThreadLocal();
	
	void set(void* value) const;
	void* get() const;

private: 

#if defined(QC_WIN32_THREADS)
	typedef DWORD Key;
#else
	typedef pthread_key_t Key;
#endif

	Key m_key;
};

QC_BASE_NAMESPACE_END

#endif //QC_MT
#endif //QC_BASE_ThreadLocal_h

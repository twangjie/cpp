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

#ifndef QC_BASE_threaddefs_h
#define QC_BASE_threaddefs_h

#ifdef QC_MT

	#if defined(QC_WIN32_THREADS)

		#include "winincl.h"
	
		// windows.h has an unfortunate habit of #defining Yield()
		#ifdef Yield
			#undef Yield
		#endif

	#elif defined(QC_POSIX_THREADS)

		#include <pthread.h>

	#else

		#error "QC_MT defined but no threading support available"

	#endif

	//
	// Macros to aid in the creation of scoped mutex locks
	//
	#define QC_AUTO_LOCK(_LOCK_TYPE, _THE_LOCK)\
		_LOCK_TYPE::Lock _scoped_lock_((_LOCK_TYPE&)_THE_LOCK)

	#define QC_MT_VOLATILE volatile

#else // !QC_MT

	#define QC_AUTO_LOCK(_LOCK_TYPE, _THE_LOCK)
	#define QC_MT_VOLATILE

#endif // QC_MT

#endif // QC_BASE_threaddefs_h

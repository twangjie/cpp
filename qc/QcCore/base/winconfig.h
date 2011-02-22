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
// This is the Windows version of the QuickCPP configuration file
//
//==============================================================================

#ifdef WIN32

	#undef  HAVE_WORKING_SOCKLEN_T
	#define HAVE_UNSIGNED_WCHAR_T
	#define SIZEOF_WCHAR_T 2
	#undef  WORDS_BIGENDIAN
	#define HAVE_THREAD_SAFE_RESOLVER
	#define CREATEFILE_SETS_MODTIME

	#ifdef _DEBUG

	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h> 
	#include <crtdbg.h>
	#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
	#define new DEBUG_NEW

	#define CHECKMEMORY() \
	do{ \
		int nFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG ); \
		nFlag |= _CRTDBG_LEAK_CHECK_DF; \
		_CrtSetDbgFlag( nFlag ); \
	} while(0);

	#endif
#else

	#error This file should only be included during a Windows build.

#endif //WIN32

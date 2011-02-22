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
// This file provides general macros that are required by the QuickCPP Library
//
//==============================================================================

#ifndef QC_BASE_gendefs_h
#define QC_BASE_gendefs_h

//
// Include QuickCPP version flags
//
#include "version.h"

//
// Include the top-level configuration header
//
#ifdef WIN32
	#include "winconfig.h"
#else
	#include "config.h"
#endif

#ifdef _MSC_VER
	// disable annoying template warning under msvc
	#pragma warning(disable: 4786)
	// disable 'this' pointer used in member initialization message
	#pragma warning(disable: 4355)
	// disable 'need to have dll interface to be used by clients' message
	#pragma warning(disable: 4251)
#endif

//
// Include compiler-specific feature detection
//
#include "compdefs.h"

//
// If QuickCPP is to be compiled against "standard compliant" systems, then
// the following macros should be defined.  Note, however, that this
// has an impact on the library names that must be linked with the application
// and is therefore not necessarily desirable.
//
//#define _XOPEN_SOURCE            500 // Specified by the "Single UNIX specification"
//#define _XOPEN_SOURCE_EXTENDED   1   // UNIX 98 socket behaviour on HP-UX
//#define _SVID_SOURCE             1   // ISO C, POSIX, and SVID things (gethostbyname_r)

//#include <stdlib.h>  // NULL etc
#include <stddef.h>  // ANSI types and functions

// ensure agreement between various flavours of DEBUG macro
#if defined(_DEBUG) && !defined(DEBUG)
	#define DEBUG
#endif //defined

#if defined(DEBUG) && !defined(_DEBUG)
#define _DEBUG
#endif //defined

#ifdef QC_MT
	#ifdef WIN32
		#define QC_WIN32_THREADS
	#else
		#define QC_POSIX_THREADS
	#endif
#endif //QC_MT

// ensure agreement between various flavours of multi-threaded options
#if defined(QC_MT) && !defined(_REENTRANT)
	#define _REENTRANT
#endif //defined

//
// Allow the QuickCPP namespace name to be set externally if required
//
#ifndef QC_NAMESPACE_NAME
	#define QC_NAMESPACE_NAME qc
#endif

#define QC_NAMESPACE_BEGIN \
namespace QC_NAMESPACE_NAME {

#define QC_NAMESPACE_END \
} /* end of namespace */

#define QC_SUB_NAMESPACE_BEGIN(N) \
QC_NAMESPACE_BEGIN \
namespace N {

#define QC_SUB_NAMESPACE_END(N) \
}} /* end of namespaces */

//
// QuickCPP is currently configured to use either char or wchar_t as its
// character type.  The default is char, but wchar_t is selected if the QC_WCHAR
// macro is defined
//
#ifndef QC_DOCUMENTATION_ONLY

#ifdef QC_WCHAR
	#define QC_CHAR_TYPE wchar_t

	// If the platform's wchar_t is unsigned, then we use that as our
	// UCharType, otherwise we use unsigned
	#ifdef HAVE_UNSIGNED_WCHAR_T
		#define QC_UCHAR_TYPE wchar_t
	#else
		#define QC_UCHAR_TYPE unsigned
	#endif

	#define QC_INT_TYPE int
	#define QC_T(t) L##t

	#ifdef QC_UCS2
		#if SIZEOF_WCHAR_T < 2
			#error UCS2 requires at least 16 bits
		#endif

		#define QC_MAX_CHAR 0xFFFE
		#undef  QC_UCS4
		#undef  QC_UTF16

	#else // !QC_UCS2

		#define QC_MAX_CHAR 0x10FFFF
		#if SIZEOF_WCHAR_T == 2
			#define QC_UTF16
			#undef QC_UCS4
		#elif SIZEOF_WCHAR_T >= 4
			#define QC_UCS4
			#undef QC_UTF16
		#else
			#error unsupported wchar_t size
		#endif //SIZEOF_WCHAR_T

	#endif // !QC_UCS2

	//#define QC_COUT std::wcout
	//#define QC_CERR std::wcerr

#else  // !QC_WCHAR

	#define QC_CHAR_TYPE char
	#define QC_UCHAR_TYPE unsigned char
	#define QC_INT_TYPE int
	#define QC_T(t) t
	#define QC_MAX_CHAR 0x10FFFF
	#define QC_UTF8
	#undef  QC_UTF16
	#undef  QC_UCS4
	#undef  QC_UCS2
	//#define QC_COUT std::cout
	//#define QC_CERR std::cerr

#endif //QC_WCHAR
#endif //QC_DOCUMENTATION_ONLY

//
// Win32 DLL Versions of the library
//
#if defined(WIN32) && defined(QC_DLL)
	#if defined QC_IOS_EXPORT
		#define QC_QUICKCPP_PKG __declspec(dllexport)
		#define QC_QUICKCPP_INCLUDE_INLINES 1
	#else
		#define QC_QUICKCPP_PKG __declspec(dllimport)
		#if defined(_MSC_VER) && 1300 <= _MSC_VER
			#define QC_QUICKCPP_INCLUDE_INLINES 0
		#else
			#define QC_QUICKCPP_INCLUDE_INLINES 1
		#endif
	#endif
#else
	#define QC_QUICKCPP_PKG
	#define QC_QUICKCPP_INCLUDE_INLINES 1
#endif

#endif // QC_BASE_gendefs_h


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
// This file provides detection and work arounds for specific compilers.
//
//==============================================================================

#ifndef QC_BASE_compdefs_h
#define QC_BASE_compdefs_h

#include <string>

#undef QC_USING_DECL_BROKEN
#undef QC_BOOL_IS_TYPEDEF

// defined when the compiler cannot be used
// to import inherited virtual functions into
// the scope of a derived class when the derived
// class already contains an overloaded function 
// with the same name.

//
// GNU C++ compiler quirks
//
#ifdef __GNUC__ 
	// GNU C++
	#if (__GNUC__ < 3 && __GNUC_MINOR__ < 96)
		#define QC_USING_DECL_BROKEN 1
	#endif
	//
	// libstdc++ v2 does not have a #define that we can use
	// to detect its existence.  For this reason we use
	// some heuristics
	// 1) __GLIBCPP__ not defined (it wasn't defined until v3)
	// 2) __BASTRING__ defined in string from libstdc++ v2
	#if !defined(__GLIBCPP__) && defined(__BASTRING__)
		#define QC_BROKEN_C_STR
		#define QC_NO_WSTRING_TYPEDEF
	#endif

#endif //__GNUC__

//
// Borland C++ compiler quirks
//
#ifdef __BORLANDC__
	// Borland C++
	#define QC_USING_DECL_BROKEN 1

#endif //__BORLANDC__

//
// Sun Workshop C++ compiler quirks
//
#ifdef __SUNPRO_CC
	#if __SUNPRO_CC < 0x420
		#error "Unsupported Sun WorkShop compiler version"
	#elif (__SUNPRO_CC == 0x420 || __SUNPRO_CC >= 0x500 && __SUNPRO_CC_COMPAT == 4)
		#define QC_BOOL_IS_TYPEDEF 1
		#define QC_USING_DECL_BROKEN 1
		typedef int bool;
		#undef false
		#undef true
		#define false 0
		#define true 1
		#define typename class
	#endif
#endif //__SUNPRO_CC

//
// HP aCC C++ compiler quirks
//
#ifdef __HP_aCC
	#define QC_USING_DECL_BROKEN 1
#endif //__HP_aCC

#ifdef _MSC_VER
	//
	// STLPort quirks under MS Visual C++
	//
	#ifdef _STLPORT_VERSION
		// std::find_first_of() requires this (in STLPort v4.5.3)
		#define QC_USE_STD_ITERATOR_BASE 1
	#endif //_STLPORT_VERSION

#endif //_MSC_VER

// 
//
#endif // QC_BASE_compdefs_h


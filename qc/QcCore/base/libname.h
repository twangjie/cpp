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
// The QC_LIBNAME macro generates a library name that encodes the values of
// some of the build environment such as: debug/unicode etc.
//
//==============================================================================

#ifndef QC_base_libname_h
#define QC_base_libname_h

#ifdef QC_MT
#define _QC_MUTIL_THREAD_SUFFIX "mt"
#else //QC_UNICODE
#define _QC_MUTIL_THREAD_SUFFIX 
#endif //QC_UNICODE

//#ifdef QC_UNICODE
//#define _QC_UNICODE_SUFFIX "u"
//#else //QC_UNICODE
//#define _QC_UNICODE_SUFFIX 
//#endif //QC_UNICODE

#ifdef QC_DLL
#define _QC_TYPE_SUFFIX "s"
#else //QC_DLL
#define _QC_TYPE_SUFFIX 
#endif //QC_DLL

#ifdef _DEBUG
#define _QC_DEBUG_SUFFIX "d"
#else //_DEBUG
#define _QC_DEBUG_SUFFIX
#endif //_DEBUG

#define QC_LIBNAME(xxx) \
#xxx _QC_MUTIL_THREAD_SUFFIX _QC_DEBUG_SUFFIX
//#xxx _QC_MUTIL_THREAD_SUFFIX _QC_UNICODE_SUFFIX _QC_TYPE_SUFFIX _QC_DEBUG_SUFFIX
//QC_LIB_PREFIX #xxx QC_LIB_SUFFIX QC_LIB_USUFFIX

//#ifdef _DEBUG
//#define _QC_DEBUG_SUFFIX "d"
//#else //_DEBUG
//#define _QC_DEBUG_SUFFIX
//#endif //_DEBUG
//
//#define QC_LIBNAME(xxx) \
//#xxx _QC_DEBUG_SUFFIX

#endif //QC_base_libname_h


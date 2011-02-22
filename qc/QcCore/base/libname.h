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

#ifndef QC_UTILS_libname_h
#define QC_UTILS_libname_h

//#ifdef QC_MT
//#define _QC_THREAD_SUFFIX "mt"
//#else //QC_WCHAR
//#define _QC_THREAD_SUFFIX 
//#endif //QC_WCHAR
//
//#ifdef QC_WCHAR
//#define _QC_CHAR_SUFFIX "w"
//#else //QC_WCHAR
//#define _QC_CHAR_SUFFIX 
//#endif //QC_WCHAR
//
//#ifdef QC_DLL
//#define _QC_TYPE_SUFFIX "s"
//#else //QC_DLL
//#define _QC_TYPE_SUFFIX 
//#endif //QC_DLL

//#ifdef _DEBUG
//#define _QC_MODE_SUFFIX "d"
//#else //_DEBUG
//#define _QC_MODE_SUFFIX
//#endif //_DEBUG

//#define QC_LIBNAME(xxx) \
//#xxx _QC_THREAD_SUFFIX _QC_CHAR_SUFFIX _QC_TYPE_SUFFIX _QC_MODE_SUFFIX
////QC_LIB_PREFIX #xxx QC_LIB_SUFFIX QC_LIB_USUFFIX

#ifdef _DEBUG
#define _QC_MODE_SUFFIX "d"
#else //_DEBUG
#define _QC_MODE_SUFFIX
#endif //_DEBUG

#define QC_LIBNAME(xxx) \
#xxx _QC_MODE_SUFFIX

#endif //QC_UTILS_libname_h


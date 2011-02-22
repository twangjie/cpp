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
// Class: Win32Utils
// 
// Overview
// --------
// A C++ wrapper class around a small part of the WIN32 API.
//
// Generally used when special facilities are required under WIN32 that are
// available via a different means in Unix/Linux.
//
//=============================================================================

#ifndef QC_UTIL_Win32Utils_h
#define QC_UTIL_Win32Utils_h

#ifdef WIN32

#ifndef QC_UTIL_DEFS_h
#include "defs.h"
#endif //QC_UTIL_DEFS_h

#include "DateTime.h"

#include "QcCore/base/winincl.h"

#include <list>

QC_UTIL_NAMESPACE_BEGIN

class QC_UTIL_PKG Win32Utils
{
public:
	static std::list<String> ExpandFiles(const String& filespec);
	static DateTime SystemTimeToDateTime(SYSTEMTIME* const pSysTime);
	static void DateTimeToSystemTime(const DateTime& time, SYSTEMTIME* pSysTime);
	static void TraceSystemCall(short nSection, short nLevel, const String& message, DWORD errNo);
};

QC_UTIL_NAMESPACE_END

#endif //WIN32
#endif //QC_UTIL_Win32Utils_h


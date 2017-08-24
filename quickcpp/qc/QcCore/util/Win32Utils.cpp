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

#ifdef WIN32

#include "Win32Utils.h"

#include "QcCore/base/Win32Exception.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/base/NullPointerException.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/base/SystemUtils.h"
#include "QcCore/base/Tracer.h"
#include "QcCore/base/debug.h"

#include <stdlib.h>

QC_UTIL_NAMESPACE_BEGIN

//==============================================================================
// Win32Utils::ExpandFiles
//
// Given a file specification (eg *.xml), expand it to a list of filenanes
// using the Win32 functions: FindFirstFile/FindNextFile.
//==============================================================================
std::list<String> Win32Utils::ExpandFiles(const String& filespec)
{
	StringUtils::TCharAutoPtr pFilespec = StringUtils::ToWin32String(filespec);

	std::list<String> fileList;

	WIN32_FIND_DATA nextInfo;

	//
	// Find the first matching file.  An INVALID_HANDLE_VALUE is returned if no files match
	//
	HANDLE hContext = ::FindFirstFile(pFilespec.get(), &nextInfo);

	if(hContext == INVALID_HANDLE_VALUE)
	{
		throw Win32Exception(::GetLastError());
	}
	else
	{
		String dirPart;
		size_t dirPos = filespec.find_last_of(QC_T("/\\"));
		if(dirPos != String::npos)
		{
			dirPart = filespec.substr(0, dirPos);
		}

		String sSep = QC_T("\\");

		do
		{
			String file = StringUtils::FromWin32String(nextInfo.cFileName);
			static String sDot(QC_T("."));
			static String sDotDot(QC_T(".."));
			if(file != sDot && file != sDotDot)
			{
				if(!dirPart.empty())
				{
					file = dirPart + sSep + file;
				}
				fileList.push_back(file);
			}
		}
		while(::FindNextFile(hContext, &nextInfo));
		
		::FindClose(hContext);
	}
	fileList.sort();
	return fileList;
}

//==============================================================================
// Win32Utils::SystemTimeToDateTime
//
//==============================================================================
DateTime Win32Utils::SystemTimeToDateTime(SYSTEMTIME* const pSysTime)
{
	QC_DBG_ASSERT(pSysTime);
	if(!pSysTime) throw NullPointerException();

	return DateTime(pSysTime->wYear, pSysTime->wMonth, pSysTime->wDay,
	                pSysTime->wHour, pSysTime->wMinute, pSysTime->wSecond,
	                pSysTime->wMilliseconds);
}

//==============================================================================
// Win32Utils::DateTimeToSystemTime
//
//==============================================================================
void Win32Utils::DateTimeToSystemTime(const DateTime& time, SYSTEMTIME* pSysTime)
{
	QC_DBG_ASSERT(pSysTime);
	if(!pSysTime) throw NullPointerException();
	UShort year, month, day, dayOfWeek, hour, minute, second, millisecond;
	time.getDate(year, month, day, dayOfWeek);
	time.getTime(hour, minute, second, millisecond);
	pSysTime->wYear = year;
	pSysTime->wMonth = month;
	pSysTime->wDay = day;
	pSysTime->wDayOfWeek = dayOfWeek;
	pSysTime->wHour = hour;
	pSysTime->wMinute = minute;
	pSysTime->wSecond = second;
	pSysTime->wMilliseconds = millisecond;
}

//==============================================================================
// Win32Utils::TraceSystemCall
//
//==============================================================================
void Win32Utils::TraceSystemCall(short nSection, short nLevel, const String& message, DWORD errNo)
{
	static String sCode = QC_T(" rc=");
	static String sComma = QC_T(", ");
	if(errNo==0) errNo = ::GetLastError();

	String traceMsg = message + sCode 
	                  + NumUtils::ToString(errNo)
					  + sComma + SystemUtils::GetWin32ErrorString(errNo);

	Tracer::Trace(nSection, nLevel, traceMsg);
}

QC_UTIL_NAMESPACE_END
#endif //WIN32


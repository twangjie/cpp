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

#include "SystemUtils.h"
#include "IllegalArgumentException.h"
#include "NullPointerException.h"
#include "NumUtils.h"
#include "StringUtils.h"
#include "Tracer.h"
#include "debug.h"

#include <algorithm>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// SystemUtils::GetSystemErrorString
//
// Portable routine to return an error message associated with the passed
// error code.
//==============================================================================
String SystemUtils::GetSystemErrorString(long errorNum)
{
	errorNum = errorNum ? errorNum : errno;
	return StringUtils::FromNativeMBCS(strerror(errorNum))
		+ StringUtils::FromLatin1(StringUtils::Format(" (%ld)", errorNum));
}

//==============================================================================
// SystemUtils::TraceSystemCall
//
//==============================================================================
void SystemUtils::TraceSystemCall(short nSection, short nLevel, const String& message, int rc)
{
	static String sRC = QC_T(" rc=");
	static String sErrno = QC_T(", errno=");
	static String sComma = QC_T(", ");
	int err=errno;

	String traceMsg = message + sRC + NumUtils::ToString(rc);
	if(rc!=0)
	{
		traceMsg += sErrno  + NumUtils::ToString(err)
	             + sComma + GetSystemErrorString(err);
	}

	Tracer::Trace(nSection, nLevel, traceMsg);
}

//==============================================================================
// SystemUtils::TestBufferIsValid
//
// Test that the passed buffer is appropriate for calls to read() operations.
//==============================================================================
void SystemUtils::TestBufferIsValid(const void* pBuffer, size_t& bufLen)
{
	if(!pBuffer) throw NullPointerException();
	if(!bufLen)  throw IllegalArgumentException(QC_T("zero buffer length"));
	if(bufLen > LONG_MAX)
		bufLen = LONG_MAX;
}

#ifdef WIN32

//==============================================================================
// SystemUtils::GetWin32ErrorString
//
// Return an error message associated with the passed WIN32 error code.
//==============================================================================
String SystemUtils::GetWin32ErrorString(DWORD errNo)
{
	LPVOID lpMessageBuffer = 0;

	::FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		errNo,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //default language
		(LPTSTR) &lpMessageBuffer,
		0,
		NULL);

	String ret = StringUtils::FromWin32String((LPCTSTR)lpMessageBuffer);

	::LocalFree((HLOCAL) lpMessageBuffer);

	//
	// Strip trailing .crlf if present
	//
	if(ret.size()>2 && (ret[ret.size()-3] == '.'))
	{
		ret.erase(ret.size()-3);
	}
	return ret;
}

#endif // !WIN32

QC_BASE_NAMESPACE_END


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
// Class: Win32Exception
// 
/**
	@class qc::Win32Exception
	
	@brief Thrown when @QuickCPP makes a Win32 API call which results
	in an unexpected return code.
*/
//==============================================================================

#ifdef WIN32

#include "Win32Exception.h"
#include "SystemUtils.h"

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// Win32Exception::Win32Exception
//
//==============================================================================
Win32Exception::Win32Exception(DWORD errorCode)
{
	setMessage(SystemUtils::GetWin32ErrorString(errorCode));
}

//==============================================================================
// Win32Exception::Win32Exception
//
//==============================================================================
Win32Exception::Win32Exception(DWORD errorCode, const String& message)
{
	String errMsg;
	if(!message.empty())
	{
		 errMsg = message;
		 errMsg += QC_T(": ");
	}
	errMsg += SystemUtils::GetWin32ErrorString(errorCode);
	setMessage(errMsg);
}

QC_BASE_NAMESPACE_END
#endif //WIN32


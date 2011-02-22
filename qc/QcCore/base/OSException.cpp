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
// Class: OSException
// 
/**
	@class qc::OSException
	
	@brief Thrown when @QuickCPP makes a system call which results
	in an unexpected return code.
*/
//==============================================================================

#include "OSException.h"
#include "SystemUtils.h"

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// OSException::OSException
//
/**
   Protected default constructor.
*/
//==============================================================================
OSException::OSException()
{
}

//==============================================================================
// OSException::OSException
//
/**
   Constructs a OSException with a system error code which is used to
   retrieve an operating system error message.
   @param errCode the system error code that gave rise to the exception
*/
//==============================================================================
OSException::OSException(long errCode) :
	RuntimeException(SystemUtils::GetSystemErrorString(errCode))
{
}

//==============================================================================
// OSException::OSException
//
/**
   Constructs a OSException with a system error code and additional message.
   The error code is used to retrieve an operating system error message.
   @param errCode the system error code that gave rise to the exception
   @param message further detail
*/
//==============================================================================
OSException::OSException(long errCode, const String& message)
{
	String errMsg;
	if(!message.empty())
	{
		 errMsg = message;
		 errMsg += QC_T(": ");
	}
	errMsg += SystemUtils::GetSystemErrorString(errCode);
	setMessage(errMsg);
}

//==============================================================================
// OSException::getErrorCode
//
/**
   Returns the system error code that gave rise to the exception.
*/
//==============================================================================
long OSException::getErrorCode() const
{
	return m_errCode;
}

QC_BASE_NAMESPACE_END

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
// Class: IllegalThreadStateException
// 
/**
	@class qc::IllegalThreadStateException
	
	@brief Thrown when a Thread object is not in the correct state
	for the method being called.  For example, a Thread cannot be started
	more than once.
*/
//==============================================================================

#ifndef QC_BASE_IllegalThreadStateException_h
#define QC_BASE_IllegalThreadStateException_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include "RuntimeException.h"

QC_BASE_NAMESPACE_BEGIN

class QC_BASE_PKG IllegalThreadStateException : public RuntimeException
{
public:
	/** Constructs an IllegalThreadStateException without a detail message */
	IllegalThreadStateException()
	{}

	/** Constructs an IllegalThreadStateException with a detail message
	* @param message the detail message.
	*/
	IllegalThreadStateException(const String& message) : 
		RuntimeException(message)
	{}
	
	virtual String getExceptionType() const {return QC_T("IllegalThreadStateException");}
};

QC_BASE_NAMESPACE_END

#endif //QC_BASE_IllegalThreadStateException_h


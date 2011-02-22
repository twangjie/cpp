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
// Class: IllegalStateException
// 
/**
	@class qc::IllegalStateException
	
	@brief Thrown when a method is called for an object that is
	not in the correct state for that operation.
*/
//==============================================================================

#ifndef QC_BASE_IllegalStateException_h
#define QC_BASE_IllegalStateException_h

#ifndef QC_BASE_defs_h
#include "defs.h"
#endif //QC_BASE_defs_h

#include "RuntimeException.h"

QC_BASE_NAMESPACE_BEGIN

class QC_BASE_PKG IllegalStateException : public RuntimeException
{
public:
	/** Constructs an IllegalStateException without a detail message */
	IllegalStateException()
	{}

	/** Constructs an IllegalStateException with a detail message
	* @param message the detail message.
	*/
	IllegalStateException(const String& message) : 
		RuntimeException(message)
	{}
	
	virtual String getExceptionType() const {return QC_T("IllegalStateException");}
};

QC_BASE_NAMESPACE_END

#endif //QC_BASE_IllegalStateException_h


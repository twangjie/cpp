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
// Class: IllegalCharacterException
// 
/**
	@class qc::IllegalCharacterException
	
	@brief Thrown when an illegal character value is detected.
	
	This can occur when @QuickCPP is using the UTF-8 or UTF-16 internal encoding
	and the application creates a multi-character sequence that would generate
	an invalid Unicode character.
	
    This exception can also be thrown when attempting to construct a Character
	with an illegal Unicode code-point value.
*/
//==============================================================================

#ifndef QC_BASE_IllegalCharacterException_h
#define QC_BASE_IllegalCharacterException_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include "RuntimeException.h"

QC_BASE_NAMESPACE_BEGIN

class QC_BASE_PKG IllegalCharacterException : public RuntimeException
{
public:
	/** Constructs an IllegalCharacterException without a detail message */
	IllegalCharacterException()
	{}

	/** Constructs an IllegalCharacterException with a detail message
	* @param message the detail message.
	*/
	IllegalCharacterException(const String& message) : 
		RuntimeException(message)
	{}
	
	virtual String getExceptionType() const {return QC_T("IllegalCharacterException");}
};

QC_BASE_NAMESPACE_END

#endif //QC_BASE_IllegalCharacterException_h


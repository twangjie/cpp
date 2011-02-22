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
// Class: UnmappableCharacterException
// 
/**
	@class qc::io::UnmappableCharacterException
	
	@brief Thrown when an OutputStreamWriter, that has been configured to abort
	when it encounters unmappable characters, writes a Unicode character that 
	cannot be mapped into the target encoding.

	@sa CodeConverter
*/
//==============================================================================

#ifndef QC_IO_UnmappableCharacterException_h
#define QC_IO_UnmappableCharacterException_h

#ifndef QC_CVT_DEFS_h
#include "defs.h"
#endif //QC_CVT_DEFS_h

#include "CharacterCodingException.h"

QC_IO_NAMESPACE_BEGIN

class QC_IO_PKG UnmappableCharacterException : public CharacterCodingException
{
public:
	/** Constructs an UnmappableCharacterException with a detail message and
	*   a decoder.
	* @param message the detail message
	* @param pDecoder the CodeConverter giving rise to the exception
	*/
	UnmappableCharacterException(const String& message, CodeConverter* pDecoder) : 
		CharacterCodingException(message, pDecoder)
	{}

	virtual String getExceptionType() const {return QC_T("UnmappableCharacterException");}
};

QC_IO_NAMESPACE_END

#endif //QC_IO_UnmappableCharacterException_h


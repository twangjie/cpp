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

#ifndef QC_IO_MalformedInputException_h
#define QC_IO_MalformedInputException_h

#ifndef QC_CVT_DEFS_h
#include "defs.h"
#endif //QC_CVT_DEFS_h

#include "CharacterCodingException.h"
#include "QcCore/cvt/CodeConverter.h"

QC_IO_NAMESPACE_BEGIN

class QC_IO_PKG MalformedInputException : public CharacterCodingException
{
public:
	MalformedInputException(const String& message, CodeConverter* pDecoder);
	MalformedInputException(const Byte* pStart, size_t len, CodeConverter* pDecoder);

	virtual String getExceptionType() const;
};


QC_IO_NAMESPACE_END

#endif //QC_IO_MalformedInputException_h


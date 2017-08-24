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
// Class: CodeConverterBase
// 
// Overview
// --------
// Base class that contains enums, values and static methods used by
// all types of CodeConverters.
//
//==============================================================================

#ifndef QC_BASE_CodeConverterBase_h
#define QC_BASE_CodeConverterBase_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include "String.h"
#include "QCObject.h"

QC_BASE_NAMESPACE_BEGIN

class QC_BASE_PKG CodeConverterBase : public virtual QCObject
{
public:
	enum Result {ok,              /*!< success, input buffer completely processed */
	             inputExhausted,  /*!< success, incomplete input sequence detected */
	             outputExhausted, /*!< success, output buffer full */
	             error,           /*!< conversion error */ 
	             noconv           /*!< no conversion required */ };

	static Result UTF8Encode(UCS4Char ch, Byte* to, const Byte* to_limit, Byte*& to_next);
	static Result UTF8Decode(UCS4Char& ch, const Byte *from, const Byte *from_end, const Byte *& from_next);
	static bool IsLegalUTF8(const Byte* pStart, size_t length);
	static bool IsLegalUTF16(const wchar_t* pStart, size_t length);

protected:
	static const char s_TrailingBytesForUTF8[256];
};

QC_BASE_NAMESPACE_END

#endif //QC_BASE_System_h

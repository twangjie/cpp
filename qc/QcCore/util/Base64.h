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

#ifndef QC_UTIL_Base64_h
#define QC_UTIL_Base64_h

#ifndef QC_UTIL_DEFS_h
#include "defs.h"
#endif //QC_UTIL_DEFS_h

QC_UTIL_NAMESPACE_BEGIN

class QC_UTIL_PKG Base64
{
public:
	enum Result {ok,              /*!< Operation successful */
	             outputExhausted, /*!< Output buffer too small for to accept result */
	             inputExhausted   /*!< Input buffer contains insufficient data */
	};

	static Result Encode(const Byte* pFrom, const Byte* pFromEnd, Byte* pTo, const Byte* pToLimit, Byte*& pToNext);
	static Result Decode(const Byte* pFrom, const Byte* pFromEnd, Byte* pTo, const Byte* pToLimit, Byte*& pToNext);
	static size_t GetEncodedLength(const Byte* pFrom, const Byte* pFromEnd);
	static size_t GetMaxDecodedLength(const Byte* pFrom, const Byte* pFromEnd);
};

QC_UTIL_NAMESPACE_END

#endif //QC_UTIL_Base64_h

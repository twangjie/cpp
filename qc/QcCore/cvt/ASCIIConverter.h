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
// Class: ASCIIConverter
// 
// Overview
// --------
// Provides a version of a CodeConverter that can encode/decode US_ASCII
// byte streams.
//
// As ASCII is a proper subset of ISO-10646 (UNICODE) it is not surprising
// that no encoding/decoding actually takes place - but we do trandform
// from byte to CharType and we do test for illegal values.
//
//=============================================================================

#ifndef QC_CVT_ASCIIConverter_h
#define QC_CVT_ASCIIConverter_h

#ifndef QC_CVT_DEFS_h
#include "defs.h"
#endif //QC_CVT_DEFS_h

#include "CodeConverter.h"

QC_CVT_NAMESPACE_BEGIN

class ASCIIConverter : public CodeConverter
{

public:

    virtual Result decode(const Byte *from, const Byte *from_end,
	                     const Byte *& from_next,
	                     CharType *to, CharType *to_limit,
	                     CharType *& to_next);

	virtual Result encode(const CharType *from, const CharType *from_end,
	                     const CharType *& from_next,
	                     Byte *to, Byte *to_limit,
	                     Byte *& to_next);

	virtual size_t getDecodedLength(const Byte *from, const Byte *from_end) const;

	virtual size_t getMaxEncodedLength() const;

	virtual bool alwaysNoConversion() const;

	virtual String getEncodingName() const;
};

QC_CVT_NAMESPACE_END

#endif //QC_CVT_ASCIIConverter_h

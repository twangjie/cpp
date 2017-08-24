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
// Class: Simple8BitConverter
// 
// Overview
// --------
// Provides a version of a CodeConverter that can encode/decode ISO-8859-1
// byte streams.
//
//=============================================================================

#ifndef QC_CVT_Simple8BitConverter_h
#define QC_CVT_Simple8BitConverter_h

#ifndef QC_CVT_DEFS_h
#include "defs.h"
#endif //QC_CVT_DEFS_h

#include "CodeConverter.h"
#include <map>

QC_CVT_NAMESPACE_BEGIN

class QC_CVT_PKG Simple8BitConverter : public CodeConverter
{

public:

	typedef unsigned short CodedChar;

	Simple8BitConverter(const String& name,
	                    const CodedChar* decodingTable);

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

protected:
	void generateEncodingMap();

private:
	typedef std::map<CodedChar, Byte, std::less<CodedChar> > EncodingMap;
	EncodingMap m_encodingMap;
	const CodedChar* m_pDecodingTable;
	String m_name;
};

QC_CVT_NAMESPACE_END

#endif //QC_CVT_Simple8BitConverter_h

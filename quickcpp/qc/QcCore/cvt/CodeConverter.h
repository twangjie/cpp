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
// Class: CodeConverter
// 
//=============================================================================

#ifndef QC_CVT_CodeConverter_h
#define QC_CVT_CodeConverter_h

#ifndef QC_CVT_DEFS_h
#include "defs.h"
#endif //QC_CVT_DEFS_h

#include "QcCore/base/CodeConverterBase.h"

QC_CVT_NAMESPACE_BEGIN

class QC_CVT_PKG CodeConverter : public virtual QCObject, public CodeConverterBase
{
public:
    
	enum CharAction {abort   /** throw exception when error detected*/,
	                 replace /** use a replacement character */
	};

	CodeConverter();

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

	void setInvalidCharAction(CharAction eAction);
	CharAction getInvalidCharAction() const;
	
	void setInvalidCharReplacement(UCS4Char ch);
	UCS4Char getInvalidCharReplacement() const;

	void setUnmappableCharAction(CharAction eAction);
	CharAction getUnmappableCharAction() const;
	
	void setUnmappableCharReplacement(UCS4Char ch);
	UCS4Char getUnmappableCharReplacement() const;

protected:
	virtual Result handleUnmappableCharacter(UCS4Char ch,
	                                         Byte*  to,
	                                         Byte*  to_limit,
	                                         Byte*& to_next);

	void handleInvalidByteSequence(const Byte* from, size_t len) const;
	void throwUnsupported(unsigned long illegalChar) const;
	void internalEncodingError(const CharType* from, size_t len) const;

private:
	void cannotEncodeChar(UCS4Char ch) const;

private:
	UCS4Char m_invalidCharReplacement;
	CharAction m_invalidCharAction;
	UCS4Char m_unmappableCharReplacement;
	CharAction m_unmappableCharAction;
};

QC_CVT_NAMESPACE_END

#endif //QC_CVT_CodeConverter_h

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
// Class: CharacterCodingException
// 
/**
	@class qc::io::CharacterCodingException
	
	@brief Base class for encoding exceptions.
	       
	Derived exceptions are thrown when an InputStreamReader reads
	a malformed byte sequence or when an OutputStreamWriter attempts
	to write a Unicode character that cannot be mapped into the
	output encoding.
*/
//==============================================================================

#ifndef QC_IO_CharacterCodingException_h
#define QC_IO_CharacterCodingException_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "IOException.h"
#include "QcCore/cvt/CodeConverter.h"

QC_IO_NAMESPACE_BEGIN

using cvt::CodeConverter;

class QC_IO_PKG CharacterCodingException : public IOException
{
public:
	/** Constructs a CharacterCodingException with a decoder.
	* @param pDecoder the CodeConverter giving rise to the exception
	*/
	CharacterCodingException(CodeConverter* pDecoder) :
		m_rpDecoder(pDecoder)
	{}

	/** Constructs a CharacterCodingException with a detail message and
	*   a decoder.
	* @param message the detail message
	* @param pDecoder the CodeConverter giving rise to the exception
	*/
	CharacterCodingException(const String& message, CodeConverter* pDecoder) : 
		IOException(message),
		m_rpDecoder(pDecoder)
	{}

	virtual String getExceptionType() const {return QC_T("CharacterCodingException");}

	/** Returns a reference to the CodeConverter that gave rise 
	*   to the exception.
	*   This may be used to change the policy of the CodeConverter before
	*   retrying the operation.
	*/
	AutoPtr<CodeConverter> getCodeConverter() const {return m_rpDecoder;}

private:
	AutoPtr<CodeConverter> m_rpDecoder;
};

QC_IO_NAMESPACE_END

#endif //QC_IO_CharacterCodingException_h


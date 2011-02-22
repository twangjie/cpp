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
// Class: MalformedInputException
// 
/**
	@class qc::io::MalformedInputException
	
	@brief Thrown when an InputStreamReader, that has been configured to abort
	when it encounters encoding errors, reads a malformed byte sequence.

	@sa CodeConverter
*/
//==============================================================================

#include "MalformedInputException.h"

#include "QcCore/base/ArrayAutoPtr.h"
#include "QcCore/base/StringUtils.h"

#include <stdio.h>

QC_IO_NAMESPACE_BEGIN

//==============================================================================
// MalformedInputException::MalformedInputException
//
/**
   Constructs a MalformedInputException with a detail message and
   a decoder.

   @param message the detail message
   @param pDecoder the CodeConverter giving rise to the exception
*/
//==============================================================================
MalformedInputException::MalformedInputException(const String& message, CodeConverter* pDecoder) : 
	CharacterCodingException(message, pDecoder)
{
}

//==============================================================================
// MalformedInputException::MalformedInputException
//
/**
	Constructs a MalformedInputException with a Byte array and a decoder.
	@param pStart pointer to the start of the array of badly encoded bytes
	@param len length of the array
	@param pDecoder the CodeConverter giving rise to the exception
*/
//==============================================================================
MalformedInputException::MalformedInputException(const Byte* pStart, size_t len, CodeConverter* pConv) :
	CharacterCodingException(pConv)
{
	QC_DBG_ASSERT(pStart!=0 && len!=0);

	size_t bufLen = len * 2 + 1;
	ArrayAutoPtr<char> apBuffer(new char[bufLen]);
	size_t j=0;
	for(size_t i=0; i<len && j<bufLen-2; i++)
	{
		j += ::sprintf(apBuffer.get()+j, "%02X", (unsigned)*(pStart+i));
	}

	String errMsg = QC_T("invalid ");
	errMsg += pConv->getEncodingName();
	errMsg += QC_T(" sequence: 0x");
	errMsg += StringUtils::FromLatin1(apBuffer.get());
	
	setMessage(errMsg);
}

//==============================================================================
// MalformedInputException::getExceptionType
//
//==============================================================================
String MalformedInputException::getExceptionType() const
{
	return QC_T("MalformedInputException");
}

QC_IO_NAMESPACE_END

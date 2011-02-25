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
// Class CodeConverter
//
/**
	@class qc::cvt::CodeConverter
	
	@brief Common base class for all code converters.  A CodeConverter
	translates Unicode characters into byte sequences and vice versa.

    The design of CodeConverter is based on the std::codecvt class
	from the C++ standard library.

    @QuickCPP comes supplied with CodeConverters for many common encodings
	such as UTF-8, UTF-16, Latin1 and many others.

	@sa CodeConverterFactory
*/
//==============================================================================

#include "CodeConverter.h"
#include "UTF8Converter.h"

#include "QcCore/base/IllegalCharacterException.h"
#include "QcCore/base/SystemCodeConverter.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/io/CharacterCodingException.h"
#include "QcCore/io/MalformedInputException.h"
#include "QcCore/io/UnmappableCharacterException.h"

#include <stdio.h>

QC_CVT_NAMESPACE_BEGIN

using io::CharacterCodingException;
using io::MalformedInputException;
using io::UnmappableCharacterException;

const UCS4Char DefaultCharReplacement = 0xFFFD;
const CodeConverter::CharAction DefaultAction = CodeConverter::replace;
const UCS4Char DefaultUnmappable = '?';

//==============================================================================
// CodeConverter::CodeConverter
//
/**
   Creates a CodeConverter with default values.
*/
//==============================================================================
CodeConverter::CodeConverter() :
	m_invalidCharReplacement(DefaultCharReplacement),
	m_invalidCharAction(DefaultAction),
	m_unmappableCharReplacement(DefaultUnmappable),
	m_unmappableCharAction(DefaultAction)
{
}

//==============================================================================
// CodeConverter::decode
//
/**
   Decodes an array of bytes into an array of ::CharType characters that
   represent Unicode characters in the internal @QuickCPP encoding.
   @param from pointer to the start of the byte array to decode
   @param from_end pointer to the next byte past the end of the byte array
   @param from_next return parameter which holds a pointer to the next byte
          in the array which has yet to be processed
   @param to pointer to the start of a ::CharType array which will hold the
          result of the decoding operation
   @param to_limit pointer to the next ::CharType past the end of the result array
   @param to_next return parameter which holds a pointer to the next ::CharType
          in the result array
   @returns a ::Result code indicating the success of the operation.
   @throws MalformedInputException if an invalid byte sequence is detected and
           the policy for this CodeConverter is to abort in this situation.
*/
//==============================================================================
CodeConverter::Result CodeConverter::decode(const Byte* /*from*/, const Byte* /*from_end*/,
                                     const Byte*& /*from_next*/,
                                     CharType*  /*to*/, CharType* /*to_limit*/,
                                     CharType*& /*to_next*/)
{
	return noconv;
}


//==============================================================================
// CodeConverter::encode
//
/**
   Encodes an array of ::CharType characters, representing Unicode characters
   in the internal @QuickCPP encoding, into an array of bytes.

   @param from pointer to the start of the ::CharType array to encode
   @param from_end pointer to the next ::CharType past the end of the input array
   @param from_next return parameter which holds a pointer to the next ::CharType
          in the array which has yet to be processed
   @param to pointer to the start of a byte array which will hold the
          result of the encoding operation
   @param to_limit pointer to the next byte past the end of the result array
   @param to_next return parameter which holds a pointer to the next byte
          in the result array
   @returns a ::Result code indicating the success of the operation.
   @throws UnmappableCharacterException if an unmappable Unicode character is detected
          and the policy for this CodeConverter is to abort in this situation.
*/
//==============================================================================
CodeConverter::Result CodeConverter::encode(const CharType* /*from*/, const CharType* /*from_end*/,
                                     const CharType*& /*from_next*/,
                                     Byte*  /*to*/, Byte* /*to_limit*/,
                                     Byte*& /*to_next*/)
{
	return noconv;
}

//==============================================================================
// CodeConverter::getDecodedLength
//
/**
   Returns the number of Unicode characters that would be created by decoding
   the array of bytes starting at @c from.  Depending on the internal encoding
   in use by @QuickCPP, this is not necessarily the same number of ::CharType
   characters that will be required to represent the Unicode characters.

   @param from pointer to the start of an encoded array of bytes
   @param from_end pointer to the next byte after the end of the array
   @returns the number of Unicode characters represented by the byte sequence
*/
//==============================================================================
size_t CodeConverter::getDecodedLength(const Byte *from, const Byte *from_end) const
{
	//
	// As the base class assumes noconv, the input bytes are in the same
	// encoding as the internal encoding.
	//
	return from_end - from;
}

//==============================================================================
// CodeConverter::getMaxEncodedLength
//
/**
   Returns the maximum number of bytes used to encode a single Unicode
   character up to U+10FFFF.
*/
//==============================================================================
size_t CodeConverter::getMaxEncodedLength() const
{
	return 4;
}

//==============================================================================
// CodeConverter::alwaysNoConversion
//
/**
   Tests if this CodeConverter is using the same encoding as the @QuickCPP
   internal encoding.  If so, the reading and writing of characters
   can be optimized to by-pass the encoding process.

   @returns true if this CodeConverter encodes Unicode characters into
   the @QuickCPP internal encoding; false otherwise
*/
//==============================================================================
bool CodeConverter::alwaysNoConversion() const
{
	return true;
}

//==============================================================================
// CodeConverter::getEncodingName
//
/**
   Returns the canonical name for the encoding handled by this CodeConverter.
*/
//==============================================================================
String CodeConverter::getEncodingName() const
{
	return String();
}

//==============================================================================
// CodeConverter::setInvalidCharAction
//
/**
   Sets the policy for dealing with badly encoded byte sequences.
   Two policies are supported: @a replace or @a abort.
   
   When the action  is set to CodeConverter::abort, a MalformedInputException
   is thrown by decode() when an invalid byte sequence is decoded.
   When the action is set to CodeConverter::replace, the invalid byte sequence
   is decoded as the replacement character returned from
   getInvalidCharReplacement().

   @param eAction the required action to take.

   @sa getInvalidCharAction()
*/
//==============================================================================
void CodeConverter::setInvalidCharAction(CharAction eAction)
{
	m_invalidCharAction = eAction;
}

//==============================================================================
// CodeConverter::getInvalidCharAction
//
/**
   Returns the policy for dealing with invalid byte sequences.

   @sa setInvalidCharAction()
*/
//==============================================================================
CodeConverter::CharAction CodeConverter::getInvalidCharAction() const
{
	return m_invalidCharAction;
}

//==============================================================================
// CodeConverter::setInvalidCharReplacement
//
/**
   Sets the replacement Unicode character used when the CodeConverter detects an
   invalid byte sequence.

   @sa setInvalidCharAction()
*/
//==============================================================================
void CodeConverter::setInvalidCharReplacement(UCS4Char c)
{
	m_invalidCharReplacement = c;
}

//==============================================================================
// CodeConverter::getInvalidCharReplacement
//
/**
   Returns the Unicode character that will be used when this CodeConverter
   detects an invalid byte sequence.

   @sa getInvalidCharAction()
*/
//==============================================================================
UCS4Char CodeConverter::getInvalidCharReplacement() const
{
	return m_invalidCharReplacement;
}

//==============================================================================
// CodeConverter::setUnmappableCharAction
//
/**
   Sets the policy for dealing with Unicode characters that cannot be mapped
   into the target encoding. Two policies are supported: @a replace or @a abort.
   
   When the action  is set to CodeConverter::abort, an UnmappableCharacterException
   is thrown by encode() when an unmappable Unicode character is encoded.
   When the action is set to CodeConverter::replace, the unmappable character
   is replaced by the character returned from
   getUnmappableCharReplacement().

   @param eAction the required action to take.

   @sa getUnmappableCharAction()
*/
//==============================================================================
void CodeConverter::setUnmappableCharAction(CharAction eAction)
{
	m_unmappableCharAction = eAction;
}

//==============================================================================
// CodeConverter::getUnmappableCharAction
//
/**
   Returns the policy for dealing with Unicode characters that cannot be mapped
   into the target encoding.

   @sa setUnmappableCharAction()
*/
//==============================================================================
CodeConverter::CharAction CodeConverter::getUnmappableCharAction() const
{
	return m_unmappableCharAction;
}

//==============================================================================
// CodeConverter::setUnmappableCharReplacement
//
/**
   Sets the replacement Unicode character used when the CodeConverter detects a
   Unicode character than cannot be encoded into the target encoding.

   @sa setUnmappableCharAction()
*/
//==============================================================================
void CodeConverter::setUnmappableCharReplacement(UCS4Char ch)
{
	m_unmappableCharReplacement = ch;
}

//==============================================================================
// CodeConverter::getUnmappableCharReplacement
//
/**
   Returns the Unicode character that will be used when this CodeConverter
   detects an unmappable Unicode character.

   @sa getUnmappableCharAction()
*/
//==============================================================================
UCS4Char CodeConverter::getUnmappableCharReplacement() const
{
	return m_unmappableCharReplacement;
}

//==============================================================================
// CodeConverter::internalEncodingError
//
/**
   Helper function called by @a derived classes when they encounter
   a badly encoded internal ::CharType array.

   @param from pointer to the start of the array
   @len length of the array
*/
//==============================================================================
void CodeConverter::internalEncodingError(const CharType* from, size_t len) const
{
	String errMsg = QC_T("invalid internal ");
	errMsg += SystemCodeConverter::GetInternalEncodingName();
	errMsg += QC_T(" character sequence: 0x");
	ByteString hexString;

	static const char* format =

#if defined(QC_UNICODE)
		"%04X";
#else
		"%02X";
#endif

	for (size_t i=0; i<len; ++i)
	{
		hexString += StringUtils::Format(format, (int)(UCharType)*from++);
	}
	errMsg += StringUtils::FromLatin1(hexString);

	throw IllegalCharacterException(errMsg);
}

//==============================================================================
// CodeConverter::handleInvalidByteSequence
//
/**
   Helper function that simply throws a MalformedInputException.

   @throws MalformedInputException always
*/
//==============================================================================
void CodeConverter::handleInvalidByteSequence(const Byte* from, size_t len) const
{
	throw MalformedInputException(from, len, const_cast<CodeConverter*>(this));
}

//==============================================================================
// CodeConverter::throwUnsupported
//
//==============================================================================
void CodeConverter::throwUnsupported(unsigned long illegalChar) const
{
	char buffer[20];
	sprintf(buffer, "0x%02X", (unsigned)illegalChar);

	String errMsg = QC_T("Unicode character: ");
	errMsg += StringUtils::FromLatin1(buffer);
	errMsg += QC_T(" is unsupported on this platform/configuration");
	throw CharacterCodingException(errMsg, const_cast<CodeConverter*>(this));
}

//==============================================================================
// CodeConverter::cannotEncodeChar
//
// Private helper.
//==============================================================================
void CodeConverter::cannotEncodeChar(UCS4Char ch) const
{
	String errMsg = QC_T("Unicode character: ");
	errMsg += StringUtils::FromLatin1(StringUtils::Format("U+%04X", (unsigned)ch));
	errMsg += QC_T(" cannot be encoded into ");
	errMsg += getEncodingName();
	throw UnmappableCharacterException(errMsg, const_cast<CodeConverter*>(this));
}

//==============================================================================
// CodeConverter::handleUnmappableCharacter
//
/**
   Helper function called by @a derived classes' encode() method when 
   it encounters an unmappable Unicode character.

   @param ch the unmappable Unicode character
   @param to pointer to the next byte in the output byte array for the current
          encoding operation
   @param to_limit pointer to the next byte after the end of the output byte buffer
   @param to_next return parameter which holds a pointer to the next byte
          in the result array

   @returns a ::Result code indicating the success of the operation
*/
//==============================================================================
CodeConverter::Result CodeConverter::handleUnmappableCharacter(UCS4Char ch,
	Byte*  to,
	Byte*  to_limit,
	Byte*& to_next)
{
	to_next = to;
	if(getUnmappableCharAction() == abort)
	{
		cannotEncodeChar(ch);
		return error; // should not reach here
	}
	else
	{
		Character repl(m_unmappableCharReplacement);
		const CharType* from_next;
		Result ret = encode(repl.data(), repl.data()+repl.length(), from_next,
		                    to, to_limit, to_next);

		//
		// If we cannot encode the replacement character then there is no
		// hope left, we must throw an exception
		//
		if(ret == error)
			cannotEncodeChar(ch);
	
		return ret;
	}
}

QC_CVT_NAMESPACE_END

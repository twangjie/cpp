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
/**
	@class qc::StringUtils
	
	@brief Class module containing functions to compare and manipulate strings.

	@sa NumUtils
*/
//==============================================================================

#include "StringUtils.h"
#include "ArrayAutoPtr.h"
#include "AutoBuffer.h"
#include "Exception.h"
#include "StringIterator.h"
#include "SystemCodeConverter.h"
#include "OSException.h"
#include "UnicodeCharacterType.h"

#ifdef WIN32
#include "Win32Exception.h"
#endif //WIN32

#include <algorithm>
#include <memory>

#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>


QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// StringUtils::CompareNoCase
//
/**
   Compares two strings without regard to case.

   @param lhs the first String
   @param rhs the second String
   @returns -1 if @c lhs compares less than @c rhs, 0 if they are equal or
   +1 if @c lhs compares greater than @c rhs
*/
// This simple string comparison function is borrowed from Stroustrup 3rd
// Edition 20.3.8 p591.  It assumes that each member of the controlled
// sequence is a character which is not really adequate.  However, when used
// with ASCII strings it works okay.
//
// todo: re-work when Unicode database is available
//==============================================================================
int StringUtils::CompareNoCase(const String& lhs, const String& rhs)
{
	String::const_iterator ilhs = lhs.begin();
	String::const_iterator irhs = rhs.begin();

	while(ilhs!=lhs.end() && irhs!=rhs.end())
	{
		if(toupper(*ilhs)!=toupper(*irhs))
			return (toupper(*ilhs) < toupper(*irhs)) ? -1 : 1;

		irhs++;
		ilhs++;
	}
	return (rhs.size() ==lhs.size()) ? 0
	                                 : (lhs.size() < rhs.size()) ? -1 : 1;
}

//==============================================================================
// StringUtils::CompareNoCase
//
/**
   Compares two null-terminated sequences of @c char disregarding case differences.
   @param pszLHS the first character sequence to compare
   @param pszRHS the second character sequence to compare
   @returns -1 if @c pszLHS is less than @c pszRHS, 0 if they are equal
   or +1 if @c pszLHS is greater than @c pszRHS
*/
//==============================================================================
int StringUtils::CompareNoCase(const char* pszLHS, const char* pszRHS)
{
#if defined(WIN32)
	return stricmp(pszLHS, pszRHS);
#else
	return strcasecmp(pszLHS, pszRHS);
#endif
}

//==============================================================================
// StringUtils::ToUpper
//
/**
   Returns a String representation of @c str with all characters converted to
   upper-case.

   @param str the String to convert to upper case.
*/
//==============================================================================
String StringUtils::ToUpper(const String& str)
{
	StringIterator i(str.data());
	StringIterator end(str.data()+str.size());
	String ret;
	ret.reserve(str.length());

	while(i != end)
	{
		ret += toupper((*i++).toUnicode());
	}
	return ret;
}

//==============================================================================
// StringUtils::ToLower
//
/**
   Returns a String representation of @c str with all characters converted to
   lower-case.

   @param str the String to convert to lower case.
*/
//==============================================================================
String StringUtils::ToLower(const String& str)
{
	StringIterator i(str.data());
	StringIterator end(str.data()+str.size());

	String ret;
	ret.reserve(str.length());

	while(i != end)
	{
		ret += tolower((*i++).toUnicode());
	}
	return ret;
}

//==============================================================================
// StringUtils::ToUTF8
//
/**
   Converts the passed String into a char string encoded as UTF-8.

   @throws IllegalCharacterException if @c str contains an illegal character
           or is not encoded in accordance with the internal @QuickCPP String
		   encoding conventions.

   @returns A UTF-8 encoded version of the String @c str.
*/
//==============================================================================
ByteString StringUtils::ToUTF8(const String& str)
{
#ifdef QC_UTF8
	// The internal encoding is already UTF-8 so we have nothing to do!
	return str;

#else // !QC_UTF8

	ByteString ret;
	const size_t workBuffSize = 256;
	Byte workBuffer[workBuffSize];
	StringIterator i(str.data());
	StringIterator end(str.data()+str.size());
	Byte* pNext=workBuffer;
	const Byte* const pBufferEnd = workBuffer+workBuffSize;

	for(; i!=end; ++i)
	{
		// If we have insufficient room left in the work buffer for a
		// large character, flush it 
		if(pNext+4 > (workBuffer+workBuffSize))
		{
			ret.append((char*)workBuffer, pNext-workBuffer);
			pNext=workBuffer;
		}
		const UCS4Char ch = (*i).toUnicode();
		CodeConverterBase::Result result = 
			CodeConverterBase::UTF8Encode(ch, pNext, pBufferEnd, pNext);
		if(result != CodeConverterBase::ok) throw IllegalCharacterException();
	}
	ret.append((char*)workBuffer, pNext-workBuffer);
	return ret;

#endif //QC_UTF8
}

//==============================================================================
// StringUtils::FromUTF8
//
/**
   Converts a UTF-8 encoded Byte sequence into an internal @QuickCPP String.  If the
   internal encoding is already UTF-8, the input string is simple returned
   unchanged.

   @throws IllegalCharacterException if @c str contains an illegal UTF-8
           sequence.

   @returns A String representing the UTF-8 encoded Byte sequence.

*/
//==============================================================================
String StringUtils::FromUTF8(const ByteString& str)
{
#if defined(QC_UTF8)

	return str;

#else

	AutoBuffer<CharType> buffer(256);

	const Byte* pFrom = (const Byte*) str.data();
	const Byte* pFromEnd = pFrom + str.length();
	
	while(pFrom < pFromEnd)
	{
		const Byte* pFromNext;
		UCS4Char ch;

		if(CodeConverterBase::UTF8Decode(ch, pFrom, pFromEnd, pFromNext) != CodeConverterBase::ok)
			throw IllegalCharacterException();

		pFrom = pFromNext;
		Character x(ch);
		buffer.append(x.data(), x.length());
	}

	return String(buffer.data(), buffer.size());

#endif //QC_UTF8
}

//==============================================================================
// StringUtils::ToLatin1
//
/**
   Converts the passed String into a char string encoded as Latin-1
   
   The Latin-1 encoding (used throughout Europe) is officially known as ISO-8859-1.

   @throws IllegalCharacterException if @c str contains a character that cannot
           be encoded as ISO-8859-1 (any character with a Unicode code-point above
		   U+00FF) or is not encoded in accordance with the internal @QuickCPP String
		   encoding conventions.

   @returns A Latin-1 encoded version of the String @c str
*/
//==============================================================================
ByteString StringUtils::ToLatin1(const String& str)
{
	AutoBuffer<Byte> workBuffer;
	StringIterator i(str.data());
	StringIterator end(str.data()+str.size());

	for(; i!=end; ++i)
	{
		const UCS4Char ch = (*i).toUnicode();
		if(ch<=0x00FFU)
			workBuffer.append(Byte(ch));
		else
			throw IllegalCharacterException();
	}

	return ByteString((char*)workBuffer.data(), workBuffer.size());
}

//=============================================================================
// StringUtils::ToAscii
// 
/**
   Converts the passed String into a char string encoded as US-ASCII
   
   The US-ASCII encoding is a subset of Unicode with characters in the
   range U+0000-U+007F.

   @throws IllegalCharacterException if @c str contains a character that cannot
           be encoded as US-ASCII (any character with a Unicode code-point above
		   0x007F) or is not encoded in accordance with the internal @QuickCPP String
		   encoding conventions.

   @returns A US-ASCII version of the String @c str
*/
//=============================================================================
ByteString StringUtils::ToAscii(const String& str)
{
	AutoBuffer<Byte> workBuffer;
	StringIterator i(str.data());
	StringIterator end(str.data()+str.size());

	for(; i!=end; ++i)
	{
		const UCS4Char ch = (*i).toUnicode();
		if(ch<=0x007FU)
			workBuffer.append(Byte(ch));
		else
			throw IllegalCharacterException();
	}

	return ByteString((char*)workBuffer.data(), workBuffer.size());
}

//==============================================================================
// StringUtils::FromLatin1
//
/**
   Converts a null-terminated sequence of @c char characters into a 
   @QuickCPP String.

   The input @c char sequence is encoded in Latin1 (ISO-8859-1),
   i.e. each @c char is an unsigned byte with a value in the Unicode range
   U+0000-U+0255.
*/
//==============================================================================
String StringUtils::FromLatin1(const char* pStr)
{
	String strRet;
	size_t len = strlen(pStr);
	strRet.reserve(len);
	const unsigned char* iter = (const unsigned char*)pStr;
	const unsigned char* end  = iter + len;

	for(; iter!=end; ++iter)
	{
		// use unsigned so that we don't propogate the sign
		Character x((UCS4Char)*iter);
		x.appendToString(strRet);
	}
	return strRet;
}

//==============================================================================
// StringUtils::FromLatin1
//
/**
   Converts an array of @c char characters into a @QuickCPP String.

   The input @c char sequence is encoded in Latin1 (ISO-8859-1),
   i.e. each @c char is an unsigned byte with a value in the Unicode range
   U+0000-U+0255.
   @param pStr pointer to the start of the @c char array
   @param len size 
*/
//==============================================================================
String StringUtils::FromLatin1(const char* pStr, size_t len)
{
	String strRet;
	strRet.reserve(len);

	const char* iter = pStr;
	const char* end  = pStr + len;

	for(; iter!=end; ++iter)
	{

		//
		// If we are in wchar_t mode, then each Latin1 character can simply
		// be padded with leading zeros and added to the string.  Otherwise
		// we need to encode ISO-8859-1 into UTF-8 which is sufficiently
		// complicated to enlist the help of the Character class.
		//
#ifdef QC_WCHAR
		// use unsigned so that we don't propogate the sign into the wide char
		CharType x = (unsigned char)(*iter);
		strRet += x;
#else
		Character x((unsigned char)*iter);
		x.appendToString(strRet);
#endif

	}

	return strRet;
}

//==============================================================================
// StringUtils::FromLatin1
//
/**
   Converts a string of @c char characters into a @QuickCPP String.

   The input string is encoded in Latin1 (ISO-8859-1),
   i.e. each @c char member of @c str is an unsigned byte with a value in the
   Unicode range U+0000-U+0255.
*/
//==============================================================================
String StringUtils::FromLatin1(const ByteString& str)
{
	return FromLatin1(str.data(), str.length());
}

//==============================================================================
// StringUtils::ReplaceAll
//
/**
   Replaces all ::CharType elements from in a String with a replacement String.

   @param in a String containing a sequence of one or more ::CharType characters
   @param search the ::CharType to match against each element of @c in
   @param replacement a String containing a sequence of or more ::CharType characters
   that will be used to replace matching elements from @c in
   @returns true if at least one matching ::CharType was found
*/
//==============================================================================
bool StringUtils::ReplaceAll(String& in, CharType search, const String& replacement)
{
	bool bRet = false;
	String::size_type pos = 0;
	while( pos < in.length() && (pos=in.find(search, pos))!=String::npos )
	{
		bRet = true;
		in.replace(pos, 1, replacement);
		pos += replacement.length();
	}
	return bRet;
}

//=============================================================================
// StringUtils::ReplaceAll
// 
/**
   Replaces all ::CharType sequences from a String that match the @c search
   String with a replacement String

   @param in a String containing a sequence of one or more ::CharType characters
   @param search a String containing a sequence of ::CharType characters to match
          against sub-sequences of @c in
   @param replacement a String containing a sequence of one or more ::CharType characters
          that will be used to replace matched sequences from @c in
   @returns true if at least one matching sequence was found
*/
//=============================================================================
bool StringUtils::ReplaceAll(String& in, const String& search, const String& replacement)
{
	bool bRet = false;
	String::size_type pos = 0;
	while( pos < in.length() && (pos=in.find(search, pos))!=String::npos )
	{
		bRet = true;
		in.replace(pos, search.length(), replacement);
		pos += replacement.length();
	}
	return bRet;
}

//==============================================================================
// StringUtils::StripWhiteSpace
//
/**
   Strips white-space from a String.

   The definition of white-space is taken from UnicodeCharacterType::IsSpace().

   @returns a copy of @c in with the requested white-space removed
   @param in the String to process
   @param type an enum value specifying from where the white-space should be removed
   (StringUtils::leading, StringUtils::trailing or StringUtils::both)
   @sa UnicodeCharacterType::IsSpace()
*/
//==============================================================================
String StringUtils::StripWhiteSpace(const String& in, eStripType type)
{
	size_t startPos=0;

	if(type == leading || type == both)
	{
		while(startPos < in.size()-1)
		{
			CharType x=in[startPos];
			if(UnicodeCharacterType::IsSpace(x))
			{
				++startPos;
			}
			else
			{
				break;
			}
		}
	}

	size_t endPos=in.length();

	if(type == trailing || type == both)
	{
		while(endPos > startPos)
		{
			CharType x=in[endPos-1];
			if(UnicodeCharacterType::IsSpace(x))
			{
				--endPos;
			}
			else
			{
				break;
			}
		}
	}

	if(startPos == endPos)
		return String();
	else
		return in.substr(startPos, endPos-startPos);
}

//==============================================================================
// StringUtils::NormalizeWhiteSpace
//
/**
   Normalize a String value by removing all leading and trailing white-space
   and converting sequences of more than one white-space character into a single
   space character (U+0032).

   The definition of white-space is taken from UnicodeCharacterType::IsSpace().

   @returns the normalized String
   @param in the String to normalize
   @sa UnicodeCharacterType::IsSpace()
*/
//==============================================================================
String StringUtils::NormalizeWhiteSpace(const String& in)
{
	String ret;
	ret.reserve(in.size());

	size_t startPos = 0;
	const size_t inSize = in.size();

	for(; startPos < inSize; ++startPos)
	{
		CharType x=in[startPos];
		if(!UnicodeCharacterType::IsSpace(x))
		{
			break;
		}
	}

	bool bSeenWhitespace = false;

	for(; startPos < inSize; ++startPos)
	{
		CharType x=in[startPos];
		if(UnicodeCharacterType::IsSpace(x))
		{
			bSeenWhitespace = true;
		}
		else
		{
			if(bSeenWhitespace)
			{
				bSeenWhitespace = false;
				ret += QC_T(" ");
			}
			ret += x;
		}
	}
	return ret;
}

//==============================================================================
// StringUtils::IsHexString
//
/**
   Tests if the passed string contains only hexadecimal characters
   [0-9], [a-f], [A-F]

   @param in the string to test
   @returns true if the passed String contains hexadecimal characters only;
            false otherwise
*/
//==============================================================================
bool StringUtils::IsHexString(const ByteString& in)
{
	for(ByteString::const_iterator i=in.begin(); i!=in.end(); ++i)
	{
		const char x = (*i);
		if(!isxdigit(x))
		{
			return false;
		}
	}
	return !in.empty();
}

//==============================================================================
// StringUtils::Format
//
/**
   A wrapper around the c-library sprintf routine.
*/
// Note: We first attempt to perform the string formatting using a stack-based
// buffer of a reasonable size.  If that fails then we use a buffer allocated
// from the free store.
//==============================================================================
ByteString StringUtils::Format(const char* fmt, ...)
{

#ifdef _MSC_VER
#define VSNPRINTF _vsnprintf
#else
#define VSNPRINTF vsnprintf
#endif

	String ret;
	const int stackBufferSize = 255;
	char stackBuffer[stackBufferSize];

	va_list ap;
	va_start(ap, fmt);
	int n = VSNPRINTF(stackBuffer, stackBufferSize, fmt, ap);
	va_end(ap);

	/* If that worked, return the string. */
	if (n > -1 && n < stackBufferSize)
	{
		return ByteString((const char*)stackBuffer, n);
	}

	int dynamSize = stackBufferSize * 2;

	while (true)
	{
		ArrayAutoPtr<char> pBuffer(new char[dynamSize]);
		//rely on global operator new throwing exceptions
		//if(!pBuffer.get()) throw std::bad_alloc();

		va_start(ap, fmt);
		n = VSNPRINTF(pBuffer.get(), dynamSize, fmt, ap);
		va_end(ap);

		/* If that worked, return the string. */
		if (n > -1 && n < dynamSize)
		{
			return ByteString(pBuffer.get(), n);
		}

		if(n == dynamSize)
		{
			dynamSize++;
		}
		else
		{
			dynamSize *= 2;
		}
	}
}

//==============================================================================
// StringUtils::ContainsMultiCharSequence
//
/**
   Tests to see if the String contains one or more Unicode characters
   expressed as a sequence of more than one ::CharType character.

   @param str the String to test
   @returns true if @c str contains a character that is encoded using a sequence
   of more than one ::CharType character
*/
//==============================================================================
bool StringUtils::ContainsMultiCharSequence(const String& str)
{
#ifdef QC_UCS4

	return false;

#else

	String::size_type len = str.length();
	const CharType* pData = str.data();
	for(String::size_type i = 0; i<len; ++i)
	{
		if(SystemCodeConverter::GetCharSequenceLength(pData[i]) > 1)
			return true;
	}
	return false;

#endif // !QC_UCS4
}

//==============================================================================
// StringUtils::FromConsoleMBCS
//
/**
   Translates a Multi-Byte Character String (MBCS) originating from the
   console into a @QuickCPP String.
*/
//==============================================================================
String StringUtils::FromConsoleMBCS(const char* pStr)
{
#if defined(WIN32)

	return FromMBCSCodePage(pStr, ::GetConsoleCP());

#else

	return FromNativeMBCS(pStr);

#endif
}

//==============================================================================
// StringUtils::ToConsoleMBCS
//
/**
   Translates a @QuickCPP string into a Multi-Byte Character String (MBCS)
   intended to be displayed on the console.
*/
//==============================================================================
ByteString StringUtils::ToConsoleMBCS(const String& str)
{
#if defined(WIN32)

	return ToMBCSCodePage(str, ::GetConsoleCP());

#else

	return ToNativeMBCS(str);

#endif
}

//==============================================================================
// StringUtils::FromNativeMBCS
//
/**
   Translates a native (locale-dependent) Multi-Byte Character String (MBCS)
   into a @QuickCPP String.

   Applications have to deal with MBCS strings when making system calls
   and accepting (some) system input.  Internal @QuickCPP Strings are composed of
   Unicode characters, encoded using one of either UTF-8, UTF16 or UCS4
   which is defined at compile time.
*/
//==============================================================================
String StringUtils::FromNativeMBCS(const char* pStr)
{

#if defined(WIN32)

	return FromMBCSCodePage(pStr, ::GetACP());

#else

	const size_t len = strlen(pStr);

	if(!len)
		return String();

	//
	// If the current locale uses the same MBCS encoding as the internal Celio
	// encoding then, hey presto, there is nothing to do.  However, testing for
	// this situation may be easier said than done.  We do know, however, that
	// this will not be the case if Celio is using wchar_t characters.
	//

#if defined(QC_UTF16) || defined(QC_UCS2) || defined(QC_UCS4)

	//
	// We are assuming that the current locale encodes wide characters
	// according to Unicode / ISO 10646.  This is the case on Linux
	// but is not necessarily so on all platforms.
	//
	// A buffer containing strlen(str)+1 wchar_t chracters must be large
	// enough.
	const size_t bufLen = len+1;

	ArrayAutoPtr<wchar_t> apWideBuffer(new wchar_t[bufLen]);
	size_t numChars = ::mbstowcs(apWideBuffer.get(), pStr, bufLen);
	if(numChars == (size_t)-1)
		throw OSException(errno, QC_T("mbstowcs"));

	return String(apWideBuffer.get(), numChars);

#elif defined(QC_UTF8)

	//
	// We are using UTF-8 internally, is the MBCS encoding UTF-8 also?
	//

	// todo: detect locale character encoding for Unix/Linux
	// assume Latin-1 for now
	return FromLatin1(pStr);

#else

	#error unrecognized internal encoding

#endif
#endif //WIN32
}

//==============================================================================
// StringUtils::ToNativeMBCS
//
/**
   Converts an internal String into a native (locale-dependent) Multi-Byte
   Character String (MBCS).

   @QuickCPP Strings are stored in an encoded Unicode format.  This function
   translates from the internal format to the MBCS format for the current
   locale.
   @param str the String to translate
   @throws IllegalCharacterException if @c str is not encoded according to
           the @QuickCPP internal encoding or @c str contains a Unicode
		   character that cannot be represented in the native MBCS format.
*/
//==============================================================================
ByteString StringUtils::ToNativeMBCS(const String& str)
{
#if defined(WIN32)

	return ToMBCSCodePage(str, ::GetACP());

#else

	if(str.empty())
		return ByteString();

	//
	// If the current locale uses the same MBCS encoding as the internal Celio
	// encoding then, hey presto, there is nothing to do.  However, testing for
	// this situation may be easier said than done.  We do know, however, that
	// this will not be the case if Celio is using wchar_t characters.
	//

#ifdef QC_WCHAR

	//
	// Calculate a reasonable length for the returned buffer
	//
	const size_t bufLen = (str.length() * 4)+1;

	ArrayAutoPtr<char> apCharBuffer(new char[bufLen]);

	#if defined(HAVE_WCSNRTOMBS) && defined(__USE_GNU)
		const wchar_t* pSrc = str.data();
		mbstate_t mbs;
		memset (&mbs, 0, sizeof(mbs));
		size_t numChars = str.size() ? ::wcsnrtombs(apCharBuffer.get(), &pSrc, str.size(), bufLen, &mbs)
		                             : 0;
	#elif defined(QC_BROKEN_C_STR)
		// libstdc++ v2 has a bug where it attempt to return "" as a wide string in c_str().
		// As a result QuickCPP does not use c_str() anywhere - except here.  This code
		// avoids it by allocating a new buffer and zero-terminating it.  Painful, but temporary!
		//	
		ArrayAutoPtr<wchar_t> apStrBuffer(new wchar_t[str.length() + 1]);
		::memcpy(apStrBuffer.get(), str.data(), str.length() * sizeof(wchar_t));
		*(apStrBuffer.get()+str.length()) = 0;
		size_t numChars = ::wcstombs(apCharBuffer.get(), apStrBuffer.get(), bufLen);
	#else
		size_t numChars = ::wcstombs(apCharBuffer.get(), str.c_str(), bufLen);
	#endif

	if(numChars == (size_t)-1)
		throw OSException(errno, QC_T("wcstombs"));

	return ByteString(apCharBuffer.get(), numChars);

#else

	//todo: detect locale character encoding...
	return ToLatin1(str);

#endif
#endif
}

#if defined(WIN32)
//==============================================================================
// StringUtils::ToWin32String
//
/**
   Converts a String into a LPCTSTR string suitable for passing to Win32 API
   functions.

   @param str the String to convert
   @returns an ArrayAutoPtr containing a pointer to a new LPCTSTR string
*/
//==============================================================================
StringUtils::TCharAutoPtr StringUtils::ToWin32String(const String& str)
{
#ifdef UNICODE
	#ifdef QC_WCHAR
		//return wcsdup(str.c_str()); //does not use new()
		StringUtils::TCharAutoPtr apRet(new CharType [mbcs.size() +1]);
		::memcpy(apRet.get(), mbcs.data(), mbcs.length()*sizeof(CharType));
		*(apRet.get()+mbcs.length()) = 0;
		return apRet;
	#else // !QC_WCHAR
		#error WIN32 UNICODE cannot be used without wide character support		
	#endif // QC_WCHAR
#else //!UNICODE
	//return strsup(str.c_str()); //does not use new()
	ByteString mbcs = ToNativeMBCS(str);
	StringUtils::TCharAutoPtr apRet(new char [mbcs.size() +1]);
	::memcpy(apRet.get(), mbcs.data(), mbcs.size());
	*(apRet.get()+mbcs.size()) = 0;
	return apRet;
#endif
}

//==============================================================================
// StringUtils::FromWin32String
//
/**
   Converts a LPCTSTR string into a @QuickCPP String.

   @param lpStr the Win32 null-terminated string to convert
   @returns a String representation of @c lpStr
*/
//==============================================================================
String StringUtils::FromWin32String(LPCTSTR lpStr)
{
#ifdef UNICODE
	#ifdef QC_WCHAR
		return lpStr;
	#else // !QC_WCHAR
		#error WIN32 UNICODE cannot be used without wide character support
	#endif
#else // !UNICODE
	return FromNativeMBCS(lpStr);
#endif // UNICODE
}

//==============================================================================
// StringUtils::FromMBCSCodePage
//
/**
   Converts a multi-byte character string into a @QuickCPP String.
*/
//==============================================================================
String StringUtils::FromMBCSCodePage(const char* pStr, int codepage)
{
	const size_t len = strlen(pStr);
	
	if(!len)
		return String();

	const size_t bufLen = len+1;

	ArrayAutoPtr<wchar_t> apWideBuffer(new wchar_t[bufLen]);
	wchar_t* pWideBuffer = apWideBuffer.get();
	int numChars = ::MultiByteToWideChar(codepage, 0, pStr, -1, pWideBuffer, bufLen);
	if(!numChars)
	{
		throw Win32Exception(::GetLastError());
	}

#ifdef QC_WCHAR
	
	return pWideBuffer;

#else

	String strRet;
	for(int i=0; i<numChars-1; ++i)
	{
		Character x(pWideBuffer[i]);
		x.appendToString(strRet);
	}
	return strRet;

#endif
}

//==============================================================================
// StringUtils::ToMBCSCodePage
//
/**
   Converts a @QuickCPP String into a multi-byte character string
   for the given @c codepage.
*/
//==============================================================================
ByteString StringUtils::ToMBCSCodePage(const String& str, int codepage)
{
	//
	// Steps required:
	// 1. Convert our string into a wchar_t array (if required)
	// 2. call WideCharToMultiByte on the wchar_t array
	//

	if(str.empty())
		return ByteString();

#ifdef QC_WCHAR

	const wchar_t* pWideChar = str.data();
	size_t bufLen = (4 * str.length())+1;

#else

	WCharAutoPtr apWideChar(ToWideChar(str));
	wchar_t* pWideChar = apWideChar.get();
	size_t bufLen = (2 * str.length())+1;

#endif

	ArrayAutoPtr<char> apCharBuffer(new char[bufLen]);
	int numChars = ::WideCharToMultiByte(codepage, 0, pWideChar, -1, apCharBuffer.get(), bufLen, NULL, NULL);
	if(!numChars)
	{
		throw Win32Exception(::GetLastError());
	}
	return apCharBuffer.get();
}

#endif //WIN32

#ifndef QC_WCHAR
//==============================================================================
// StringUtils::WCharAutoPtr 
//
/**
   Converts a @QuickCPP String, consisting of Unicode characters encoded
   as a sequence of ::CharType characters, into a null-terminated array
   of @c wchar_t characters.

   This function is not present in the @wchar_t version of the library.
*/
//==============================================================================
StringUtils::WCharAutoPtr StringUtils::ToWideChar(const String& str)
{
	//
	// Allocate a sufficiently large buffer.  The wide character
	// string cannot be longer than the encoded string
	//
	const size_t bufLen = str.length()+1;

	WCharAutoPtr apWideBuffer(new wchar_t[bufLen]);
	wchar_t* pNext = apWideBuffer.get();
	StringIterator end = str.data()+str.length();
	
	for(StringIterator iter = str.data(); iter != end; ++iter, ++pNext)
	{
		*pNext = wchar_t((*iter).toUnicode());
	}
	*pNext=0;
	QC_DBG_ASSERT(pNext < apWideBuffer.get() + bufLen);
	return apWideBuffer;
}
#endif // !QC_WCHAR

//==============================================================================
// StringUtils::ToHexDisplayString
//
/**
   Creates and returns a String containing each Unicode character from @c str in
   hexadecimal notation.

   @param str a String containing Unicode characters encoded according to the
   internal @QuickCPP convention.

   @returns a hexadecimal representation of the Unicode characters within @c str
*/
//==============================================================================
String StringUtils::ToHexString(const String& str)
{
	StringIterator i(str.data());
	StringIterator end(str.data()+str.size());
	String ret;

	while(i != end)
	{
		ret += QC_T("\\x");
		ret += FromLatin1(Format("%04X", (unsigned int)(*i++).toUnicode()));
	}
	return ret;
}

#include <xfunctional>
using namespace std;

String StringUtils::leftTrim(String ss)
{
	String::iterator p=find_if(ss.begin(), ss.end(), not1(ptr_fun(isspace)));
	ss.erase(ss.begin(),p);     
	
	return  ss;
}

String StringUtils::rightTrim(String ss)
{
	String::reverse_iterator p=find_if(ss.rbegin(), ss.rend(), not1(ptr_fun(isspace)));
	ss.erase(p.base(),ss.end());
	
	return ss;
}

String StringUtils::trim(String st)
{
	leftTrim(rightTrim(st));
	
	return st;     
}

bool StringUtils::startsWith(const String str, const String starts)
{
	return (str.compare(0, starts.length(), starts) == 0);
}

bool StringUtils::endsWith(const String str, const String ends)
{
	int elen = ends.length();
	return (str.compare(str.length() - elen, elen, ends)  == 0);
}

QC_BASE_NAMESPACE_END

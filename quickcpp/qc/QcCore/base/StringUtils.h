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

#ifndef QC_BASE_StringUtils_h
#define QC_BASE_StringUtils_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include "ArrayAutoPtr.h"
#include "String.h"

#if defined(WIN32)
	#include "winincl.h"
#endif //WIN32

#include <functional>

QC_BASE_NAMESPACE_BEGIN

class QC_BASE_PKG StringUtils
{
public:
	enum eStripType {leading, trailing, both};
	
	static int CompareNoCase(const String& lhs, const String& rhs);
	static int CompareNoCase(const char* pszLHS, const char* pszRHS);
	static bool LessNoCase(const String& lhs, const String rhs);

	static bool startsWith(const String str, const String starts);
	static bool endsWith(const String str, const String ends);

	static ByteString ToAscii(const String& str);
	static ByteString ToLatin1(const String& str);
	static ByteString ToUTF8(const String& str);

	static String ToUpper(const String& str);
	static String ToLower(const String& str);
	static String ToHexString(const String& str);

	static String FromNativeMBCS(const char* pStr);
	static ByteString ToNativeMBCS(const String& str);

	static String FromConsoleMBCS(const char* pStr);
	static ByteString ToConsoleMBCS(const String& str);

	static String FromLatin1(const char* pStr);
	static String FromLatin1(const char* pStr, size_t len);
	static String FromLatin1(const ByteString& str);
	static String FromUTF8(const ByteString& str);

	static bool ContainsMultiCharSequence(const String& str);
	static bool ReplaceAll(String& in, CharType search, const String& replacement);
	static bool ReplaceAll(String& in, const String& search, const String& replacement);
	static String StripWhiteSpace(const String& in, eStripType type);
	static String NormalizeWhiteSpace(const String& in);
	static bool IsHexString(const ByteString& in);
	
	static ByteString Format(const char* format, ...);

	static String leftTrim(String ss);
	static String rightTrim(String ss);
	static String trim(String ss);

#ifndef QC_UNICODE
	typedef ArrayAutoPtr< wchar_t > WCharAutoPtr;
	static WCharAutoPtr ToWideChar(const String& str);
#endif //QC_UNICODE

#if defined(WIN32)
	typedef ArrayAutoPtr< TCHAR > TCharAutoPtr;
	static TCharAutoPtr ToWin32String(const String& str);
	static String FromWin32String(LPCTSTR lpStr);
	static String FromMBCSCodePage(const char* pStr, int codepage);
	static ByteString ToMBCSCodePage(const String& str, int codepage);
#endif //WIN32

private:
	StringUtils(); // not implemented
};

QC_BASE_NAMESPACE_END

#endif //QC_BASE_StringUtils_h


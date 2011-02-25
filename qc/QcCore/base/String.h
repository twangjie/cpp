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

#ifndef QC_BASE_String_h
#define QC_BASE_String_h

#include "defs.h"

#include <string>

#ifndef QC_DOCUMENTATION_ONLY

// The QC_STRING_TYPE Macro is used to
// set the type of the String type used
// throughout QuickCPP.  It can either be set
// explicitly, or allowed to default to the
// C++ standard string types.

#ifndef QC_STRING_TYPE
	#ifdef QC_UNICODE
		#ifdef QC_NO_WSTRING_TYPEDEF
			#define QC_STRING_TYPE std::basic_string<wchar_t>
		#else
			#define QC_STRING_TYPE std::wstring
		#endif
	#else // !QC_UNICODE
		#define QC_STRING_TYPE std::string
	#endif // QC_UNICODE
#endif // QC_STRING_TYPE

#ifndef QC_BYTE_STRING_TYPE
	#define QC_BYTE_STRING_TYPE std::string
#endif // QC_BYTE_STRING_TYPE


#endif //QC_DOCUMENTATION_ONLY

//
// namespace celbase follows:
//
QC_BASE_NAMESPACE_BEGIN

//==============================================================================
//  typedef: String
/**
	This typedef determines the String type used by @QuickCPP.

    The type is set equal to the value of the pre-processor symbol @c QC_STRING_TYPE
	which is normally set to std::string (or std::wstring if the @c QC_UNICODE
	pre-processor symbol is defined).

    The library requires that the String type supports a set of operations equivalent
	to those provided by the standard C++ string @c std::basic_string<T>.
*/
//==============================================================================
typedef QC_STRING_TYPE String;

//==============================================================================
//  typedef: ByteString
/**
	This typedef determines the ByteString type used by @QuickCPP.

    The type is set equal to the value of the pre-processor symbol 
	@c QC_BYTE_STRING_TYPE	which is normally set to std::string.

    The library requires that the ByteString type supports a set of operations equivalent
	to those provided by the standard C++ string @c std::basic_string<char>.
*/
//==============================================================================
typedef QC_BYTE_STRING_TYPE ByteString;

QC_BASE_NAMESPACE_END

#endif //QC_BASE_String_h

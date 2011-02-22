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
	@namespace qc

	This is the top-level @QuickCPP namespace.  It contains classes that deal with:-
	- object lifetime management and garbage collection
	- multi-threading and synchronization
	- Unicode string encoding, formatting and conversion
	- message text retrieval and internationalization
	- tracing (event logging)

  	The top-level @c qc namespace contains classes that support this general-purpose
	functionality in addition to a number of subordinate namespaces which 
	house classes providing more specialized behaviour.
*/
//==============================================================================

#ifndef QC_BASE_DEFS_h
#define QC_BASE_DEFS_h

#include "gendefs.h"
#include "threaddefs.h"

#include <string>

#define QC_BASE_PKG QC_QUICKCPP_PKG
#define QC_BASE_INCLUDE_INLINES QC_QUICKCPP_INCLUDE_INLINES
#define QC_BASE_NAMESPACE_BEGIN QC_NAMESPACE_BEGIN
#define QC_BASE_NAMESPACE_END   QC_NAMESPACE_END

//
// Under MSVC, automatically link to requirted library
//
#if defined(_MSC_VER) && !defined(QC_IOS_EXPORT)
#include "libname.h"
#pragma comment(lib, QC_LIBNAME(QcCore))
#endif

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
//  typedef: CharType
/**
	Determines the character type used by @QuickCPP.

    The type is set equal to the value of the pre-processor symbol @c QC_CHAR_TYPE
	which is set equal to @c char unless the @c QC_WCHAR pre-processor
	symbol is specified, in which case it is set to @c wchar_t.

    As @c char is normally only 8-bits wide and @c wchar_t is only 16-bits wide
	on some platforms, it is not always possible to store a Unicode character
	into one ::CharType character.  In this case @QuickCPP represents Unicode
	characters using a multi-character encoding.
	
	If ::CharType is @c char then the internal encoding used is UTF-8.  
	If ::CharType is a 16-bit @c wchar_t then UTF-16 is used (not UCS-2).
	If ::CharType is a 32-bit value then no encoding is required as characters are
	UCS4 already.
*/
//==============================================================================
typedef QC_CHAR_TYPE  CharType;

//==============================================================================
//  typedef: UCharType
/**
    Unsigned version of ::CharType. 

    As ::CharType is supposed to represent character values, ideally it should
	be an unsigned value.  This is the case on some platforms but not on others.
	Functions that must use an unsigned value use ::UCharType in place of ::CharType.

	On platforms where ::CharType is already unsigned, ::CharType and ::UCharType 
	represent the same type.
*/
//==============================================================================
typedef QC_UCHAR_TYPE UCharType;

//==============================================================================
//  typedef: Byte
/**
	Represents a byte which is an unsigned 8-bit value with a range of 0-255.
	
	Bytes are used as the value type when reading or writing external media
	or communicating with other systems.  Bytes are typically converted to and
	from characters using some form of encoding.
*/
//==============================================================================
typedef unsigned char Byte;

//==============================================================================
//  typedef: UCS4Char
/**
	Represents an integer type that is capable of holding the code-points for
	all Unicode characters in the range U+0000 to U+10FFFF.
	
    This range requires an integer at least 21 bits wide, so @c UCS4Char is normally
	implemented using a 32-bit value.

    On some platforms (such as Linux) @c wchar_t is a 32-bit integer, so @c 
	UCS4Char and @c wchar_t are the same.  However, on other platforms (such
	as Microsoft Windows) @c wchar_t is a 16-bit integer and is therefore 
	incapable of representing the required Unicode range.
*/
//==============================================================================
typedef unsigned long UCS4Char;

//==============================================================================
//  typedef: UShort
/**
	Convenient type used as an unsigned short integer.
*/
//==============================================================================
typedef unsigned short UShort;

//==============================================================================
//  typedef: IntType
/**
	Type used as the return type in operations that need to return either
	a ::CharType or -1 which might not be a legal ::CharType value.
*/
//==============================================================================
typedef QC_INT_TYPE IntType;

QC_BASE_NAMESPACE_END

#endif //QC_BASE_DEFS_h


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
	@class qc::NumUtils
	
	@brief Class module containing functions to convert numbers into Strings
	       and vice versa.

	@sa StringUtils
*/
//==============================================================================

#include "NumUtils.h"
#include "Exception.h"
#include "StringUtils.h"

#include <stdio.h>

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// NumUtils::ToString
//
/**
   Converts the long value @c x into a String.
*/
//==============================================================================
String NumUtils::ToString(long x)
{
	return StringUtils::FromLatin1(StringUtils::Format("%ld", x));
}

//==============================================================================
// NumUtils::ToString
//
/**
   Converts the unsigned long value @c x into a String.
*/
//==============================================================================
String NumUtils::ToString(unsigned long x)
{
	return StringUtils::FromLatin1(StringUtils::Format("%lu", x));
}

//==============================================================================
// NumUtils::ToString
//
/**
   Converts the integer value @c x into a String.
*/
//==============================================================================
String NumUtils::ToString(int x)
{
	return StringUtils::FromLatin1(StringUtils::Format("%d", x));
}

//==============================================================================
// NumUtils::ToString
//
/**
   Converts the unsigned integer value @c x into a String.
*/
//==============================================================================
String NumUtils::ToString(unsigned int x)
{
	return StringUtils::FromLatin1(StringUtils::Format("%u", x));
}

//==============================================================================
// NumUtils::ToString
//
/**
   Converts the time_t value @c x into a String.
*/
//==============================================================================
String NumUtils::ToString(time_t x)
{
	return StringUtils::FromLatin1(StringUtils::Format("%u", x));
}

//==============================================================================
// NumUtils::ToString
//
/**
   Converts the double-precision floating-point value @c x into a String.
*/
//==============================================================================
String NumUtils::ToString(double d)
{
	return StringUtils::FromLatin1(StringUtils::Format("%f", d));
}

//==============================================================================
// NumUtils::ToInt
//
/**
   Converts a String into an integer value.

   @param str the String to convert
   @param base the number base to use for the conversion (e.g. 10 for decimal,
          16 for hexadecimal)
   @returns the String converted into an integer.  @c 0 is returned if the
            String cannot be converted.
*/
//==============================================================================
int NumUtils::ToInt(const String& str, int base)
{
	try
	{
		const ByteString ascii = StringUtils::ToAscii(str);
		return (int)strtol(ascii.c_str(), 0, base);
	}
	catch(Exception& /*e*/)
	{
	}
	return 0;
}

//=============================================================================
// NumUtils::ToLong
//
/**
   Converts a String into a long integer value.

   @param str the String to convert
   @param base the number base to use for the conversion (e.g. 10 for decimal,
          16 for hexadecimal)
   @returns the String converted into a long integer.  @c 0 is returned if the
            String cannot be converted.
*/
//=============================================================================
long NumUtils::ToLong(const String& str, int base)
{
	try
	{
		const ByteString ascii = StringUtils::ToAscii(str);
		return strtol(ascii.c_str(), 0, base);
	}
	catch(Exception& /*e*/)
	{
	}
	return 0L;
}

//=============================================================================
// NumUtils::ToDouble
//
/**
   Converts a String into a double-precision floating-point value.

   @param str the String to convert
   @returns the String converted into an double.  @c 0.0 is returned if the
            String cannot be converted.
*/
//=============================================================================
double NumUtils::ToDouble(const String& str)
{
	try
	{
		const ByteString ascii = StringUtils::ToAscii(str);
		return ::strtod(ascii.c_str(), 0);
	}
	catch(Exception& /*e*/)
	{
	}
	return 0.0;
}

QC_BASE_NAMESPACE_END

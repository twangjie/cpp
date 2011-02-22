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
// Class MessageFormatter
//
// MessageFormatter is a replacement for sprintf that enables messages to be
// created using positional replacement.
//
// A typical use would be:
//
// cout << MessageFormatter::Format("Illegal value {0} expecting {1}",
//                                  sValue, sExpected);
//
// The Format() method is overloaded to allow type-safe checking of the
// input parameters (which are always String&s).
//==============================================================================
#ifndef QC_UTIL_MessageFormatter_h
#define QC_UTIL_MessageFormatter_h

#ifndef QC_UTIL_DEFS_h
#include "defs.h"
#endif //QC_UTIL_DEFS_h

QC_UTIL_NAMESPACE_BEGIN

class QC_UTIL_PKG MessageFormatter
{
public:
	
	static String Format(const String& spec, const String& arg1);
	static String Format(const String& spec, const String& arg1, const String& arg2);
	static String Format(const String& spec, const String& arg1, const String& arg2, const String& arg3);
	static String Format(const String& spec, const String& arg1, const String& arg2, const String& arg3, const String& arg4);
	static String Format(const String& spec, const String& arg1, const String& arg2, const String& arg3, const String& arg4, const String& arg5);
	static String Format(const String& spec, const String& arg1, const String& arg2, const String& arg3, const String& arg4, const String& arg5, const String& arg6);

protected:
	static String FormatImpl(const String& spec, size_t count, ...);

private:
	MessageFormatter(); // not implemented
};

QC_UTIL_NAMESPACE_END

#endif //QC_UTIL_MessageFormatter_h


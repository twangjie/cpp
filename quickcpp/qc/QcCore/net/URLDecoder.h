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
// Class: URLDecoder
// 
// Overview
// --------
// based on the java.net.URLDecoder class.  Provides the capability to encode
// a unicode URL into its www escaped form
//
// see http://java.sun.com/products/jdk/1.0.2/api/java.net.URLDecoder.html
// for documentation.
//
// Note: the JDK URLDecoder has a deficiency that this class does NOT copy,
// that is the inability to deal with non ASCII characters.
//
// Also, the function name is changed from encode to Encode to satisfy the
// QC static function naming convention.
//=============================================================================

#ifndef QC_NET_URLDecoder_h
#define QC_NET_URLDecoder_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

QC_NET_NAMESPACE_BEGIN

class QC_NET_PKG URLDecoder
{
public:
	// Translates a string into x-www-form-urlencoded format.
	static String Decode(const String& s);
	
	// Decode the uri string using the up-to-date algorithm
	static String RawDecode(const String& s);
};

QC_NET_NAMESPACE_END

#endif //QC_NET_URLDecoder_h


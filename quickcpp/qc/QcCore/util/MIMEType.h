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
// Class MIMEType
//
// This class is used to parse and represent a MIME Type as defined in 
// RFC 2045 and 2046.
//
//==============================================================================

#ifndef QC_UTIL_MIMEType_h
#define QC_UTIL_MIMEType_h

#ifndef QC_UTIL_DEFS_h
#include "defs.h"
#endif //QC_UTIL_DEFS_h

#include <map>

QC_UTIL_NAMESPACE_BEGIN

class QC_UTIL_PKG MIMEType
{
public:
	
	MIMEType(const String& rawdata);

	String getParameter(const String& name) const;
	const String& getType() const;
	const String& getSubType() const;

private:
	void addParameter(const String& name, const String& value);
	bool parseContentTypeHeader(const String& contentType);

private:
	typedef std::map<String, String, std::less<String> > ParamMap;
	ParamMap m_paramMap;
	String m_type;
	String m_subType;
};

QC_UTIL_NAMESPACE_END

#endif //QC_UTIL_MIMEType_h


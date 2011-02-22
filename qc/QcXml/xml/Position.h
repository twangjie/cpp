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
// Class Position
//
// Used to report the position within a parsed entity
// 
//==============================================================================


#ifndef QC_XML_Position_h
#define QC_XML_Position_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

#include "QcCore/net/URL.h"

QC_XML_NAMESPACE_BEGIN

using net::URL;

class QC_XML_PKG Position
{
public:

	virtual size_t getColumnNumber() const = 0;
	virtual size_t getLineNumber() const = 0;
	virtual size_t getOffset() const = 0;
	virtual String getPublicId() const = 0;
	virtual String getSystemId() const = 0;
	virtual String getResolvedSystemId() const = 0;
};

QC_XML_NAMESPACE_END

#endif //QC_XML_Position_h


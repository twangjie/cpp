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

#ifndef QC_XML_XMLParsingException_h
#define QC_XML_XMLParsingException_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

#include "QcCore/base/Exception.h"
#include "StreamPosition.h"

QC_XML_NAMESPACE_BEGIN

class QC_XML_PKG XMLParsingException : public Exception
{
public:
	XMLParsingException(const String& systemId,
	                    const StreamPosition& position,
						int errorLevel,
						long reserved,
						const String& message);

	const String& getSystemId() const;
	const StreamPosition& getStreamPosition() const;
	int getErrorLevel() const;
	virtual String getExceptionType() const {return QC_T("XMLParsingException");}

private:
	String         m_systemId;
	StreamPosition m_position;
	int            m_errorLevel;
};


QC_XML_NAMESPACE_END

#endif //QC_XML_XMLParsingException_h


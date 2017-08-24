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

#ifndef QC_SAX_SAXParseException_h
#define QC_SAX_SAXParseException_h

#ifndef QC_SAX_defs_h
#include "defs.h"
#endif //QC_SAX_defs_h

#include "SAXException.h"

QC_SAX_NAMESPACE_BEGIN

class Locator;

class QC_SAX_PKG SAXParseException : public SAXException
{
public:
	SAXParseException(const String& message, const Locator& locator);
	
	SAXParseException(const String& message,
	                  const String& publicId,
	                  const String& systemId,
	                  long lineNumber,
	                  long columnNumber);

	virtual long getColumnNumber() const;
	virtual long getLineNumber() const;
	virtual String getPublicId() const;
	virtual String getSystemId() const;

	virtual String getExceptionType() const {return QC_T("SAXParseException");}

private:
	String m_publicId;
	String m_systemId;
	long m_columnNumber;
	long m_lineNumber;
};


QC_SAX_NAMESPACE_END

#endif //QC_SAX_SAXParseException_h


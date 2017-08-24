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
// Class: SAXException
// 
/**
	@class qc::sax::SAXException
	
	@brief Encapsulate a general SAX error or warning.
	       
	<p>This class can contain basic error or warning information from
	either the XML parser or the application: a parser writer or
	application writer can subclass it to provide additional
	functionality.  SAX handlers may throw this exception or
	any exception @a derived from it.</p>
	
	<p>If the parser or application needs to include information about a
	specific location in an XML document, it should use the
	SAXParseException @a derived class.</p>
*/
//==============================================================================

#ifndef QC_SAX_SAXException_h
#define QC_SAX_SAXException_h

#ifndef QC_SAX_defs_h
#include "defs.h"
#endif //QC_SAX_defs_h

#include "QcCore/base/Exception.h"

QC_SAX_NAMESPACE_BEGIN

class QC_SAX_PKG SAXException : public Exception
{
public:
	/** Constructs a SAXException with a detail message
	* @param message the detail message.
	*/
	SAXException(const String& message) : 
		Exception(message)
	{}
	
	virtual String getExceptionType() const {return QC_T("SAXException");}
};


QC_SAX_NAMESPACE_END

#endif //QC_SAX_SAXException_h


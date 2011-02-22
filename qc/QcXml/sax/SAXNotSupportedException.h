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
// Class: SAXNotSupportedException
// 
/**
	@class qc::sax::SAXNotSupportedException
	
	@brief Exception class for an unsupported operation.
	       
	<p>An XMLReader will throw this exception when it recognizes a
	feature, but cannot perform the requested operation.  
	Other SAX2 applications and extensions may use this class 
	for similar purposes.</p>
*/
//==============================================================================

#ifndef QC_SAX_SAXNotSupportedException_h
#define QC_SAX_SAXNotSupportedException_h

#ifndef QC_SAX_defs_h
#include "defs.h"
#endif //QC_SAX_defs_h

#include "SAXException.h"

QC_SAX_NAMESPACE_BEGIN

class QC_SAX_PKG SAXNotSupportedException : public SAXException
{
public:
	/** Constructs a SAXNotSupportedException with a detail message
	* @param message the detail message.
	*/
	SAXNotSupportedException(const String& message) : 
		SAXException(message)
	{}
	
	virtual String getExceptionType() const {return QC_T("SAXNotSupportedException");}
};


QC_SAX_NAMESPACE_END

#endif //QC_SAX_SAXNotSupportedException_h


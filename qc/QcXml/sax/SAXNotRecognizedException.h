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
// Class: SAXNotRecognizedException
// 
/**
	@class qc::sax::SAXNotRecognizedException
	
	@brief Exception class for an unrecognized identifier.

	<p>An XMLReader will throw this exception when it finds an
	unrecognized feature identifier; SAX applications and
	extensions may use this class for other, similar purposes.</p>
*/
//==============================================================================

#ifndef QC_SAX_SAXNotRecognizedException_h
#define QC_SAX_SAXNotRecognizedException_h

#ifndef QC_SAX_defs_h
#include "defs.h"
#endif //QC_SAX_defs_h

#include "SAXException.h"

QC_SAX_NAMESPACE_BEGIN

class QC_SAX_PKG SAXNotRecognizedException : public SAXException
{
public:
	/** Constructs a SAXNotRecognizedException with a detail message
	* @param message the detail message.
	*/
	SAXNotRecognizedException(const String& message) : 
		SAXException(message)
	{}
	
	virtual String getExceptionType() const {return QC_T("SAXNotRecognizedException");}
};


QC_SAX_NAMESPACE_END

#endif //QC_SAX_SAXNotRecognizedException_h


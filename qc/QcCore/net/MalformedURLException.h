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
// Class: MalformedURLException
// 
/**
	@class qc::net::MalformedURLException
	
	@brief Thrown when an error occurs parsing a URL specification string.
	       This usually indicates that the specification did not contain
		   a recognized protocol (e.g. http).
*/
//==============================================================================

#ifndef QC_NET_MalformedURLException_h
#define QC_NET_MalformedURLException_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "QcCore/base/Exception.h"

QC_NET_NAMESPACE_BEGIN

class QC_NET_PKG MalformedURLException : public Exception
{
public:
	/** Constructs a MalformedURLException with a detail message
	* @param message the detail message.
	*/
	MalformedURLException(const String& message) : 
		Exception(message)
	{}
	
	virtual String getExceptionType() const {return QC_T("MalformedURLException");}
};

QC_NET_NAMESPACE_END

#endif //QC_NET_MalformedURLException_h


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
// Class: IOException
// 
/**
	@class qc::io::IOException
	
	@brief Base class for general IO exceptions.
	       
	Objects of this class or derived classes are thrown when an error occurs
	during an I/O operation.
*/
//==============================================================================

#ifndef QC_IO_IOException_h
#define QC_IO_IOException_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "QcCore/base/Exception.h"

QC_IO_NAMESPACE_BEGIN

class QC_IO_PKG IOException : public Exception
{
public:
	/** Constructs an IOException without a detail message */
	IOException() {}

	/** Constructs an IOException with a detail message
	* @param message the detail message.
	*/
	IOException(const String& message) : 
		Exception(message) {}

	virtual String getExceptionType() const {return QC_T("IOException");}
};


QC_IO_NAMESPACE_END

#endif //QC_IO_IOException_h


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
// Class: AtomicReadException
// 
/**
	@class qc::io::AtomicReadException
	
	@brief Thrown when a readAtomic() operation is performed on a Reader,
	       and a prior read() operation left the stream located part-way through
		   a multi-character sequence.
*/
//==============================================================================

#ifndef QC_IO_AtomicReadException_h
#define QC_IO_AtomicReadException_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "IOException.h"

QC_IO_NAMESPACE_BEGIN

class QC_IO_PKG AtomicReadException : public IOException
{
public:

	/** Constructs an AtomicReadException with a detail message
	* @param message the detail message.
	*/
	AtomicReadException(const String& message) : 
		IOException(message)
	{}

	virtual String getExceptionType() const {return QC_T("AtomicReadException");}
};


QC_IO_NAMESPACE_END

#endif //QC_IO_AtomicReadException_h


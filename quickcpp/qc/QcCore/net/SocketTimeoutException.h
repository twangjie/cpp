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
// Class: SocketTimeoutException
// 
/**
	@class qc::net::SocketTimeoutException
	
	@brief Thrown when a time-out occurs on a socket performing
	       a read(), accept() or connect() call.
*/
//==============================================================================

#ifndef QC_NET_SocketTimeoutException_h
#define QC_NET_SocketTimeoutException_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "QcCore/io/InterruptedIOException.h"

QC_NET_NAMESPACE_BEGIN

using io::InterruptedIOException;

class QC_NET_PKG SocketTimeoutException : public InterruptedIOException
{
public:
	/** Constructs a SocketTimeoutException with a detail message
	* @param message the detail message.
	*/
	SocketTimeoutException(const String& message) : 
		InterruptedIOException(message)
	{}
	
	virtual String getExceptionType() const {return QC_T("SocketTimeoutException");}
};


QC_NET_NAMESPACE_END

#endif //QC_NET_SocketTimeoutException_h


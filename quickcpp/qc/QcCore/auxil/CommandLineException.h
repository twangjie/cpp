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

#ifndef QC_AUXIL_CommandLineException_h
#define QC_AUXIL_CommandLineException_h

#include "defs.h"

#include "QcCore/base/Exception.h"

QC_AUXIL_NAMESPACE_BEGIN

class QC_AUXIL_PKG CommandLineException : public Exception
{
public:
	CommandLineException(const String& option, const String& message);
	
	virtual String getExceptionType() const;
	virtual String getOption() const;
	
private:
	String m_option;
};

QC_AUXIL_NAMESPACE_END

#endif  //QC_AUXIL_CommandLineException_h


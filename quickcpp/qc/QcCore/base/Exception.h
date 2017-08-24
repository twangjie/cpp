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

#ifndef QC_BASE_Exception_h
#define QC_BASE_Exception_h

#include "defs.h"
#include "String.h"

#include <string>
#include <exception>

QC_BASE_NAMESPACE_BEGIN

class QC_BASE_PKG Exception
{
public:
	Exception();
	Exception(const String& message);
	
	virtual ~Exception();

	virtual String getMessage() const;
	virtual String toString() const;
	virtual String getExceptionType() const;
	virtual String getLocalizedDescription() const;
	String getDescription() const;

protected:
	void setMessage(const String& message);

private:
	String m_message;
};

QC_BASE_NAMESPACE_END

#endif //QC_BASE_Exception_h

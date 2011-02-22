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

#include "CommandLineException.h"

QC_AUXIL_NAMESPACE_BEGIN

CommandLineException::CommandLineException(const String& option, const String& message) : 
	Exception(message),
	m_option(option)
{
}

String CommandLineException::getExceptionType() const
{
	return QC_T("CommandLineException");
}

String CommandLineException::getOption() const
{
	return m_option;
}

QC_AUXIL_NAMESPACE_END


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

#include "ContextString.h"
#include "QcCore/base/StringUtils.h"

#include <algorithm>

QC_XML_NAMESPACE_BEGIN

ContextString::ContextString() :
	m_startCol(0),
	m_lineNo(0)
{
}

ContextString::ContextString(const String& value, size_t startCol, size_t lineNo) :
	m_value(value),
	m_startCol(startCol),
	m_lineNo(lineNo)
{
}

size_t ContextString::getStartColumn() const
{
	return m_startCol;
}

size_t ContextString::getLine() const
{
	return m_lineNo;
}

const String& ContextString::getValue() const
{
	return m_value;
}

//==============================================================================
// ContextString::getNormalizedValue
//
// Normalize the string by replacing tab characters with spaces.
//==============================================================================
String ContextString::getNormalizedValue() const
{
	String ret = m_value;
	StringUtils::ReplaceAll(ret, '\t', QC_T(" "));
	return ret;
}

String ContextString::getPointerString(size_t pointerCol) const
{
	size_t offset = pointerCol - (m_startCol-1);
	if(offset > 1)
	{
		String strRet(offset-1, '-');
		strRet += QC_T("^");
		return strRet;
	}
	else
		return QC_T("^");
}

QC_XML_NAMESPACE_END


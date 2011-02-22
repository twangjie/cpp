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
//
// Class: ContextString
// 
// Overview
// --------
// A class to encapsulate a small part of an xml file - usually containing
// the location of an error condition.
//
// This class can be used to aid in the writing of user interfaces where
// error messages need to be enhanced with contextual info.
//
//==============================================================================

#ifndef QC_XML_ContextString_h
#define QC_XML_ContextString_h

#include "defs.h"

QC_XML_NAMESPACE_BEGIN

class QC_XML_PKG ContextString
{
public:

	ContextString(const String& value, size_t startCol, size_t lineNo);
	ContextString();

	size_t getStartColumn() const;
	size_t getLine() const;
	const String& getValue() const;
	String getNormalizedValue() const;
	String getPointerString(size_t pointerCol) const;

private:
	String m_value;
	size_t m_startCol;
	size_t m_lineNo;
};

QC_XML_NAMESPACE_END

#endif //QC_XML_ContextString_h

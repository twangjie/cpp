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
// StreamPosition
//
// Used to record the relative human-understandable position within an input
// stream.  i.e. We record the position in terms of line and column number
// starting at position (1,1)
//
// This class also takes some of the workload from the scanner by incrementing
// position given a [sequence of] characters.
// 
//==============================================================================

#ifndef QC_XML_StreamPosition_h
#define QC_XML_StreamPosition_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h 

#include "QcCore/base/debug.h"

QC_XML_NAMESPACE_BEGIN

class QC_XML_PKG StreamPosition
{
public:
	StreamPosition();

	void incrementByChar(const Character& ch); 
	void incrementByString(const String& str); 
	void decrementColumns(size_t colCount); 

	bool operator==(const StreamPosition& rhs) const;
	bool operator!=(const StreamPosition& rhs) const;

	size_t getLineNo() const;
	size_t getColNo() const;
	size_t getOffset() const;

private:
	size_t m_lineNo;
	size_t m_colNo;
	size_t m_offset;
};

inline StreamPosition::StreamPosition() :
m_lineNo(1),
	m_colNo(1),
	m_offset(0)
{
}

inline size_t StreamPosition::getLineNo() const
{
	return m_lineNo;
}

inline size_t StreamPosition::getColNo() const
{
	return m_colNo;
}

inline size_t StreamPosition::getOffset() const
{
	return m_offset;
}

inline void StreamPosition::incrementByChar(const Character& ch)
{
	m_offset++;
	if(ch == '\n')
	{
		m_lineNo++;
		m_colNo=1;
	}
	else
	{
		m_colNo++;
	}
}

inline void StreamPosition::incrementByString(const String& str)
{
	for(String::const_iterator it=str.begin(); it!=str.end(); ++it)
	{
		// replicated StreamPosition::incrementByChar(CharType ch)
		// to avoid function call overhead!
		m_offset++;
		if(*it == '\n')
		{
			m_lineNo++;
			m_colNo=1;
		}
		else
		{
			m_colNo++;
		}
	}
}

inline bool StreamPosition::operator==(const StreamPosition& rhs) const
{
	return (m_offset == rhs.m_offset);
}

inline bool StreamPosition::operator!=(const StreamPosition& rhs) const
{
	return !(*this==rhs);
}

inline void StreamPosition::decrementColumns(size_t colCount)
{
	QC_DBG_ASSERT(colCount < m_colNo);
	QC_DBG_ASSERT(colCount <= m_offset);
	m_colNo -= colCount;
	m_offset -= colCount;
}

QC_XML_NAMESPACE_END

#endif //QC_XML_StreamPosition_h


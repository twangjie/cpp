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
// ScannerPosition
//
// Used to record the position within a scanner's input stream.  An instance
// of this class is passed to the scanner for each scanning operation.
// By recording position outside of the scanner class itself, the parsing
// application is given the power to back-track to any position in the input
// stream.
//
// The ScannerPosition owns a pointer to the "next" position.  This caters for
// entity expansion by inserting lexical tokens into the current position of
// the input stream.
// 
//==============================================================================

#ifndef QC_XML_ScannerPosition_h
#define QC_XML_ScannerPosition_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

#include "Position.h"
#include "Buffer.h"
#include "StreamPosition.h"

QC_XML_NAMESPACE_BEGIN

class StreamPosition;
class Entity;

class QC_XML_PKG ScannerPosition : public Position
{
	friend class Scanner;

public:

	enum EOFAction {softEOF, skip, space};

	ScannerPosition() : m_pNext(NULL), m_eofAction(softEOF) {};
	virtual ~ScannerPosition();
	ScannerPosition(const ScannerPosition& rhs);
	ScannerPosition& operator=(const ScannerPosition& rhs);

	void setNextPosition(const ScannerPosition& next, EOFAction eofAction);
	const Entity& getEntity() const;
	const StreamPosition& getStreamPosition() const;
	bool isValid() const;

	//
	// Position methods
	//
	virtual size_t getColumnNumber() const;
	virtual size_t getLineNumber() const;
	virtual size_t getOffset() const;
	virtual String getPublicId() const;
	virtual String getSystemId() const;
	virtual String getResolvedSystemId() const;

private:

	ScannerPosition(Buffer* pBuffer);
	ScannerPosition(Buffer* pBuffer, const StreamPosition& streamPosition);

private:
	ScannerPosition* m_pNext;
	AutoPtr<Buffer> m_rpBuffer;
	size_t m_bufferOffset;
	StreamPosition m_streamPosition;
	EOFAction m_eofAction;
};

inline ScannerPosition::~ScannerPosition()
{
	delete m_pNext;
}

inline ScannerPosition::ScannerPosition(const ScannerPosition& rhs) :
	m_pNext(NULL)
{
	*this = rhs;
}

inline ScannerPosition& ScannerPosition::operator=(const ScannerPosition& rhs)
{
	if(this != &rhs)
	{
		m_streamPosition = rhs.m_streamPosition;
		m_bufferOffset = rhs.m_bufferOffset;
		m_rpBuffer = rhs.m_rpBuffer;
		m_eofAction = rhs.m_eofAction;

		ScannerPosition* pNewNext = NULL;
		if(rhs.m_pNext)
		{
			pNewNext = new ScannerPosition(*rhs.m_pNext);
		}
		delete m_pNext;
		m_pNext = pNewNext;
	}
	return *this;
}


QC_XML_NAMESPACE_END

#endif //QC_XML_ScannerPosition_h

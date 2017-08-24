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

#include "Buffer.h"
#include "Entity.h"
#include "QcCore/base/debug.h"

QC_XML_NAMESPACE_BEGIN

//
// For atomic reads, the mimumum read buffer size must be able to 
// hold the longest possible character sequence which is 6 bytes
//
static const size_t MinReadSize = 6;

Buffer::Buffer(size_t size, const Entity& entity) : 
	m_pData(new CharType[size]),
	m_used(0),
	m_bEOF(false),
	m_bFull(false),
	m_size(size),
	m_entity(entity)
{
	QC_DBG_ASSERT(size > 0);
	QC_DBG_ASSERT(m_pData!=0);
}

Buffer::~Buffer()
{
	delete [] m_pData;
}

void Buffer::read()
{
	if(m_bEOF)
	{
		return;
	}

	if(m_bFull)
	{
		if(m_rpNext.isNull())
		{
			m_rpNext = new Buffer(m_size, m_entity);
			m_rpNext->read();
		}
	}
	else
	{
		const size_t available = m_size-m_used;
		QC_DBG_ASSERT(available >= MinReadSize);
		const AutoPtr<Reader>& rpReader = m_entity.getReader();
		long count = rpReader->readAtomic(m_pData+m_used, available);
		
		if(count == Reader::EndOfFile)
		{
			m_bEOF = true;
		}
		else
		{
			QC_DBG_ASSERT(count <= (long)available && count > 0);
			m_used += count;
			m_bFull = (m_size-m_used) <= MinReadSize;
		}
	}
}

const Entity& Buffer::getEntity() const
{
	return m_entity;
}

QC_XML_NAMESPACE_END


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

#ifndef QC_XML_Buffer_h
#define QC_XML_Buffer_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

#ifndef QC_XML_StreamPosition_h
#include "StreamPosition.h"
#endif //QC_XML_StreamPosition_h

QC_XML_NAMESPACE_BEGIN

class Entity;

class QC_XML_PKG Buffer : public virtual ManagedObject
{
public:

	Buffer(size_t size, const Entity& entity);
	~Buffer();

	CharType* m_pData;
	size_t m_used;
	bool m_bEOF;
	bool m_bFull;
	AutoPtr<Buffer> m_rpNext;

	void read();
	const Entity& getEntity() const;

private: // not implemented
	Buffer(const Buffer& rhs);            // cannot be copied
	Buffer& operator=(const Buffer& rhs); // nor assigned

private:
	size_t m_size;
	const Entity& m_entity;
};

QC_XML_NAMESPACE_END

#endif //QC_XML_Buffer_h

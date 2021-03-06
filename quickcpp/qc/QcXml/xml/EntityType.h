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

#ifndef QC_XML_EntityType_h
#define QC_XML_EntityType_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

QC_XML_NAMESPACE_BEGIN

class QC_XML_PKG EntityType
{
public:

	enum Type {Document, DTD, General, Parameter};

	EntityType(Type eType);
	EntityType(const EntityType& rhs);
	EntityType& operator=(const EntityType& rhs);

	bool operator==(Type eType) const;
	bool operator!=(Type eType) const;
	bool operator==(const EntityType& rhs) const;
	bool operator!=(const EntityType& rhs) const;

private:
	Type m_eType;
};

QC_XML_NAMESPACE_END

#endif //QC_XML_EntityEventHandler_h

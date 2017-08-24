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

#ifndef QC_SAX_DTDHandler_h
#define QC_SAX_DTDHandler_h

#include "defs.h"

QC_SAX_NAMESPACE_BEGIN 

class QC_SAX_PKG DTDHandler : public virtual QCObject
{
public:

	virtual void notationDecl(const String& name, const String& publicId, const String& systemId) = 0;
	virtual void unparsedEntityDecl(const String& name, const String& publicId, const String& systemId, const String& notationName) = 0;
};

QC_SAX_NAMESPACE_END

#endif //QC_SAX_DTDHandler_h

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

#ifndef QC_XML_ErrorEventHandler_h
#define QC_XML_ErrorEventHandler_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

#include "EventHandler.h"

QC_XML_NAMESPACE_BEGIN

class Position;
class ContextString;

class QC_XML_PKG ErrorEventHandler : public EventHandler
{
public:
	
	enum ErrorLevel {Success=0, Warning=1, Error=2, Fatal=3};

	virtual void onError(int level,
	                     long /*reserved*/,
                         const String& message,
                         const Position& position,
						 const ContextString* pContextString) = 0;
};

QC_XML_NAMESPACE_END

#endif //QC_XML_ErrorEventHandler_h

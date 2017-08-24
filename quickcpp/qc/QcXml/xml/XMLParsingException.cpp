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

#include "XMLParsingException.h"

QC_XML_NAMESPACE_BEGIN

XMLParsingException::XMLParsingException(const String& systemId,
                                         const StreamPosition& position,
                                         int errorLevel,
                                         long /*reserved*/,
                                         const String& message) :
	Exception(message),
	m_systemId(systemId),
	m_position(position),
	m_errorLevel(errorLevel)
{
}

const String& XMLParsingException::getSystemId() const
{
	return m_systemId;
}

const StreamPosition& XMLParsingException::getStreamPosition() const
{
	return m_position;
}

int XMLParsingException::getErrorLevel() const
{
	return m_errorLevel;
}

QC_XML_NAMESPACE_END

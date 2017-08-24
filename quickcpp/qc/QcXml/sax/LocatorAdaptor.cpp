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

#include "LocatorAdaptor.h"

#include "QcCore/base/NullPointerException.h"
#include "QcXml/xml/Parser.h"
#include "QcXml/xml/Position.h"

QC_SAX_NAMESPACE_BEGIN

LocatorAdaptor::LocatorAdaptor(Parser* pParser, bool bResolveSystemIds) :
	m_rpParser(pParser),
	m_bResolveSystemIds(bResolveSystemIds)
{
	if(!pParser) throw NullPointerException();
}

//==============================================================================
// LocatorAdaptor::getColumnNumber
//
// Implementation of Locator::getColumnNumber()
//==============================================================================
long LocatorAdaptor::getColumnNumber() const
{
	const long col = m_rpParser->getCurrentPosition().getColumnNumber();
	return (col) ? col : -1;
}

//==============================================================================
// LocatorAdaptor::getLineNumber
//
// Implementation of Locator::getLineNumber()
//==============================================================================
long LocatorAdaptor::getLineNumber() const
{
	const long line = m_rpParser->getCurrentPosition().getLineNumber();
	return (line) ? line : -1;
}

//==============================================================================
// LocatorAdaptor::getPublicId
//
// Implementation of Locator::getPublicId()
//==============================================================================
String LocatorAdaptor::getPublicId() const
{
	return m_rpParser->getCurrentPosition().getPublicId();
}

//==============================================================================
// LocatorAdaptor::getSystemId
//
// Implementation of Locator::getSystemId()
//==============================================================================
String LocatorAdaptor::getSystemId() const
{
	return m_bResolveSystemIds
		? m_rpParser->getCurrentPosition().getResolvedSystemId()
		: m_rpParser->getCurrentPosition().getSystemId();
}

void LocatorAdaptor::setResolveSystemIds(bool bSet)
{
	m_bResolveSystemIds = bSet;
}

bool LocatorAdaptor::getResolveSystemIds() const
{
	return m_bResolveSystemIds;
}


QC_SAX_NAMESPACE_END

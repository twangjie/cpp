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
// Class: MessageSet
// 
// Overview
// --------
// An implementation of MessageFactory that loads messages from a
// file.
//
//==============================================================================

#include "MessageSet.h"

QC_AUXIL_NAMESPACE_BEGIN

MessageSet::MessageSet(const String& org, const String& app) :
	m_org(org),
	m_app(app)
{
}

void MessageSet::addMessage(size_t messageID, const String& message)
{
	m_messageMap[messageID] = message;
}

bool MessageSet::getMessageText(size_t messageID, String& ret) const
{
	MessageMap::const_iterator iter = m_messageMap.find(messageID);
	if(iter != m_messageMap.end())
	{
		ret = (*iter).second;
		return true;
	}
	else
	{
		return false;
	}
}

const String& MessageSet::getOrganizationName() const
{
	return m_org;
}

const String& MessageSet::getApplicationName() const
{
	return m_app;
}

QC_AUXIL_NAMESPACE_END

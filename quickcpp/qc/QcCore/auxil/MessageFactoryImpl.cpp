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

#include "MessageFactoryImpl.h"

QC_AUXIL_NAMESPACE_BEGIN

bool MessageFactoryImpl::getMessage(const String& org, const String& app,
                                    size_t messageID, String& ret)
{
	MessageSet* pFoundSet = 0;

	MessageSetList::iterator iter = m_messageSetList.begin();
	for(; iter != m_messageSetList.end(); ++iter)
	{
		MessageSet* pSet = (*iter).get();
		QC_DBG_ASSERT(pSet!=0);
		if(pSet->getOrganizationName() == org 
		&& pSet->getApplicationName() == app)
		{
			pFoundSet = pSet;
			break;
		}
	}
	
	if(!pFoundSet)
	{
		AutoPtr<MessageSet> rpSet = getMessageSet(org, app);
		if(rpSet)
		{
			pFoundSet = rpSet.get();
			m_messageSetList.push_back(rpSet);
		}
	}

	if(pFoundSet)
	{
		return pFoundSet->getMessageText(messageID, ret);
	}
	else
	{
		return false;
	}
}

AutoPtr<MessageSet> MessageFactoryImpl::getMessageSet(const String& /*org*/, const String& /*app*/)
{
	return 0;	
}

QC_AUXIL_NAMESPACE_END

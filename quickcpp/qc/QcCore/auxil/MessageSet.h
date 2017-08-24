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

#ifndef MessageSet_h
#define MessageSet_h

#include "defs.h"
#include "QcCore/base/String.h"

#include <map>

QC_AUXIL_NAMESPACE_BEGIN

class QC_AUXIL_PKG MessageSet : public virtual QCObject
{
public:
	MessageSet(const String& org, const String& app);
	
	void addMessage(size_t messageID, const String& message);
	
	bool getMessageText(size_t messageID, String& ret) const;
	
	const String& getOrganizationName() const;
	const String& getApplicationName() const;

private:
	String m_org;
	String m_app;
	typedef std::map<size_t, String, std::less<size_t> > MessageMap;
	MessageMap m_messageMap;
};

QC_AUXIL_NAMESPACE_END

#endif //MessageSet_h


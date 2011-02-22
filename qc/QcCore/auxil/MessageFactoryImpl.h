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
// Class: MessageFactoryImpl
// 
// Overview
// --------
// An implementation of MessageFactory that loads messages from a
// file.
//
//==============================================================================

#ifndef MessageFactoryImpl_h
#define MessageFactoryImpl_h

#include "defs.h"
#include "MessageSet.h"

#include "QcCore/base/MessageFactory.h"

#include <list>

QC_AUXIL_NAMESPACE_BEGIN

class QC_AUXIL_PKG MessageFactoryImpl : public MessageFactory
{
public:

	virtual bool getMessage(const String& org, const String& app,
	                        size_t messageID, String& ret);

protected:
	typedef AutoPtr<MessageSet> RPMessageSet;
	virtual RPMessageSet getMessageSet(const String& org, const String& app);

private:
	typedef std::list<RPMessageSet> MessageSetList;
	MessageSetList m_messageSetList;
};

QC_AUXIL_NAMESPACE_END

#endif //MessageFactoryImpl_h


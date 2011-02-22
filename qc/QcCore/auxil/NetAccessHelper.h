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
// Class: NetAccessHelper
// 
// Overview
// --------
// A class that eases the load on networking applications be performing some
// of the common boiler-plate functions like setting Proxy servers and passwords.
//
//==============================================================================

#ifndef NetAccessHelper_h
#define NetAccessHelper_h

#include "defs.h"

#include "QcCore/net/Authenticator.h"
#include "QcCore/net/InetAddress.h"

QC_AUXIL_NAMESPACE_BEGIN

using namespace net;

class QC_AUXIL_PKG NetAccessHelper : public Authenticator
{
public:
	NetAccessHelper(const String& HTTPProxy,
	                const String& proxyUserOption,
	                const String& proxyPasswordOption,
	                const String& userOption,
	                const String& passwordOption);

protected:
	// implement Authenticator interface...
	virtual bool getPasswordAuthentication(InetAddress* pAddr, int port,
		const String& protocol, const String& prompt, const String& scheme,
		bool bProxy, String& userRet, String& passwordRet);

private:
	String m_proxyUser;
	String m_proxyPassword;
	String m_user;
	String m_password;
};

QC_AUXIL_NAMESPACE_END

#endif //NetAccessHelper_h


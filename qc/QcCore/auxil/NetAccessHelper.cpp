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

#include "NetAccessHelper.h"
#include "BasicOption.h"

#include "QcCore/base/System.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/net/HttpURLConnection.h"
#include "QcCore/net/FtpURLConnection.h"

QC_AUXIL_NAMESPACE_BEGIN

using namespace net;

NetAccessHelper::NetAccessHelper(const String& HTTPProxy,
                                 const String& proxyUser,
                                 const String& proxyPassword,
                                 const String& user,
                                 const String& password)
{
	//
	// If a HTTP proxy server has been provided, then use it
	//
	String sHttpProxy = HTTPProxy.empty() 
	                  ? System::GetEnvironmentString(QC_T("QC_HTTP_PROXY"))
					  : HTTPProxy;

	if(!sHttpProxy.empty())
	{
		System::SetPropertyBool(QC_T("http.proxySet"), true);
		size_t portPos = sHttpProxy.find(QC_T(":"));
		if(portPos != String::npos)
		{
			String host = sHttpProxy.substr(0, portPos);
			String port = sHttpProxy.substr(portPos+1);
			System::SetProperty(QC_T("http.proxyHost"), host);
			System::SetPropertyLong(QC_T("http.proxyPort"), NumUtils::ToLong(port));
		}
		else
		{
			System::SetProperty(QC_T("http.proxyHost"), sHttpProxy);
		}
	}

	//
	// If a proxy password has been provided, then use it
	//
	m_proxyPassword = proxyPassword.empty()
	                ? System::GetEnvironmentString(QC_T("QC_HTTP_PROXY_PASSWORD"))
	                : proxyPassword;


	//
	// If a proxy user has been provided, then use it
	//
	m_proxyUser = proxyUser.empty()
		        ? System::GetEnvironmentString(QC_T("QC_HTTP_PROXY_USER"))
	            : proxyUser;

	//
	// If a password has been provided, then use it
	//
	m_password = password.empty()
		       ? System::GetEnvironmentString(QC_T("QC_HTTP_PASSWORD"))
	           : password;

	//
	// If a user has been provided, then use it
	//
	m_user = user.empty()
		   ? System::GetEnvironmentString(QC_T("QC_HTTP_USER"))
	       : user;
}

bool NetAccessHelper::getPasswordAuthentication(InetAddress* /*pAddr*/, int /*port*/,
	const String& /*protocol*/, const String& /*prompt*/, const String& /*scheme*/,
	bool bProxy, String& userRet, String& passwordRet)
{
	if(bProxy && !m_proxyUser.empty())
	{
		userRet = m_proxyUser;
		passwordRet = m_proxyPassword;
	}
	else
	{
		userRet = m_user;
		passwordRet = m_password;
	}
	return true;
}

QC_AUXIL_NAMESPACE_END

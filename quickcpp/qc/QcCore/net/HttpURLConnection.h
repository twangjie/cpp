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

#ifndef QC_NET_HttpURLConnection_h
#define QC_NET_HttpURLConnection_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "URLConnection.h"
#include "Socket.h"

QC_NET_NAMESPACE_BEGIN

class QC_NET_PKG HttpURLConnection : public URLConnection
{
public:

	enum HttpResponseCode {
		HTTP_ACCEPTED=202,
		HTTP_BAD_GATEWAY=502,
		HTTP_BAD_METHOD=405,
		HTTP_BAD_REQUEST=400,
		HTTP_CLIENT_TIMEOUT=408,
		HTTP_CONFLICT=409,
		HTTP_CREATED=201,
		HTTP_ENTITY_TOO_LARGE=413,
		HTTP_FORBIDDEN=403,
		HTTP_GATEWAY_TIMEOUT=504,
		HTTP_GONE=410,
		HTTP_INTERNAL_ERROR=500,
		HTTP_LENGTH_REQUIRED=411,
		HTTP_MOVED_PERM=301,
		HTTP_MOVED_TEMP=302,
		HTTP_MULT_CHOICE=300,
		HTTP_NO_CONTENT=204,
		HTTP_NOT_ACCEPTABLE=406,
		HTTP_NOT_AUTHORITATIVE=203,
		HTTP_NOT_FOUND=404,
		HTTP_NOT_IMPLEMENTED=501,
		HTTP_NOT_MODIFIED=304,
		HTTP_OK=200,
		HTTP_PARTIAL=206,
		HTTP_PAYMENT_REQUIRED=402,
		HTTP_PRECON_FAILED=412,
		HTTP_PROXY_AUTH=407,
		HTTP_REQ_TOO_LONG=414,
		HTTP_RESET=205,
		HTTP_SEE_OTHER=303,
		HTTP_UNAUTHORIZED=401,
		HTTP_UNAVAILABLE=503,
		HTTP_UNSUPPORTED_TYPE=415,
		HTTP_USE_PROXY=305,
		HTTP_VERSION=505 };

	HttpURLConnection(const URL& url);

	virtual void setRequestMethod(const String& method)=0;
	virtual String getRequestMethod() const=0;

	virtual int getResponseCode()=0;
	virtual String getResponseMessage()=0;

	virtual void setFollowRedirects(bool bFollow)=0;
	virtual bool getFollowRedirects() const=0;

	virtual AutoPtr<InputStream> getErrorStream() const=0;

public: // public static methods
	static void SetDefaultFollowRedirects(bool bFollow);
	static bool GetDefaultFollowRedirects();

private: // static member variables
	static bool s_bDefaultFollowRedirects;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_HttpURLConnection_h

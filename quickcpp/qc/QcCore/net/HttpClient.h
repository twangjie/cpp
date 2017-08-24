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

#ifndef QC_NET_HttpClient_h
#define QC_NET_HttpClient_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "TcpNetworkClient.h"
#include "MimeHeaderSequence.h"
#include "URL.h"

#include "QcCore/io/Writer.h"

QC_NET_NAMESPACE_BEGIN

using io::Writer;

class QC_NET_PKG HttpClient : public virtual QCObject, private TcpNetworkClient
{
public:

	HttpClient();

	void connectToURL(const URL& url, size_t timeoutMS);

	void setProxyHost(const String& host);
	void setProxyPort(int port);

	String getProxyHost() const;
	int getProxyPort() const;

	String getRequestMethod() const;
	void setRequestMethod(const String& method);

	AutoPtr<MimeHeaderSequence> getResponseHeaders() const;
	AutoPtr<MimeHeaderSequence> getRequestHeaders() const;

	bool getFollowRedirects() const;
	void setFollowRedirects(bool bFollow);

	URL getURL() const;
	int sendRequest();

	int getResponseCode() const;
	const String& getResponseMessage() const;
	const String& getResponseLine() const;

	virtual AutoPtr<InputStream> getInputStream() const;
	virtual AutoPtr<OutputStream> getOutputStream() const;

protected:
	virtual int getDefaultPort() const;
	virtual void postConnect(const String& server, int port, size_t timeoutMS);

private:
	void parseReturnedHeaders();
	bool createAuthorizationHeader();

private:
	AutoPtr<Writer>       m_rpWriter;
	AutoPtr<InputStream>  m_rpInputStream;
	AutoPtr<OutputStream> m_rpOutputStream;
	AutoPtr<MimeHeaderSequence> m_rpRequestHeaders;
	AutoPtr<MimeHeaderSequence> m_rpResponseHeaders;
	URL                  m_url;
	String               m_requestMethod;
	String               m_responseMessage;
	String               m_responseLine;
	int                  m_nResponseCode;
	bool                 m_bFollowRedirects;
	int                  m_nProxyPort;
	size_t               m_timeoutMS;
	String               m_proxyHost;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_HttpClient_h

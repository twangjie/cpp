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

#ifndef QC_NET_URL_h
#define QC_NET_URL_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "URLStreamHandler.h" // reqd for AutoPtr<>

#include "QcCore/base/AutoPtr.h"

QC_NET_NAMESPACE_BEGIN

class URLConnection;

class QC_NET_PKG URL
{
public:

	friend class URLStreamHandler;

	URL();
	URL(const String& spec);
	URL(const URL& context, const String& spec);
	URL(const String& protocol, const String& host, int port, const String& file);
	URL(const String& protocol, const String& host, int port, const String& file, URLStreamHandler* pHandler);
	URL(const String& protocol, const String& host, const String& file);

	const String& getAuthority() const;
	String getFile() const;
	const String& getHost() const;
	const String& getPath() const;
	int getPort() const;
	const String& getProtocol() const;
	const String& getQuery() const;
	const String& getRef() const;
	const String& getUserInfo() const;

	String getPassword() const;
	String getUserID() const;

	bool sameFile(const URL& rhs) const;
	String toExternalForm() const;

	bool equals(const URL& rhs) const;
	bool operator==(const URL& rhs) const;
	bool operator!=(const URL& rhs) const;

	AutoPtr<URLConnection> openConnection() const;
	AutoPtr<InputStream> openStream() const;
	AutoPtr<URLStreamHandler> getStreamHandler() const;

private:
	void parseSpecification(const String& spec);

	void set(const String& protocol,
             const String& host,
             int port,
             const String& authority,
             const String& userInfo,
             const String& path,
             const String& query,
             const String& ref);

	void init(const String& protocol,
	          const String& host,
	          int port,
	          const String& file,
	          URLStreamHandler* pHandler);

private:
	String m_protocol;
	String m_authority;
	String m_host;
	String m_path;
	String m_query;
	String m_ref;
	String m_userInfo;
	int m_port;
	AutoPtr<URLStreamHandler> m_rpHandler;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_URL_h


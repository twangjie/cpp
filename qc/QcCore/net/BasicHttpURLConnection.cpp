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
// Class BasicHttpURLConnection
/**
	@class qc::net::BasicHttpURLConnection
	
	@brief An @a specialized URLConnection class that represents a connection
	with a resource specified by a HTTP URL.

	<p>HTTP stands for Hyper Text Transfer Protocol, which is defined by
	RFCs published by the <a href="http://www.ietf.org/">Internet Engineering
	Task Force</a>.</p>

    @sa URLConnection
*/
//==============================================================================

#include "BasicHttpURLConnection.h"
#include "ProtocolException.h"

#include "QcCore/base/System.h"
#include "QcCore/base/Tracer.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/base/debug.h"

QC_NET_NAMESPACE_BEGIN

const size_t HTTP_TIMEOUT = 30000;  // 30 second connection timeout

//==============================================================================
// BasicHttpURLConnection::BasicHttpURLConnection
//
// Construct a BasicHttpURLConnection using the passed URL and default
// (or global) settings.
//==============================================================================
BasicHttpURLConnection::BasicHttpURLConnection(const URL& url) :
	HttpURLConnection(url),
	m_rpHttp(new HttpClient)
{
	//
	// If redirects have been disabled globally, then disable them
	// for this instance
	//
	if(!GetDefaultFollowRedirects())
	{
		m_rpHttp->setFollowRedirects(false);
	}
}

//=============================================================================
// BasicHttpURLConnection::connect
// 
// Creates a socket connection to the remote host and sends the HTTP
// request headers etc.
// 
// The HttpClient will follow redirect requests if they have been enabled
// (which they are by default).
//=============================================================================
void BasicHttpURLConnection::connect()
{
	if(!isConnected())
	{
		//
		// Connect to the server (if not already connected)
		//
		m_rpHttp->connectToURL(getURL(), HTTP_TIMEOUT);

		//
		// If caching is disabled, add a Mime-type header
		//	
		if(!getUseCaches())
		{
			m_rpHttp->getRequestHeaders()->setHeaderExclusive(QC_T("Pragma"), QC_T("no-cache"));
		}
		
		//
		// Send the HTTP request, and parse the response
		// (possibly following redirects along the way)
		//
		// This will throw a variety of exceptions if it fails
		//
		m_rpHttp->sendRequest();

		//
		//  If we get this far, we have successfully connected
		//
		setConnected(true);

		//
		//  Reset the URL in case of redirects
		//
		setURL(m_rpHttp->getURL());
	}
}

//==============================================================================
// BasicHttpURLConnection::getInputStream
//
// Returns a (ref-counted) ptr to the socket's InputStream.
//==============================================================================
AutoPtr<InputStream> BasicHttpURLConnection::getInputStream()
{
	connect();
	return m_rpHttp->getInputStream();
}

//==============================================================================
// BasicHttpURLConnection::getOutputStream
//
// Returns a (ref-counted) ptr to the socket's OutputStream.
//==============================================================================
AutoPtr<OutputStream> BasicHttpURLConnection::getOutputStream()
{
	if(!getDoOutput())
	{
		throw ProtocolException(QC_T("URLConnection not enabled for output"));
	}

	if(isConnected())
	{
		throw IOException(QC_T("already opened for input"));
	}

	//
	// To get an early indication of a host error, 
	// connect to the server (if not already connected).  This is 
	// not strictly required (at present), but it is good practice
	// and it prepares the way for using HTTP/1.1 continue
	//
	m_rpHttp->connectToURL(getURL(), HTTP_TIMEOUT);

	//
	// Ifthe request method is still at the default (GET), then
	// it should be changed to POST.  The HttpClient doesn't do this
	// for us - it thinks we know what we're doing!
	if(m_rpHttp->getRequestMethod() == QC_T("GET"))
	{
		m_rpHttp->setRequestMethod(QC_T("POST"));
	}

	return m_rpHttp->getOutputStream();
}

//==============================================================================
// BasicHttpURLConnection::setFollowRedirects
//
//==============================================================================
void BasicHttpURLConnection::setFollowRedirects(bool bFollow)
{
	m_rpHttp->setFollowRedirects(bFollow);
}

//==============================================================================
// BasicHttpURLConnection::getFollowRedirects
//
//==============================================================================
bool BasicHttpURLConnection::getFollowRedirects() const
{
	return m_rpHttp->getFollowRedirects();
}

//==============================================================================
// BasicHttpURLConnection::setRequestProperty
//
//==============================================================================
void BasicHttpURLConnection::setRequestProperty(const String& name,
                                                const String& value)
{
	m_rpHttp->getRequestHeaders()->setHeaderExclusive(name, value);
}

//==============================================================================
// BasicHttpURLConnection::getRequestProperty
//
//==============================================================================
String BasicHttpURLConnection::getRequestProperty(const String& name) const
{
	return m_rpHttp->getRequestHeaders()->getHeader(name);
}

//==============================================================================
// BasicHttpURLConnection::getHeaderField
//
//==============================================================================
String BasicHttpURLConnection::getHeaderField(const String& name)
{
	connect();
	return m_rpHttp->getResponseHeaders()->getHeader(name);
}

//==============================================================================
// BasicHttpURLConnection::getHeaderField
//
// Note: We synthesize the first header which is the response line.
//==============================================================================
String BasicHttpURLConnection::getHeaderField(size_t index)
{
	connect();

	if(index == 0)
		return m_rpHttp->getResponseLine();
	else
		return m_rpHttp->getResponseHeaders()->getHeader(index-1);
}

//==============================================================================
// BasicHttpURLConnection::getHeaderFieldKey
//
// Note: We synthesize the first header which is the response line.
//==============================================================================
String BasicHttpURLConnection::getHeaderFieldKey(size_t index)
{
	connect();

	if(index == 0)
		return String();
	else
		return m_rpHttp->getResponseHeaders()->getHeaderKey(index-1);
}

//==============================================================================
// BasicHttpURLConnection::getHeaderFieldCount
//
// Note: We synthesize the first header which is the response line.
//==============================================================================
size_t BasicHttpURLConnection::getHeaderFieldCount()
{
	connect();
	return m_rpHttp->getResponseHeaders()->size() + 1;
}

//==============================================================================
// BasicHttpURLConnection::getErrorStream
//
//==============================================================================
AutoPtr<InputStream> BasicHttpURLConnection::getErrorStream() const
{
	if(m_rpHttp->getResponseCode() >= 400)
	{
		return m_rpHttp->getInputStream();
	}
	else
	{
        return 0;
	}
}

//==============================================================================
// BasicHttpURLConnection::setRequestMethod
//
//==============================================================================
void BasicHttpURLConnection::setRequestMethod(const String& method)
{
	if(isConnected())
	{
		throw ProtocolException(QC_T("cannot set request method: already connected"));
	}

	m_rpHttp->setRequestMethod(method);
}

//==============================================================================
// BasicHttpURLConnection::getRequestMethod
//
//==============================================================================
String BasicHttpURLConnection::getRequestMethod() const
{
	return m_rpHttp->getRequestMethod();
}

//==============================================================================
// BasicHttpURLConnection::getResponseCode
//
//==============================================================================
int BasicHttpURLConnection::getResponseCode()
{
	connect();
	return m_rpHttp->getResponseCode();
}

//==============================================================================
// BasicHttpURLConnection::getResponseMessage
//
//==============================================================================
String BasicHttpURLConnection::getResponseMessage()
{
	connect();
	return m_rpHttp->getResponseMessage();
}

QC_NET_NAMESPACE_END

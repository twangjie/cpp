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
// http://www.ietf.org/rfc/rfc959.txt
//==============================================================================

#include "HttpClient.h"
#include "Authenticator.h"
#include "HttpChunkedInputStream.h"
#include "HttpURLConnection.h" // for response codes
#include "InetAddress.h"
#include "MimeHeaderParser.h"
#include "ProtocolException.h"
#include "Socket.h"
#include "ServerSocket.h"
#include "URLEncoder.h"

#include "QcCore/base/AutoBuffer.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/base/System.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/base/Tracer.h"
#include "QcCore/base/debug.h"
#include "QcCore/io/ByteArrayOutputStream.h"
#include "QcCore/io/FileNotFoundException.h"
#include "QcCore/io/IOException.h"
#include "QcCore/io/InputStreamReader.h"
#include "QcCore/io/OutputStreamWriter.h"
#include "QcCore/util/AttributeListParser.h"
#include "QcCore/util/Base64.h"
#include "QcCore/util/MessageFormatter.h"
#include "QcCore/util/StringTokenizer.h"

#include <map>

QC_NET_NAMESPACE_BEGIN

using namespace util;
using namespace io;

const int HTTP_PORT = 80;
const int PROXY_PORT = 8080;
const int MAX_REDIRECTS = 10; // HTTP 1.0 used to suggest 5

//==============================================================================
// HttpClient::HttpClient
//
/**
   Constructs a new HttpClient with default values.
*/
//==============================================================================
HttpClient::HttpClient() :
	m_rpRequestHeaders(new MimeHeaderSequence),
	m_rpResponseHeaders(new MimeHeaderSequence),
	m_requestMethod(QC_T("GET")),
	m_nResponseCode(-1),
	m_bFollowRedirects(true),
	m_nProxyPort(0),
	m_timeoutMS(0)
{
	if(System::GetPropertyBool(QC_T("http.proxySet"), false))
	{
		m_proxyHost = System::GetProperty(QC_T("http.proxyHost"));
		m_nProxyPort = (int)System::GetPropertyLong(QC_T("http.proxyPort"), PROXY_PORT);
	}
}

//==============================================================================
// HttpClient::getDefaultPort
//
/**
   Returns the default port for HTTP servers which is 80.
*/
//==============================================================================
int HttpClient::getDefaultPort() const
{
	return HTTP_PORT;
}

//==============================================================================
// HttpClient::postConnect
//
/**
   Protected function called when a socket connection has been established with 
   the TCP/IP network server.
*/
//==============================================================================
void HttpClient::postConnect(const String& /*server*/,
                             int /*port*/,
                             size_t /*timeoutMS*/)
{
	const String encoding = QC_T("ISO-8859-1");

	m_rpWriter = new OutputStreamWriter(
		TcpNetworkClient::getOutputStream().get(), encoding);
}

//==============================================================================
// HttpClient::connectToURL
//
/**
   Establishes a TCP/IP socket connection with the host that will
   satisfy requests for the provided URL.

   This may connect to the host name contained within the URL, or to 
   a proxy server if one has been set.

   This function does not send any information to the remote server
   after the connection is established.

   @param url a fully-qualified http URL for the required resource
   @param timeoutMS the time-out value in milliseconds.  A value of zero indicates
          that the client will wait forever, or until the underlying
		  operating system decides that the connection cannot be established.
   @throws IOException if an error occurs trying to connect to the server
   @throws ProtocolException if an invalid response is received from the server
   @throws UnknownHostException if an error occurs trying to resolve the host name
   @throws SocketTimeoutException if the specified timeout value expires while
           attempting to connect to the server
   @sa sendRequest()
*/
//==============================================================================
void HttpClient::connectToURL(const URL& url, size_t timeoutMS)
{
	//
	// Guard against multiple connections
	//
	if(isConnected())
		return;

	//
	// Check we've been passed a valid URL
	//
	if(StringUtils::CompareNoCase(url.getProtocol().substr(0,4), QC_T("http")) != 0)
	{
		throw IllegalArgumentException(QC_T("not a http URL"));
	}

	//
	// Save the URL and timeout for the future
	// (in case we need them again e.g. when redirected to another host)
	//
	m_url = url;
	m_timeoutMS = timeoutMS;

	String sHost = url.getHost();
	int port = url.getPort();

	//
	// If we need to use a proxy server, then connect to that,
	// otherwise create a connection to the specified host.
	//
	if(m_proxyHost.empty())
	{
		TcpNetworkClient::connect(sHost, port, timeoutMS);
	}
	else
	{
		TcpNetworkClient::connect(m_proxyHost, m_nProxyPort, timeoutMS);
	}
}

//=============================================================================
// HttpClient::sendRequest
// 
// Send the http request to the remote host.
// 
// The request is made up of a request line followed by request
// header fields.  Eg:-
// GET filename HTTP/1.1
// Host: hostname[:port]
// Connection: close
// 
// The response from the server will contain a number of header
// fields followed by the requested data.  The headers are parsed
// and removed leaving the InputStream to point at the requested
// data.
//
// Note: HTTP Redirection
// ----------------------
// By default we will follow HTTP redirects.  These are communicated
// to us by a 3xx HTTP response code and the presence of a "Location" header
// field.  A 3xx response code without a Location header is an error.
// Redirection may be an iterative process, so it continues until
// we receive a 200 OK response orthe maximum number of redirects is exceeded.
//
// We do not process Location headers when accompanying a 200 OK response.
//=============================================================================
int HttpClient::sendRequest()
{
	if(!isConnected())
	{
		throw ProtocolException(QC_T("Http client not connected"));
	}

	int redirectCount=0;
	const String CRLF = QC_T("\r\n");

	//
	// Add a User-Agent header identifying QuickCPP, unless the application
	// has inserted its own header
	//
	String sUserAgent = QC_T("QuickCPP ");
	sUserAgent += System::GetVersionAsString();
	m_rpRequestHeaders->setHeaderIfAbsent(QC_T("User-Agent"), sUserAgent);

	while(redirectCount == 0 || (m_bFollowRedirects && redirectCount < MAX_REDIRECTS))
	{
		if(!isConnected())
			connectToURL(m_url, m_timeoutMS);

		String request = m_requestMethod + QC_T(" ");
		
		if(m_proxyHost.empty())
		{
			if(m_url.getFile().empty())
			{
				request += QC_T("/"); 
			}
			else
			{
				request += URLEncoder::RawEncode(m_url.getFile());
			}
			request += QC_T(" HTTP/1.1");

			// As we don't yet support persistent connections
			// we must include a Connection: close header
			m_rpRequestHeaders->setHeaderExclusive(QC_T("Connection"), QC_T("close"));
		}
		else
		{
			// We'll drop down to HTTP 1.0 for the proxy server
			// I don't know why, but this is what JDK does from examining
			// the httpd log
			//
			// The URL should probably not contain the user information
			// part or the ref part when it is transmitted to the server,
			// so we'll assemble a synthesized URL missing those components
			//
			request += m_url.getProtocol();
			request += QC_T("://");
			request += m_url.getHost();
			request += URLEncoder::RawEncode(m_url.getPath());
			if(!m_url.getQuery().empty())
			{
				request += QC_T("?");
				request += URLEncoder::RawEncode(m_url.getQuery());
			}

			request += QC_T(" HTTP/1.0");

			// As we don't yet support persistent connections
			// we must include a Connection: close header
			m_rpRequestHeaders->setHeaderExclusive(QC_T("Proxy-Connection"), QC_T("close"));
		}

		//
		// If the URL contains a port, then add that to the Host header
		//
		String sHost = m_url.getHost();
		if(m_url.getPort() != -1)
		{
			sHost += QC_T(":");
			sHost += NumUtils::ToString(m_url.getPort());
		}
		m_rpRequestHeaders->setHeaderExclusive(QC_T("Host"), sHost);

		Tracer::Trace(Tracer::Net, Tracer::Low, request);

		//
		// If we have created an output stream (on demand)
		// then close it and use its length to generate a 
		// "content-length" header
		//
		ByteArrayOutputStream* pOS = reinterpret_cast<ByteArrayOutputStream*>(m_rpOutputStream.get());
		if(pOS)
		{
			m_rpOutputStream->close();
			m_rpRequestHeaders->setHeaderExclusive(
				QC_T("content-length"),
				NumUtils::ToString(pOS->size()));
		}

		//
		// Write the request to the HTTP server
		//
		m_rpWriter->write(request + CRLF);

		//
		// Write the MIME-type headers to the HTTP server
		//
		m_rpRequestHeaders->writeHeaders(m_rpWriter.get());

		//
		// Indicate the end of the headers with an empty line
		//
		m_rpWriter->write(CRLF);
		m_rpWriter->flush();

		//
		// Write the contents of the OutputStream (if any).
		// Note: this of course by-passes the writer and writes 
		// directly to the byte stream
		//
		if(pOS)
		{
			pOS->writeTo(TcpNetworkClient::getOutputStream().get());
			TcpNetworkClient::getOutputStream()->flush();
		}

		//
		// Read the result line and attached headers...
		//
		parseReturnedHeaders();

		if(m_bFollowRedirects && m_nResponseCode >= 300 && m_nResponseCode < 400)
		{
			disconnect();

			++redirectCount;

			if(redirectCount > MAX_REDIRECTS)
			{
				throw IOException(QC_T("Redirection limit reached"));
			}

			String newLocation = m_rpResponseHeaders->getHeader(QC_T("Location"));

			if(newLocation.empty())
			{
				const String& errMsg = MessageFormatter::Format(
					QC_T("Location header missing from HTTP redirect response: {0} ({1}) for URL: {2}"),
					NumUtils::ToString(m_nResponseCode),
					m_responseMessage,
					m_url.toExternalForm());
				throw ProtocolException(errMsg);
			}
			else
			{
				//
				// Combine the new location with our URL
				//
				m_url = URL(m_url, newLocation);

				//
				// New locations will possibly need different authentication,
				// so we should reset our origin authentication header (if any)
				//
				m_rpRequestHeaders->removeAllHeaders(QC_T("Authorization"));	
			}
		}
		else if(m_nResponseCode == HttpURLConnection::HTTP_UNAUTHORIZED)
		{
			//
			// If we haven't already done so, attempt to create an 
			// Authentication header.  If this fails
			// (due to application not passing the credentials), then
			// we treat it as an error.
			//
			// If we already have one then the server is rejecting it so
			// we have an error anyway.
			//
			if(m_rpRequestHeaders->containsHeader(QC_T("Authorization"))
			   || !createAuthorizationHeader())
			{
				break;
			}
			disconnect();
		}
		else if(m_nResponseCode == HttpURLConnection::HTTP_PROXY_AUTH)
		{
			//
			// If we haven't already done so, attempt to create an 
			// Authentication header.  If this fails
			// (due to application not passing the credentials), then
			// we treat it as an error.
			//
			// If we already have one then the server is rejecting it so
			// we have an error anyway.
			//
			if(m_rpRequestHeaders->containsHeader(QC_T("Proxy-Authorization"))
			   || !createAuthorizationHeader())
			{
				break;
			}
			disconnect();
		}
		else
		{
			break;
		}
	}

	const String& tfrEncoding = 
		m_rpResponseHeaders->getHeader(QC_T("Transfer-Encoding"));

	if(StringUtils::CompareNoCase(tfrEncoding, QC_T("chunked")) ==0)
	{
		m_rpInputStream = new HttpChunkedInputStream(
			TcpNetworkClient::getInputStream().get());
	}
	else
	{
		m_rpInputStream = TcpNetworkClient::getInputStream();
	}

	//
	// Any error code 300 or above becomes an eexception at this point.
	//
	if(m_nResponseCode >= 300) 
	{
		//
		// We don't disconnect from the server because the application
		// may wish to read from the error stream after catching
		// the exception we are about to throw!
		//disconnect();

		if(m_nResponseCode == HttpURLConnection::HTTP_NOT_FOUND)
		{
			throw FileNotFoundException(m_url.toExternalForm());
		}
		else
		{
			const String& errMsg = MessageFormatter::Format(
				QC_T("server returned HTTP response: {0} ({1}) for URL: {2}"),
				NumUtils::ToString(m_nResponseCode),
				getResponseMessage(),
				m_url.toExternalForm());
			throw IOException(errMsg);
		}
	}

	return m_nResponseCode;
}

//==============================================================================
// HttpClient::createAuthenticationHeader
//
// In response to a HTTP_UNAUTHORISED response from the HTTP server,
// this function will attempt to generate an Authentication header
// to satisfy the server.
//==============================================================================
bool HttpClient::createAuthorizationHeader()
{
	const bool bProxy = (m_nResponseCode == HttpURLConnection::HTTP_PROXY_AUTH);

	const String sHeader = bProxy
	                     ? QC_T("Proxy-Authenticate")
	                     : QC_T("WWW-Authenticate");
	
	const String& authResponse = m_rpResponseHeaders->getHeader(sHeader);

	if(authResponse.empty())
	{
		// assume there's no need for an error stream
		// in this case
		disconnect();

		const String& errMsg = MessageFormatter::Format(
			QC_T("{0} header missing from HTTP response: {1} ({2}) for URL: {3}"),
			sHeader,
			NumUtils::ToString(m_nResponseCode),
			getResponseMessage(),
			getURL().toExternalForm());
		throw ProtocolException(errMsg);
	}

	//
	// The authentication header is constructed like a tagged attribute
	// list.  The AttributeListParser can deal with this.
	//
	AttributeListParser parser;
	if(!parser.parseString(authResponse))
	{
		const String& errMsg = MessageFormatter::Format(
			QC_T("{0} header value: {1} is invalid"),
			sHeader, authResponse);
		throw ProtocolException(errMsg);
	}

	const String& scheme = parser.getAttributeValue(0);
	const String& realm = parser.getAttributeValueICase(QC_T("realm"));

	//
	// Obtain a userid and password for the scheme/realm returned
	// from the HTTP server
	//
	String user, password;

	if(!Authenticator::RequestPasswordAuthentication(getServerAddress().get(),
		getServerPort(), QC_T("HTTP"),
		realm, scheme, bProxy,
		user, password))
	{
		//
		// If the registered Authenticator cannot supply a user/password
		// then we cannot continue.  This is signalled by returning
		// false to the sendRequest() function.
		//
		return false;
	}

	//
	// According to RFC2617 HTTP Authentication: Basic and Digest Access Authentication
	// For "Basic" authentication, the user and password are concatentated with a
	// colon separator before being encoded in base64
	//
	// According to RFC 2068 (HTTP/1.1) the Username and Password are defined as
	// TEXT productions and are therefore supposed to be encoded in ISO-8859-1
	// before being Base64-encoded.
	//
	// TODO: schemes may be listed, we need to parse the value
	//
	if(StringUtils::CompareNoCase(scheme, QC_T("Basic")) == 0)
	{
		const String sColon = QC_T(":");
		String headerValue = QC_T("Basic ");

		String combo = user + sColon + password;
		ByteString bytes = StringUtils::ToLatin1(combo);

		const size_t base64Size = 
			Base64::GetEncodedLength((const Byte*)bytes.data(),
			                         (const Byte*)bytes.data()+bytes.size());

		ArrayAutoPtr<Byte> apBuffer(new Byte[base64Size]);
		Byte* pToNext;
		if(Base64::Encode((const Byte*)bytes.data(),
		                  (const Byte*)bytes.data()+bytes.size(),
		                  apBuffer.get(),
		                  apBuffer.get()+base64Size,
		                  pToNext) != Base64::ok)
		{
			throw IOException(QC_T("unable to base64 encode password"));
		}

		QC_DBG_ASSERT(base64Size ==  (size_t)(pToNext-apBuffer.get()));
		
		headerValue += StringUtils::FromLatin1((const char*)apBuffer.get(), pToNext-apBuffer.get());

		const String sReqHeader = bProxy
		                        ? QC_T("Proxy-Authorization")
			                    : QC_T("Authorization");

		m_rpRequestHeaders->setHeaderExclusive(sReqHeader, headerValue);

		return true;
	}
	else
	{
		//
		// Only basic authentication is supported at present.  By failing
		// to create an authentication header, and returning false, we
		// signal to the caller that the authenticate response should be
		// treated as an error.
	}

	return false;
}

//=============================================================================
// HttpClient::parseReturnedHeaders
// 
// A typical http response will take the form:-
//
//HTTP/1.1 200 OK
//Via: 1.1 PROXY-01
//Connection: Keep-Alive
//Proxy-Connection: Keep-Alive
//Transfer-Encoding: chunked
//Expires: Tue, 22 Feb 2011 05:10:06 GMT
//Date: Tue, 22 Feb 2011 05:01:14 GMT
//Content-Type: text/html
//Server: nginx/0.7.65
//Vary: Accept-Encoding
//Vary: Accept-Encoding
//X-Powered-By: PHP/5.2.10
//Cache-Control: max-age=120
//X-Cache: MISS from web
// 
// <the data>
//
// We make use of the MimeHeaderParser utility module to parse the headers
// for us.
//=============================================================================
void HttpClient::parseReturnedHeaders()
{
	QC_DBG_ASSERT(isConnected());

	const AutoPtr<InputStream>& rpInputStream = TcpNetworkClient::getInputStream();

	//
	// reset our state information
	//
	m_nResponseCode = 0;
	m_responseMessage.erase();

	MimeHeaderParser::ReadLineLatin1(rpInputStream.get(), m_responseLine);

	//
	// Parse the response line:
	// HTTP/n.n nnn <ssss>?
	//
	size_t codePos = m_responseLine.find(QC_T(' '));
	size_t msgPos = String::npos;
	if(codePos != String::npos)
	{
		msgPos = m_responseLine.find(QC_T(' '), codePos+1);
	}
	if(msgPos != String::npos)
	{
		String strCode = m_responseLine.substr(codePos+1, msgPos-(codePos+1));
		m_nResponseCode = NumUtils::ToLong(strCode);
		m_responseMessage = m_responseLine.substr(msgPos+1);
	}
	else
	{
		m_responseMessage.erase();
	}

	m_rpResponseHeaders = MimeHeaderParser::ParseHeaders(rpInputStream.get());
}

//==============================================================================
// HttpClient::getInputStream
//
// Returns a (ref-counted) ptr to the socket's InputStream.
//==============================================================================
AutoPtr<InputStream> HttpClient::getInputStream() const
{
	return m_rpInputStream;
}

//==============================================================================
// HttpClient::getOutputStream
//
// Returns an OutputStream that can be used by the application for
// POST or PUT operations.
//==============================================================================
AutoPtr<OutputStream> HttpClient::getOutputStream() const
{
	if(!m_rpOutputStream)
	{
		const_cast<HttpClient*>(this)->m_rpOutputStream = 
			new ByteArrayOutputStream;
	}
	return m_rpOutputStream;
}

//==============================================================================
// HttpClient::setRequestMethod
//
//==============================================================================
void HttpClient::setRequestMethod(const String& method)
{
	m_requestMethod = method;
}

//==============================================================================
// HttpClient::getRequestMethod
//
//==============================================================================
String HttpClient::getRequestMethod() const
{
	return m_requestMethod;
}

//==============================================================================
// HttpClient::getResponseCode
//
//==============================================================================
int HttpClient::getResponseCode() const
{
	return m_nResponseCode;
}

//==============================================================================
// HttpClient::getResponseMessage
//
//==============================================================================
const String& HttpClient::getResponseMessage() const
{
	return m_responseMessage;
}

//==============================================================================
// HttpClient::setProxyHost
//
/**
   Sets a proxy host to use for connections from this HttpClient instance.
*/
//==============================================================================
void HttpClient::setProxyHost(const String& host)
{
	m_proxyHost = host;
}

//==============================================================================
// HttpClient::setProxyPort
//
/**
   Sets the port number on the proxy host to use for connections from this
   HttpClient instance.
*/
//==============================================================================
void HttpClient::setProxyPort(int port)
{
	m_nProxyPort = port;
}

//==============================================================================
// HttpClient::getProxyHost
//
/**
   Returns the name of the proxy host that will be used for this
   HttpClient instance.
*/
//==============================================================================
String HttpClient::getProxyHost() const
{
	return m_proxyHost;
}

//==============================================================================
// HttpClient::getProxyPort
//
/**
   Returns the proxy host port number that will be used for this
   HttpClient instance.
*/
//==============================================================================
int HttpClient::getProxyPort() const
{
	return m_nProxyPort;
}

//==============================================================================
// HttpClient::getResponseHeaders
//
/**
   Returns a sequence of Mime-type headers that were received in
   response from the HTTP server.
*/
//==============================================================================
AutoPtr<MimeHeaderSequence> HttpClient::getResponseHeaders() const
{
	return m_rpResponseHeaders;
}

//==============================================================================
// HttpClient::getRequestHeaders
//
/**
   Returns a modifiable sequence of Mime-type headers that will be used to form
   a request to the HTTP server.
*/
//==============================================================================
AutoPtr<MimeHeaderSequence> HttpClient::getRequestHeaders() const
{
	return m_rpRequestHeaders;
}

//==============================================================================
// HttpClient::getFollowRedirects
//
/**
   Returns a flag indicating if HTTP redirects will be followed.

   @returns @c true if redirect requests will be followed (the default);
            @c false otherwise
   @sa setFollowRedirects()
*/
//==============================================================================
bool HttpClient::getFollowRedirects() const
{
	return m_bFollowRedirects;
}

//==============================================================================
// HttpClient::setFollowRedirects
//
/**
   Sets a flag indicating if HTTP redirects will be followed.

   @param bFollow @c true for redirect requests to be followed (the default);
                  @c false prevents this behaviour.
   @sa getFollowRedirects()
*/
//==============================================================================
void HttpClient::setFollowRedirects(bool bFollow)
{
	m_bFollowRedirects = bFollow;
}

//==============================================================================
// HttpClient::getURL
//
/**
   Returns the URL that was last connect.  This will reflect the latest
   URL, updated as a result of any HTTP redirect processing.
*/
//==============================================================================
URL HttpClient::getURL() const
{
	return m_url;
}

//==============================================================================
// HttpClient::getResponseLine
//
/**
   Returns the response line from the HTTP server.
*/
//==============================================================================
const String& HttpClient::getResponseLine() const
{
	return m_responseLine;
}

QC_NET_NAMESPACE_END

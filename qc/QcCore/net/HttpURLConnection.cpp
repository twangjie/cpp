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
// Class HttpURLConnection
/**
	@class qc::net::HttpURLConnection
	
	@brief A @a specialized URLConnection class that represents a connection
	with a resource specified by a HTTP URL.

	<p>HTTP stands for Hyper Text Transfer Protocol, which is defined by
	RFCs published by the <a href="http://www.ietf.org/">Internet Engineering
	Task Force</a>.</p>

    A HttpURLConnection instance can make only one HTTP request (although
	this one request may involve several message exchanges if HTTP redirection
	or authorization is involved).  However, HTTP 1.1 introduces
	persistent connections which can make more effective use of TCP/IP.  @QuickCPP
	may make use of persistent connections, but this is an implementation detail
	which is not exposed to the HttpURLConnection interface.
*/
//==============================================================================

#include "HttpURLConnection.h"
#include "ProtocolException.h"

QC_NET_NAMESPACE_BEGIN

bool HttpURLConnection::s_bDefaultFollowRedirects = true;

//======================================================================
// Multi-threaded locking strategy
//
// The only global value is a boolean, so this can safely be accessed
// from multiple threads.  Timing should not be important, the 
// application should be able to set this before any threads require it.
//======================================================================

//==============================================================================
// HttpURLConnection::HttpURLConnection
//
/**
   Constructs a HttpURLConnection for the specified HTTP URL.
*/
//==============================================================================
HttpURLConnection::HttpURLConnection(const URL& url) :
	URLConnection(url)
{
}

//==============================================================================
// HttpURLConnection::SetDefaultFollowRedirects
//
/**
   Set the global @c FollowRedirects value which is used to initialize new
   HttpURLConnection objects.  
   
   Setting this does not affect any HttpURLConnection objects already
   constructed.

   The default value is @true.

   @sa setFollowRedirects()
   @sa GetDefaultFollowRedirects()
*/
//==============================================================================
void HttpURLConnection::SetDefaultFollowRedirects(bool bFollow)
{
	s_bDefaultFollowRedirects = bFollow;
}

//==============================================================================
// HttpURLConnection::GetDefaultFollowRedirects
//
/**
   Returns the global @c FollowRedirects value which is used to initialize
   new HttpURLConnection objects.

   If this has not been explicitly set, the default value is @c true.

   @sa SetDefaultFollowRedirects()
*/
//==============================================================================
bool HttpURLConnection::GetDefaultFollowRedirects()
{
	return s_bDefaultFollowRedirects;
}

#ifdef QC_DOCUMENTATION_ONLY
//=============================================================================
//
// Documentation for pure virtual methods follows:
//
//=============================================================================

//==============================================================================
// HttpURLConnection::setRequestMethod
//
/**
   Sets the request method that will be transmitted to the HTTP server.

   The default is @c GET for input requests and @c POST for output
   requests.  Other possible alternatives are:-
   - HEAD 
   - OPTIONS 
   - PUT 
   - DELETE 
   - TRACE 

   See http://ietf.org/rfc/rfc2616.txt for further information about HTTP
   request methods.

   @throws ProtocolException if the request method cannot be set or the
           passed @c method is invalid.
   @sa getRequestMethod()
*/
//==============================================================================
void HttpURLConnection::setRequestMethod(const String& method);

//==============================================================================
// HttpURLConnection::getRequestMethod
//
/**
   Returns the request method.
   @sa setRequestMethod()
*/
//==============================================================================
String HttpURLConnection::getRequestMethod() const;

//==============================================================================
// HttpURLConnection::getResponseCode
//
/**
   Returns the numeric response code returned from the HTTP server.  This
   will attempt to connect to the HTTP server and access the specified 
   resource if it is not already connected.

   Refer to the enumeration HttpURLConnection::HttpResponseCode
   for a list of possible values.

   @throws IOException is an error occurs while reading from or writing
           to the HTTP server.
   @sa getResponseMessage()
*/
//==============================================================================
int HttpURLConnection::getResponseCode();

//==============================================================================
// HttpURLConnection::getResponseMessage
//
/**
   Returns the message text from the HTTP response line.

   For example, an HTTP response of <tt>HTTP/1.1 200 OK</tt>
   will return @c "OK".
 
   @sa getResponseCode()
*/
//==============================================================================
String HttpURLConnection::getResponseMessage();

//==============================================================================
// HttpURLConnection::setFollowRedirects
//
/**
   Specifies whether or not HTTP redirect responses should be followed.

   The HTTP protocol allows for resources to be moved from one location
   to another.  When this occurs, the original server may respond to the client
   with a @c HTTP_MOVED_PERM (301) or @c HTTP_MOVED_TEMP (302) message.  When 
   redirects are enabled, this will cause the resource to be fetched from the new
   location, otherwise the request will fail with a FileNotFoundException.

   @sa getFollowRedirects()
*/
//==============================================================================
void HttpURLConnection::setFollowRedirects(bool bFollow);

//==============================================================================
// HttpURLConnection::getFollowRedirects
//
/**
   Returns a flag indicating whether HTTP redirects will be followed.

   @sa setFollowRedirects()
   @sa SetDefaultFollowRedirects()
*/
//==============================================================================
bool HttpURLConnection::getFollowRedirects() const;

//==============================================================================
// HttpURLConnection::getErrorStream
//
/**
   Returns an InputStream that contains the output from the HTTP server
   after an error occurs.

   For example, if a request results in a "404 File Not Found" error, the
   server may respond with additional useful information.  This additional
   data (if any) will be returned in the error stream.

   @returns an InputStream or null if no error occurred, or this 
   HttpURLConnection is not yet connected to the HTTP server.
*/
//==============================================================================
AutoPtr<InputStream> HttpURLConnection::getErrorStream() const;

#endif //QC_DOCUMENTATION_ONLY

QC_NET_NAMESPACE_END

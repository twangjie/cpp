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
//=============================================================================
//
// Class: URL
// 
/**
	@class qc::net::URL
	
	Represents a <em>Uniform Resource Locator</em> (URL).  This class
	provides the capability to parse, manipulate and compare URL strings in addition
	to making a URL's resource available as an InputStream.

	The following example opens an InputStream to read from a URL:-
    @code
    try {
        const URL url(QC_T("http://www.google.com"));
        AutoPtr<InputStream> rpIS = url.openStream();
        // wrap the InputStream with an InputStreamReader to convert the
        // byte stream into Unicode characters
        AutoPtr<InputStreamReader> rpReader(new InputStreamReader(rpIS.get());
        CharType ch;
        while( (ch = rpReader->read()) != Reader::EndOfFile) {
            // do something
        }
    }
    catch(Exception& e) {
        Console::cerr() << e.toString() << endl;
    }
    @endcode

	<h4>The URL Format</h4>
	A URL is a string representation of a resource that is available via the
	Internet.  The format of URLs is formally defined in the IETF RFC 1738 
	which is available online at http://www.ietf.org/rfc/rfc1738.txt
	(which is itself a URL!)

	The URL syntax is dependent upon the scheme.  In general, absolute
	URL are written as follows:

	@c <scheme>:<scheme-specific-part>

	A URL contains the name of the scheme being used (<scheme>) followed
	by a colon and then a string (the <scheme-specific-part>) whose
	interpretation depends on the scheme.

	The URL syntax does not require that the scheme-specific-part is common
	among all URL, however, many forms of URL do share a common syntax for
	representing hierarchical relationships.  This
	"generic URL" syntax consists of a sequence of four main components:

	@c <scheme>://<authority><path>?<query>

	The @b scheme is often the name of a network protocol which can be used to 
	retrieve the resource from the Internet.  The words @a protocol and
	@a scheme are used interchangeably within this document.

	The @b authority is comprised of three sub-components:

	@c <userInfo@><host><:port>

	The @b path is comprised of everything following the authority up to
	the query part.  In contrast to the description in RFC 1738, this class
	includes the "/" separator between the authority part and the path as
	part of the path.

	@QuickCPP supports URLs for the following schemes: @c file, @c http and 
	@c ftp.  However, it is possible to extend this by creating a custom
	URLStreamHandlerFactory.

	URL strings can be either @a absolute or @a relative.  A relative URL
	(described in <a href="http://www.ietf.org/rfc/rfc1808.txt">RFC 1808</a>))
	is interpreted within the context of another absolute URL.
	For example, the URL:

	@c rfc1808.txt

	is a valid example of a relative URL, but it is meaningless unless
	it is resolved within the context of an absolute URL such as:

	@c http://www.ietf.org/rfc/rfc1738.txt

	In the above case, the relative URL resolves by replacing the filename
	from the context URL with @c rfc1808.txt.  

	The URL class represents an absolute URL.  Constructors are available
	that facilitate the creation of an absolute URL from 
	a relative URL, interpreted within the context of another absolute URL.
*/
//=============================================================================

#include "URL.h"
#include "URLConnection.h"
#include "URLDecoder.h"
#include "URLStreamHandlerFactory.h"
#include "URLStreamHandler.h"
#include "MalformedURLException.h"

#include "QcCore/base/debug.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/base/StringUtils.h"

QC_NET_NAMESPACE_BEGIN

//==============================================================================
// URL::URL
//
/**
   Default constructor..
*/
//==============================================================================
URL::URL() :
	m_port(-1)
{
}

//==============================================================================
// URL::URL
//
/**
   Constructs a URL by parsing the string @c spec.

   @throws MalformedURLException if @c spec does not contain a supported
   protocol.
*/
//==============================================================================
URL::URL(const String& spec)  :
	m_port(-1)
{
	parseSpecification(spec);
}

//==============================================================================
// URL::URL
//
/**
   Constructs a URL by parsing the string @c spec within the context of an
   existing URL @c context.

   @param context a URL which provides a context for the interpretation of
          the String @c spec
   @param spec a String containing either a relative or absolute URL

   @throws MalformedURLException if @c spec is absolute and contains a 
           protocol which is not supported or if @c context does not
		   contain a valid protocol.
*/
//==============================================================================
URL::URL(const URL& context, const String& spec)
{
	// extract all the details from the context URL...
	*this = context;
	// ... then substitute the bits from the spec
	// with the exception of ref - that shouldn't be copied
	m_ref = QC_T("");
	parseSpecification(spec);
}

//==============================================================================
// URL::URL
//
/**
   Constructs a URL with the components set from the parameters
   provided.

   @param protocol the protocol of the URL.  This must be a supported protocol
   @param host the name or dotted IP address of the network host
   @param port the port number of the remote server
   @param file the filename

   @throws MalformedURLException if @c protocol is not supported
*/
//==============================================================================
URL::URL(const String& protocol, const String& host, int port, const String& file)
{
	init(protocol, host, port, file, 0);
}

//==============================================================================
// URL::URL
//
/**
   Constructs a URL with the components set from the parameters
   provided.

   @param protocol the protocol of the URL.  This must be a supported protocol
   @param host the name or dotted IP address of the network host
   @param port the port number of the remote server
   @param file the filename
   @param pHandler the URLStreamHandler that understands how to create a
          URLConnection for the given protocol

   @throws MalformedURLException if @c protocol is not supported and pHandler
           is null
*/
//==============================================================================
URL::URL(const String& protocol, const String& host, int port, const String& file, URLStreamHandler* pHandler)
{
	init(protocol, host, port, file, pHandler);
}

//==============================================================================
// URL::URL
//
/**
   Constructs a URL with the components set from the parameters
   provided.

   @param protocol the protocol of the URL.  This must be a supported protocol
   @param host the name or dotted IP address of the network host
   @param file the filename

   @throws MalformedURLException if @c protocol is not supported
*/
//==============================================================================
URL::URL(const String& protocol, const String& host, const String& file)
{
	init(protocol, host, -1, file, 0);
}

//==============================================================================
// URL::init
//
// Private initialization function.
//==============================================================================
void URL::init(const String& protocol,
			   const String& host,
			   int port,
			   const String& file,
			   URLStreamHandler* pHandler)
{
	// init() is only called as part of construction
	QC_DBG_ASSERT(m_rpHandler.isNull());

	m_protocol = protocol;
	m_port = port;
	m_authority = m_host = host;
	m_path = file;

	if(m_port != -1)
	{
		m_authority += QC_T(":");
		m_authority += NumUtils::ToString(m_port);
	}

	size_t queryPos = file.find(QC_T('?'));
	if(queryPos != String::npos)
	{
		m_path = file.substr(0, queryPos);
		m_query = file.substr(queryPos+1, file.length()-(queryPos+1));
	}

	if(pHandler)
	{
		m_rpHandler = pHandler;
	}
	else if( (m_rpHandler = URLStreamHandlerFactory::GetInstance()->createURLStreamHandler(protocol)).isNull())
	{
		static const String errMsg = QC_T("unknown protocol: ");
		throw MalformedURLException(errMsg + protocol);
	}
}

//==============================================================================
// URL::set
//
// Private implementation, called by URLStreamHandler which is a friend
//==============================================================================
void URL::set(const String& protocol,
              const String& host,
              int port,
              const String& authority,
              const String& userInfo,
              const String& path,
              const String& query,
              const String& ref)
{
	m_protocol = protocol;
	m_host = host;
	m_port = port;
	m_authority = authority;
	m_userInfo = userInfo;
	m_path = path;
	m_query = query;
	m_ref = ref;
}

//=============================================================================
// URL::parseSpecification
// 
// Locate the protocol from the passed specification, obtain a 
// URLStreamHandler for the protocol and use that to parse the rest of the
// specification.
//
//=============================================================================
void URL::parseSpecification(const String& spec)
{
	size_t protpos;
	size_t refpos;
	String protocol = m_protocol;
	AutoPtr<URLStreamHandlerFactory> rpFactory = URLStreamHandlerFactory::GetInstance();	

	//
	// locate the protocol
	//
	if( (protpos = spec.find(QC_T(":"), 0)) != String::npos)
	{
		protocol = spec.substr(0, protpos);

		//
		// Warning: Just because we have found a ":" does not necessarily mean
		// we have found a protocol.  If the spec is relative then it may contain
		// a ":" as the drive delimiter on windows eg C:\rob.txt
		//
		// A simple test for this is to check if the protocol is just
		// one character long.  If so, it is not really a protocol at all.
		//
		if(protpos < 2)
		{
			protpos = 0;
		}
		else
		{
			protpos++; // increment past ":"
			//
			// If the protocol has changed from the context, then
			// reset our handler reference (if any)
			//
			if(StringUtils::CompareNoCase(m_protocol, protocol) != 0)
			{
				m_protocol = protocol;
				m_rpHandler.release();
			}
		}
	}
	else
	{
		protpos = 0;
	}

	if( (refpos = spec.find(QC_T("#"), 0)) != String::npos)
	{
		m_ref = spec.substr(refpos+1, spec.length()-(refpos+1));
	}
	else
	{
		refpos = spec.length();
	}

	String middleBit = spec.substr(protpos, refpos-protpos); 

	//
	// if no protocol has been provided then we assume must throw
	// an exception.  Sorry!
	//
	if(m_protocol.empty())
	{
		String errMsg = QC_T("no protocol: ");
		errMsg += spec;
		throw MalformedURLException(errMsg);
	}

	if(m_rpHandler.isNull())
	{
		m_rpHandler = rpFactory->createURLStreamHandler(m_protocol);
	}

	if(m_rpHandler.isNull())
	{
		static const String errMsg = QC_T("unknown protocol: ");
		throw MalformedURLException(errMsg + m_protocol);
	}
	else
	{
		m_rpHandler->parseURL(*this, middleBit);
	}
}

//==============================================================================
// URL::getPath
//
/**
   Returns the path for this URL.  The path consists of the file name
   part of the URL without any query information.

   For example, the path for the following URL is @a '/search'.

   @c http://www.google.com/search?q=xml

   @sa getFile()
*/
//==============================================================================
const String& URL::getPath() const
{
	return m_path;
}

//==============================================================================
// URL::getFile
//
/**
   Returns the file name for this URL.  The file name consists of
   the path plus the query (if present).

   For example, the file name for the following URL is @a '/search?q=xml'.

   @c http://www.google.com/search?q=xml

   @sa getPath()
*/
//==============================================================================
String URL::getFile() const
{
	if(!m_query.empty())
	{
		return m_path + String(QC_T("?")) + m_query;
	}
	else
	{
		return m_path;		
	}
}

//==============================================================================
// URL::getHost
//
/**
   Returns the host name part of the URL.

   Not all URLs contain a host name, but those that do specify the
   host as part of the authority segment which is contained
   between the @c '//' and the following @c '/' or @c '?' characters.

   The host name is a sub-string of the authority part, with user and port
   number information removed.

   For example, the following URL's host is @c www.google.com :-

   @c http://user:password@www.google.com:80/index.html
   @sa getAuthority()
   @sa getUserInfo()
   @sa getPort()
*/
//==============================================================================
const String& URL::getHost() const
{
	return m_host;
}

//==============================================================================
// URL::getAuthority
//
/**
   Returns the authority part of the URL.
*/
//==============================================================================
const String& URL::getAuthority() const
{
	return m_authority;
}

//==============================================================================
// URL::getPort
//
/**
   Returns the port number from the URL or -1 if no port number is present.

   The port number is usually contained within the authority part of the URL
   and is separated from the host by a colon character.
    
   For example, the following URL has a port number of 81:-

   @c http://www.acme.org:81
   @sa getAuthority()
*/
//==============================================================================
int URL::getPort() const
{
	return m_port;
}

//==============================================================================
// URL::getProtocol
//
/**
   Returns the protocol name (scheme part) of this URL.
*/
//==============================================================================
const String& URL::getProtocol() const
{
	return m_protocol;
}

//==============================================================================
// URL::getRef
//
/**
   Returns the reference part of this URL.  The reference contains all characters
   following the @c '#' (if any).
*/
//==============================================================================
const String& URL::getRef() const
{
	return m_ref;
}

//==============================================================================
// URL::getQuery
//
// Returns the portion of the file after (but not including) '?' which
// represents the start of a query string.
//==============================================================================
const String& URL::getQuery() const
{
	return m_query;
}

//==============================================================================
// URL::getUserInfo
//
/**
   Returns the UserInfo part of this URL.
   @sa getUserID()
   @sa getPassword()
*/
//==============================================================================
const String& URL::getUserInfo() const
{
	return m_userInfo;
}

//==============================================================================
// URL::getUserID
//
/**
   Returns the user identifier part of the UserInfo part of this URL.

   This method assumes that UserInfo is structured like this:

   @c <userid>:<password>
*/
//==============================================================================
String URL::getUserID() const
{
	size_t pwPos = m_userInfo.find(':');
	if(pwPos != String::npos)
	{
		return m_userInfo.substr(0, pwPos);
	}
	else
	{
		return m_userInfo;
	}
}

//==============================================================================
// URL::getPassword
//
/**
   Returns the password identifier part of the UserInfo part of this URL.

   This method assumes that UserInfo is structured like this:

   @c <userid>:<password>
*/
//==============================================================================
String URL::getPassword() const
{
	size_t pwPos = m_userInfo.find(':');
	if(pwPos != String::npos)
	{
		return m_userInfo.substr(pwPos+1);
	}
	else
	{
		return String();
	}
}

bool URL::operator==(const URL& rhs) const
{
	return equals(rhs);
}

bool URL::operator!=(const URL& rhs) const
{
	return !equals(rhs);
}

//==============================================================================
// URL::equals
//
/**
   Compares this URL with @c rhs.  Two URLs are considered equal if they
   refer to the same network file, which is determined by calling sameFile(),
   and they both have the same reference.

   @returns true if @c rhs refers to the same file and reference as this URL.

   @sa sameFile()
   @sa getRef()
*/
//==============================================================================
bool URL::equals(const URL& rhs) const
{
	if(!sameFile(rhs))
		return false;

	return (getRef() == rhs.getRef());
}

//==============================================================================
// URL::sameFile
//
/**
   Tests this URL against @c other to see if they refer to the same file.

   The test is delegated to the URLStreamHandler for this URL.
*/
//==============================================================================
bool URL::sameFile(const URL& other) const
{
	if(m_rpHandler)
		return m_rpHandler->sameFile(*this, other);
	else
		return false;
}

//==============================================================================
// URL::toExternalForm
//
/**
   Converts this URL into a String in URL format.

   This might be useful if you have constructed a URL programatically
   and now wish to output it to another process.
*/
//==============================================================================
String URL::toExternalForm() const
{
	if(m_rpHandler)
		return m_rpHandler->toExternalForm(*this);
	else
		return QC_T("");
}

//==============================================================================
// URL::openConnection
//
/**
   Returns a URLConnection which is suitable for the communication
   protocol designated by this URL.
*/
//==============================================================================
AutoPtr<URLConnection> URL::openConnection() const
{
	if(m_rpHandler)
		return m_rpHandler->openConnection(*this);
	else
		return 0;
}

//==============================================================================
// URL::openStream
//
/**
   A convenience method that opens a connection to the network resource
   identified by this URL and returns an InputStream for the resource
   content.

   This is exactly the same as coding the following:-
   
   @code
   openConnection()->getInputStream()
   @endcode

   Note that the URLConnection object is not accessible once the InputStream 
   has been returned.  If you require access to the URLConnection
   use the openConnection() method.

   @returns an InputStream for reading the URL's resource.
   @throws IOException if an error occurs connecting to the resource

   @sa openConnection()
   @sa URLConnection::getInputStream()
*/
//==============================================================================
AutoPtr<InputStream> URL::openStream() const
{
	if(m_rpHandler)
		return openConnection()->getInputStream();
	else
		return 0;
}

//==============================================================================
// URL::getStreamHandler
//
/**
   Returns the URLStreamHandler associated with this URL, or null if
   no protocol or stream handler has been provided.
*/
//==============================================================================
AutoPtr<URLStreamHandler> URL::getStreamHandler() const
{
	return m_rpHandler;
}

QC_NET_NAMESPACE_END

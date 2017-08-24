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
/**
	@class qc::net::URLStreamHandler
	
	@brief An @a abstract base class for dealing with the protocol-specific aspects
	of URL handling.

    Instances of URLStreamHandler are created by the registered
	URLStreamHandlerFactory.

	@sa URLStreamHandlerFactory
*/
//==============================================================================

#include "URLStreamHandler.h"
#include "InetAddress.h"
#include "UnknownHostException.h"
#include "URL.h"

#include "QcCore/base/NumUtils.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/util/StringTokenizer.h"
#include "QcCore/io/File.h"

QC_NET_NAMESPACE_BEGIN

using io::File;
using util::StringTokenizer;

//=============================================================================
// URLStreamHandler::parseURL
// 
/**
   Extracts the component parts of the URL from the passed string and sets the
   values in urlRet accordingly.

   This base-class version assumes an hierarchical HTTP-like URL syntax.

   The caller (URL constructor) will already have stripped the protocol
   and #ref parts from the specification string.
*/
//
// For our purposes, we will use the following productions:
//
// URL ::= (protocol ':')? ('//' Authority (('/' Path ('#' ref)?)?
// Authority ::= (user(:password)?'@')? host (':' port)?
// Path ::= file ('?' query)
//
// This is just an approximation to the URL syntax as described in RFC1808 and
// later clarified in RFC 2396 (URI)
//
//=============================================================================
void URLStreamHandler::parseURL(URL& urlRet, const String& spec) const
{
	// 
	// Protocol and Ref have already been set by URL
	//
	String protocol = urlRet.getProtocol();
	String ref = urlRet.getRef();

	//
	// Default to the current values
	//
	String authority = urlRet.getAuthority(); 
	String userInfo = urlRet.getUserInfo();
	String host = urlRet.getHost();
	int port = urlRet.getPort();
	String path = urlRet.getPath();

	//
	// Query is reset every time, we do not inherit it from the
	// context URL
	//
	String query;

	size_t currpos = 0;
	size_t findpos;

	//
	// If the string starts with "//" then we have an authority, otherwise we are
	// straight into the filename
	//
	if( (findpos = spec.find(QC_T("//"), currpos)) != String::npos)
	{
		currpos = findpos+2;

		//
		// If we have an authority, then the path from the
		// context URL is irrelevant
		//
		path.erase();

		//
		// Locate the end of the authority part
		//
		if( (findpos = spec.find(QC_T("/"), currpos)) != String::npos)
		{
			authority = spec.substr(currpos, findpos-currpos);
		}
		else
		{
			authority = spec.substr(currpos);
		}
		currpos = findpos;

		//
		// Parse the contents of the Authority part
		// 
		// 1. Check for user information
		//
		size_t hostpos=0;
		if( (findpos = authority.find(QC_T("@"), 0)) != String::npos)
		{
			userInfo = authority.substr(0, findpos);
			hostpos = findpos+1; // point past the "@"
		}
		else
		{
			// No user info in this authority, so reset
			// the contents from the context URL to empty
			userInfo.erase();
		}

		//
		// 2. Check for port information
		//
		if( (findpos = authority.find(QC_T(":"), hostpos)) != String::npos)
		{
			host = authority.substr(hostpos, findpos-hostpos);
			const String& strPort = authority.substr(findpos+1);
			port = NumUtils::ToInt(strPort);
		}
		else
		{
			host = authority.substr(hostpos);
			port = -1;
		}
	}

	//
	// And now, the path...
	//
	// 1. Locate the query part (if any)
	//
	String specPath;

	if(currpos != String::npos)
	{
		if( (findpos = spec.find(QC_T("?"), currpos)) != String::npos)
		{
			query = spec.substr(findpos+1);
			specPath = spec.substr(currpos, findpos-currpos);
		}
		else
		{
			specPath = spec.substr(currpos);
		}
	}

	//
	// Call our path resolution function
	//
	path = parsePath(path, specPath);

	//
	// If we have a host, then the path must start with '/'
	//
	if(!host.empty() && !path.empty() && path[0] != '/')
	{
		path.insert((String::size_type)0, 1, '/');
	}

	setURL(urlRet,
	       urlRet.getProtocol(),
	       host,
	       port,
	       authority,
	       userInfo,
	       path,
	       query,
	       urlRet.getRef());
}

//==============================================================================
// URLStreamHandler::parsePath
//
/**
   Protected helper function called by ParseURL() to process the path name of a URL.

   The base class implementation performs the following normalization of filenames:
   - '\' characters are translated to '/'
   - when combining a relative path from @c path, the path is compressed
     by removing redundant directory names
   @param context the path from the context URL (if any)
   @param path the path being parsed
   @returns the normalized path
*/
//==============================================================================
String URLStreamHandler::parsePath(const String& context, const String& path) const
{
	String ret = path;
	StringUtils::ReplaceAll(ret, '\\', QC_T("/"));

	if(ret.size() && ret[0] == '/')
	{
		// absolute path
	}
	else
	{
		size_t seppos;
		String base = context;
		if( (seppos = base.find_last_of('/')) != String::npos)
		{
			//
			// We have a context path onto which we must
			// append our relative path.
			// The combined path must be normalized
			// to remove redundant separator sequences
			//
			ret = base.substr(0, seppos+1) + ret;

			StringTokenizer tokenizer(ret, QC_T("/"), true);
			ret.erase();

			const CharType sep = '/';
			while(tokenizer.hasMoreTokens())
			{
				const String& token = tokenizer.nextToken();

				if(token[0] == '/')
				{
					ret += token;
				}
				else if(token == QC_T("."))
				{
					tokenizer.nextToken();
				}
				else if(token == QC_T(".."))
				{
					//
					// If the first character of ret is "/" this indicates
					// the root, so we do not want to attempt to step before
					// it
					//
					size_t pos = ret.length() > 1
					           ? ret.find_last_of(sep, ret.length()-2)
							   : String::npos;

					if(pos != String::npos && ret[pos+1] != '.')
					{
						ret.erase(pos+1);
						tokenizer.nextToken();
					}
					else if(!ret.empty() && ret[0] != '/' && ret[0] != '.')
					{
						ret.erase();
						tokenizer.nextToken();
					}
					else
					{
						// This is not an error.  We have a ".." with no preceding
						// name (except perhaps the root).
						// We just go back further, creating a relative path
						ret += token;
						ret += tokenizer.nextToken();
					}
				}
				else
				{
					ret += token;
					ret += tokenizer.nextToken();
				}
			}
		}
	}
	return ret;
}

//==============================================================================
// URLStreamHandler::setURL
//
/**
   Protected helper function which provides update access to the individual
   components of a URL.
*/
//==============================================================================
void URLStreamHandler::setURL(URL& url,
                              const String& protocol,
                              const String& host,
                              int port,
                              const String& authority,
                              const String& userInfo,
                              const String& path,
                              const String& query,
                              const String& ref) const
{
	url.set(protocol, host, port, authority, userInfo, path, query, ref);
}

//==============================================================================
// URLStreamHandler::toExternalForm
//
/**
   Converts a URL into a valid string representation.
   @param url the URL to convert 
*/
//==============================================================================
String URLStreamHandler::toExternalForm(const URL& url) const
{
	String ret = url.getProtocol();
	ret += QC_T(":");
	
	if(!url.getAuthority().empty())
	{
		ret += QC_T("//");
		ret += url.getAuthority();
	}

	ret += url.getFile();

	if(!url.getRef().empty())
	{
		ret += QC_T("#");
		ret += url.getRef();
	}

	return ret;
}

//==============================================================================
// URLStreamHandler::sameFile
//
/**
   Compares two URLs to see if they relate to the same file.

   The base class implementation compares the protocol, host, port
   and filename components.
   
   Host names are compared by the hostsEqual() method.  Port numbers are
   considered equal when they have the same value,
   or when one isn't specified and the other has the default port number
   specified.
   @param first the first URL to compare
   @param second the second URL to compare
   @returns true if @c first and @c second refer to the same network file.
*/
//==============================================================================
bool URLStreamHandler::sameFile(const URL& first, const URL& second) const
{
	// 1. compare the protocol
    if(StringUtils::CompareNoCase(first.getProtocol(), second.getProtocol()) != 0)
	{
		return false;
	}

	// 2. compare the hosts
	if(!hostsEqual(first, second))
	{
		return false;
	}

	// 3. compare the files
	if(first.getFile() != second.getFile())
	{
	    return false;
	}

	// Compare the ports.
	int firstPort = (first.getPort() != -1) ? first.getPort() : getDefaultPort();
	int secondPort = (second.getPort() != -1) ? second.getPort() : getDefaultPort();
	if(firstPort != secondPort)
	{
		return false;
	}

	return true;
}

//==============================================================================
// URLStreamHandler::hostsEqual
//
/**
   Compares the hosts from two URLs for equality.  Two hostnames
   are considered the same if they compare equal using
   a case-insensitive comparison, or if they both resolve to the same
   Internet address.

   @param first the first URL to compare
   @param second the second URL to compare
   @returns true if @c first and @c second refer to the same network host.
*/
//==============================================================================
bool URLStreamHandler::hostsEqual(const URL& first, const URL& second) const
{
	if(StringUtils::CompareNoCase(first.getHost(), second.getHost()) == 0)
	{
		return true;
	}

	try
	{
		AutoPtr<InetAddress> rpfirst = InetAddress::GetByName(first.getHost());
		AutoPtr<InetAddress> rpsecond = InetAddress::GetByName(second.getHost());
		if(*rpsecond.get() == *rpfirst.get())
		{
			return true;
		}
	}
	catch(UnknownHostException& /*e*/)
	{
	}

	return false;
}

//==============================================================================
// URLStreamHandler::getDefaultPort
//
/**
   Returns the default port number which servers for the protocol usually
   listen on.  This is used when comparing URLs where one has a port
   and the other doesn't to see if they would actually contact the same
   remote server.
   @returns the default port number for the protocol (e.g. port 80 for HTTP)
*/
//==============================================================================
int URLStreamHandler::getDefaultPort() const
{
	return -1;
}

#ifdef QC_DOCUMENTATION_ONLY
//=============================================================================
//
// Documentation for pure virtual methods follows:
//
//=============================================================================

//==============================================================================
// URLStreamHandler::openConnection
//
/**
   Opens a URLConnection to the resource identified by a URL.

   This is a protected member. Applications should not call this member directly,
   rather they should make use of the URL class.

   @param url the URL to connect to
   @returns a URLConnection instance for the URL.
   @throws IOException if an I/O error occurs while opening the connection
   @sa URL::openConnection()
*/
//==============================================================================
AutoPtr<URLConnection> URLStreamHandler::openConnection(const URL& url) const;

#endif //QC_DOCUMENTATION_ONLY

QC_NET_NAMESPACE_END

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
// This class implements the URLConnection interface for FTP URLs.
//
// For convenience to client applications that wish to view HTTP/FTP
// URLs as abstract resources with similar characteristics, this class
// interprets the HTTP/1.1 Range header as an FTP restart request.
//
//==============================================================================

#include "FtpURLConnection.h"
#include "FtpClient.h"
#include "URL.h"
#include "URLDecoder.h"
#include "Socket.h"
#include "InetAddress.h"
#include "MalformedURLException.h"
#include "ProtocolException.h"

#include "QcCore/base/StringUtils.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/base/debug.h"
#include "QcCore/io/FileNotFoundException.h"
#include "QcCore/util/StringTokenizer.h"

QC_NET_NAMESPACE_BEGIN

using io::FileNotFoundException;
using util::StringTokenizer;

//==============================================================================
// FtpURLConnection::FtpURLConnection
//
// Construct a FtpURLConnection using the passed URL and default
// (or global) settings.
//==============================================================================
FtpURLConnection::FtpURLConnection(const URL& url) :
	BasicURLConnection(url),
	m_bDirectoryReqd(false),
	m_restartPoint(0)
{
}

//=============================================================================
// FtpURLConnection::connect
// 
// Creates a FtpClient to service the connection to the remote host
//
//=============================================================================
void FtpURLConnection::connect()
{
	if(!isConnected())
	{
		//
		// Determine if this is an anonymous or specified user.
		// If the URL contains userinfo, use that.
		//

		String user = QC_T("anonymous");
		String password = QC_T("quickcpp@");

		if(!getURL().getUserInfo().empty())
		{
			user = URLDecoder::RawDecode(getURL().getUserID());
			password = URLDecoder::RawDecode(getURL().getPassword());
		}

		m_rpFtpClient = new FtpClient;

		// don't wait for longer than 60 seconds - it's not really justified!
		const size_t timeout = 60000;

		m_rpFtpClient->connect(getURL().getHost(), getURL().getPort(), timeout);
		m_rpFtpClient->login(user, password);
		m_rpFtpClient->setDataConnectionTimeout(timeout);

		//
		// It's a good idea to be firewall-friendly and use passive data connections
		//
		m_rpFtpClient->setDataConnectionType(FtpClient::Passive);

		//
		// Now is a good time to massage the URL's filename
		//
		// This includes the following steps:
		// 1) unescape the entire path.  URL encoding such as "%20" is changed to " "
		// 2) check for a TYPE parameter
		// 3) isolate the filename from the path (some FTP servers won't serve up
		//    qualified paths
		// 4) update the path to remove the leading "/" (if any).  The URL syntax
		//    does not allow us to specify a path without the leading "/", but it
		//    is rare that a FTP url would be relative to the root filesystem.
		//
		// (1) Unescape the filename from the URL
		//
		String filename = URLDecoder::RawDecode(getURL().getFile());

		// (2) FTP URLS have an optional type parameter after the filename
		// file ::= file [;type=TYPE]?
		// RFC1738 states that TYPE must be "a" (ascii) "i" (image) or "d" (directory)
		// 
		// The default transmission type is binary
		//
		const size_t typePos = filename.find(QC_T(";type="));

		if(typePos != String::npos)
		{
			String strType = filename.substr(typePos+6);
			
			if(StringUtils::CompareNoCase(strType, QC_T("a")) == 0)
			{
				m_rpFtpClient->setTransferType(FtpClient::Ascii);
			}
			else if(StringUtils::CompareNoCase(strType, QC_T("i")) == 0)
			{
				m_rpFtpClient->setTransferType(FtpClient::Binary);
			}
			else
			{
				m_rpFtpClient->setTransferType(FtpClient::Ascii);
				m_bDirectoryReqd = true;
			}

			filename = filename.substr(0, typePos);
		}
		else
		{
			m_rpFtpClient->setTransferType(FtpClient::Binary);
		}

		//
		// (3) isolate the filename from the path
		//
		size_t pathPos = filename.find_last_of(QC_T("/"));
		if(pathPos != String::npos)
		{
			m_pathname = filename.substr(0, pathPos);
			if(pathPos < (filename.length() - 1))
			{
				m_filename = filename.substr(pathPos+1);
			}
		}
		else
		{
			m_filename = filename;
		}

		//
		// (4) update the path to remove the leading "/" (if any). 
		//
		if(m_pathname.length() && m_pathname.at(0) == '/')
		{
			m_pathname = m_pathname.substr(1);
		}

		setConnected(true);
	}
}

//==============================================================================
// FtpURLConnection::getInputStream
//
// Returns a (ref-counted) ptr to the FtpClient's InputStream.
//==============================================================================
AutoPtr<InputStream> FtpURLConnection::getInputStream()
{
	connect();

	if(!m_rpInputStream)
	{
		try
		{
			if(m_rpOutputStream)
			{
				throw IOException(QC_T("already opened for output"));
			}

			if(!getDoInput())
			{
				throw ProtocolException(QC_T("URLConnection not enabled for input"));
			}

			//
			// change the FTP servers working directory to the appropriate place
			//
			changeDirectory();

			//
			// If the URL has specified that a directory listing is required
			// that's what we'll deliver.  
			// RFC1738 states that a NLST should be used, but this does not seem
			// appropriate when a filename has been specified as part of the URL
			// (the NLST doesn't like it), so we use LIST instead.
			//
			if(m_bDirectoryReqd)
			{
				if(m_filename.empty())
				{
					m_rpInputStream = m_rpFtpClient->listNames(m_filename);
				}
				else
				{
					m_rpInputStream = m_rpFtpClient->listDetails(m_filename);
				}
			}
			//
			// For a file transfer, we can set the content-length header, but only
			// if we are performing a binary (image) transfer, otherwise the
			// NVT encoding may change the number of bytes transmitted.
			//
			else
			{
				if(m_rpFtpClient->getTransferType() == FtpClient::Binary)
				{
					size_t fileSize = m_rpFtpClient->getFileSize(m_filename);
					setHeaderField(QC_T("content-length"), NumUtils::ToString(fileSize-m_restartPoint));
					if(m_restartPoint!=0)
					{
						// Produce a header in the form:
						// 	 Content-Range: bytes 4095-6394/6394
						String rangeHdr = QC_T("bytes ");
						rangeHdr+=NumUtils::ToString(m_restartPoint) + QC_T("-");
						rangeHdr+=NumUtils::ToString(fileSize-1) + QC_T("/");
						rangeHdr+=NumUtils::ToString(fileSize);
						setHeaderField(QC_T("Content-Range"), rangeHdr);
					}
					m_rpInputStream = m_rpFtpClient->retrieveFile(m_filename, m_restartPoint);
				}
				else
				{
					m_rpInputStream = m_rpFtpClient->retrieveFile(m_filename);
				}
			}
		}
		catch(IOException& /*e*/)
		{
		}

		if(!m_rpInputStream)
		{
			//
			// Use our representation of the filename when throwing the
			// FileNotFound exception - it gives a better indication of what has happened
			// wrt escaping, TYPE= removal etc.
			//
			String filename = m_pathname;
			if(filename.length())
				filename += QC_T("/");
			filename += m_filename;
			throw FileNotFoundException(filename);
		}
	}

	return m_rpInputStream;
}

//==============================================================================
// FtpURLConnection::getOutputStream
//
// Returns a (ref-counted) ptr to the FtpClient's OutputStream.
//==============================================================================
AutoPtr<OutputStream> FtpURLConnection::getOutputStream()
{
	if(!getDoOutput())
	{
		throw ProtocolException(QC_T("URLConnection not enabled for output"));
	}

	if(m_rpInputStream)
	{
		throw IOException(QC_T("already opened for input"));
	}

	connect();

	if(!m_rpOutputStream)
	{
		if(m_filename.empty())
		{
			throw IOException(QC_T("filename missing"));
		}

		try
		{
			changeDirectory();
		}
		catch(Exception& /*e*/)
		{
			throw FileNotFoundException(m_pathname);
		}

		m_rpOutputStream = m_rpFtpClient->storeFile(m_filename);
	}

	return m_rpOutputStream;
}

//==============================================================================
// FtpURLConnection::changeDirectory
//
//==============================================================================
void FtpURLConnection::changeDirectory() const
{
	StringTokenizer tokenizer(m_pathname, QC_T("/"));
	while(tokenizer.hasMoreTokens())
	{
		if(!m_rpFtpClient->changeWorkingDirectory(tokenizer.nextToken()))
		{
			throw IOException(QC_T("unable to change directory"));
		}
	}
}

//==============================================================================
// FtpURLConnection::setRequestProperty
//
//==============================================================================
void FtpURLConnection::setRequestProperty(const String& name, const String& value)
{
	if(StringUtils::CompareNoCase(name, QC_T("Range"))==0)
	{
		//bytes=2341-
		if(value.substr(0,6) == QC_T("bytes=") && value[value.size()-1]==QC_T('-'))
		{
			m_restartPoint = NumUtils::ToLong(value.substr(6,value.size()-7));
		}
	}
}

//==============================================================================
// FtpURLConnection::getRequestProperty
//
//==============================================================================
String FtpURLConnection::getRequestProperty(const String& name) const
{
	if(m_restartPoint>0 && StringUtils::CompareNoCase(name, QC_T("Range"))==0)
	{
		String ret = QC_T("bytes=");
		ret += NumUtils::ToString(m_restartPoint) + QC_T("-");
		return ret;
	}
	else
	{
		return String();
	}
}

QC_NET_NAMESPACE_END


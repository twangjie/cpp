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

#include "FileURLConnection.h"
#include "ProtocolException.h"
#include "URL.h"
#include "URLDecoder.h"

#include "QcCore/base/StringUtils.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/io/IOException.h"
#include "QcCore/io/FileInputStream.h"
#include "QcCore/io/FileOutputStream.h"
#include "QcCore/io/File.h"

QC_NET_NAMESPACE_BEGIN

using namespace io;

//==============================================================================
// FileURLConnection::FileURLConnection
//
// Construct a FileURLConnection with a URL.  As with all URLConnections,
// construction is a two-stage process.  It is not complete until connect()
// has been called.
//==============================================================================
FileURLConnection::FileURLConnection(const URL& url) :
	BasicURLConnection(url)
{
}

//==============================================================================
// FileURLConnection::connect
//
// Perform the second-stage of construction, actually connnecting to the url
// that was provided to the constructor.
//
// For file: URLs this is a no-op.  We don't yet know whether the file
// URL is going to be used for reading or writing, so there is little
// point in checking if the file exists.  This will be done
// in getInputStream/getOutputStream.
//==============================================================================
void FileURLConnection::connect()
{
	if(!isConnected())
	{
		setConnected(true);
	}
}

//==============================================================================
// FileURLConnection::getInputStream
//
// Returns a (ref counted) ptr to the FileInputStream.
//
// If the url does not points at a directory rather than a file the JDK
// equivalent tries to be very helpful by creating an html listing of the
// directory.  This is probably a throw-back to the HotJava browser
// and has no useful place in QuickCPP.
//
// Additionally, we try to be helpful by unescaping URL escape sequences.
// This may be overkill, but it is quite useful because it makes relative
// URLs behave the same locally as they would over an HTTP connection.
//==============================================================================
AutoPtr<InputStream> FileURLConnection::getInputStream()
{
	connect();

	if(!m_rpInputStream)
	{
		if(m_rpOutputStream)
		{
			throw IOException(QC_T("already opened for output"));
		}

		if(!getDoInput())
		{
			throw ProtocolException(QC_T("URLConnection not enabled for input"));
		}

		File file(URLDecoder::RawDecode(getURL().getFile()));
		m_rpInputStream = new FileInputStream(file);
		String strLen = NumUtils::ToString(file.length());
		setHeaderField(QC_T("content-length"), strLen);
		DateTime modDate = file.lastModified();
		//  Format as RFC 822 eg: Thu, 25 Oct 2001 20:03:28 GMT
		setHeaderField(QC_T("last-modified"), modDate.Format(QC_T("%a, %d %b %Y %H:%M:%S GMT")));
	}

	return m_rpInputStream;
}

//==============================================================================
// FileURLConnection::getOutputStream
//
// Returns a (ref counted) ptr to the FileOutputStream.
//==============================================================================
AutoPtr<OutputStream> FileURLConnection::getOutputStream()
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
		File file(URLDecoder::RawDecode(getURL().getFile()));
		m_rpOutputStream = new FileOutputStream(file);
	}

	return m_rpOutputStream;
}

QC_NET_NAMESPACE_END

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

#include "FileURLStreamHandler.h"
#include "FileURLConnection.h"

#include "QcCore/base/StringUtils.h"
#include "QcCore/io/File.h"

QC_NET_NAMESPACE_BEGIN

using io::File;

//==============================================================================
// FileURLStreamHandler::openConnection
//
//==============================================================================
AutoPtr<URLConnection> FileURLStreamHandler::openConnection(const URL& url) const
{
	return new FileURLConnection(url);
}

//==============================================================================
// FileURLStreamHandler::parsePath
//
// Specialized Helper function to determine the filename for a URL given a
// context and a (possibly) relative filename.
// This has been overloaded for the file: protocol because the URL for
// a file protocol can be a lot more relaxed and more system dependent
//==============================================================================
String FileURLStreamHandler::parsePath(const String& context, const String& path) const
{
	String ret = URLStreamHandler::parsePath(context, path);

#if defined(WIN32)

	// There is a good deal of confusion regarding what constitutes a legal
	// file: URL, especially on Windows platforms.
	// The following are all widely used: file:c:\rob    [1]
	//                                    file:///c:\rob [2]
	//                                    file://c:\rob  [3]
	//                                    file:///rob    [4]
	//                                    file:////<host>/<share>/ [5]
	// Case [3] appears invalid according to RFC1738 because the file: scheme
	// allows an optional host, which the "c:" could be interpreted as.
	// Case [1] is convenient because the resultant filename is directly
	// usable.
	// Case [4] is ambiguous because it may relate to a file in the
	// current directory of the root directory for the current drive.  In experiments
	// it seems JDK treats case [4] as refering to the root directory.  This is
	// reasonable as it mirrors the behaviour under UNIX, but means that relative
	// file URLs must be written as file:<path> without any "//".
	// 
	// 
	// To cater for case [2], we need to remove the leading "/" before
	// the local file system is asked to open it.
	if(ret.length() > 2 && ret[0] == '/' && ret[2] == ':')
	{
		ret.erase(0,1);
	}

#endif

	return ret;
}

//==============================================================================
// FileURLStreamHandler::toExternalForm
//
// Custom implementation for the @c file protocol.
//==============================================================================
String FileURLStreamHandler::toExternalForm(const URL& url) const
{
	String ret = url.getProtocol();
	ret += QC_T(":");
	
	const String& file = url.getFile();

	if((!url.getAuthority().empty()) || file.size() && file[0] == '/')
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

QC_NET_NAMESPACE_END


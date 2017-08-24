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

#ifndef QC_NET_FileURLConnection_h
#define QC_NET_FileURLConnection_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "BasicURLConnection.h"

QC_NET_NAMESPACE_BEGIN

class QC_NET_PKG FileURLConnection : public BasicURLConnection
{
	friend class FileURLStreamHandler;

public:

	virtual void connect();
	virtual AutoPtr<InputStream> getInputStream();
	virtual AutoPtr<OutputStream> getOutputStream();

protected: // created by URLStreamHandler
	FileURLConnection(const URL& url);

private:
	AutoPtr<InputStream>  m_rpInputStream;
	AutoPtr<OutputStream> m_rpOutputStream;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_FileURLConnection_h

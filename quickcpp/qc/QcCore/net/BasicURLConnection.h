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

#ifndef QC_NET_BasicURLConnection_h
#define QC_NET_BasicURLConnection_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "URLConnection.h"
#include "MimeHeaderSequence.h"

QC_NET_NAMESPACE_BEGIN

class QC_NET_PKG BasicURLConnection : public URLConnection
{
public:
	BasicURLConnection(const URL& url);

	virtual String getHeaderField(const String& name);
	virtual String getHeaderField(size_t index);
	virtual String getHeaderFieldKey(size_t index);
	virtual size_t getHeaderFieldCount();
	
	virtual void setHeaderField(const String& key, const String& value);

private:
	AutoPtr<MimeHeaderSequence> m_rpResponseHeaders;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_BasicURLConnection_h

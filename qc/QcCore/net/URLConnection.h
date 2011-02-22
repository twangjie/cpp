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

#ifndef QC_NET_URLConnection_h
#define QC_NET_URLConnection_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "URL.h"
#include "QcCore/util/DateTime.h"
#include <vector>

QC_NET_NAMESPACE_BEGIN

using util::DateTime;

class QC_NET_PKG URLConnection : public virtual ManagedObject
{
public:
	URLConnection(const URL& url);

	virtual void connect() = 0;

	virtual String getContentEncoding();
	virtual long getContentLength();
	virtual String getContentType();
	virtual DateTime getDate();
	virtual DateTime getLastModified();
	
	virtual AutoPtr<InputStream> getInputStream() = 0;
	virtual AutoPtr<OutputStream> getOutputStream() = 0;

	virtual String getHeaderField(const String& name) = 0;
	virtual String getHeaderField(size_t index) = 0;
	virtual String getHeaderFieldKey(size_t index) = 0;
	virtual size_t getHeaderFieldCount() = 0;
	
	virtual void setRequestProperty(const String& name, const String& value);
	virtual String getRequestProperty(const String& name) const;

	virtual DateTime getHeaderFieldDate(const String& name);
	virtual long getHeaderFieldLong(const String& name, long defaultValue);
	virtual const URL& getURL() const;

	void setUseCaches(bool bUseCaches);
	bool getUseCaches() const;

	bool getDoInput() const;
	void setDoInput(bool bEnable);
	bool getDoOutput() const;
	void setDoOutput(bool bEnable);

	bool isConnected() const;

	static void SetDefaultUseCaches(bool bUseCaches);
	static bool GetDefaultUseCaches();

protected:
	void setConnected(bool bConnected);
	void resetHeaderFields();
	void setURL(const URL& url);

private:
	bool m_bConnected;
	std::vector<std::pair<String, String> > m_headerFields;
	URL m_url;
	bool m_bUseCaches;
	enum TriState {TriUnspecified, TriTrue, TriFalse};
	TriState m_doInput;
	TriState m_doOutput;

private: // static members
	static bool s_bUseCachesDefault;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_URLConnection_h


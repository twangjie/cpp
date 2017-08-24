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

#ifndef QC_NET_MimeHeaderSequence_h
#define QC_NET_MimeHeaderSequence_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include <utility>
#include <vector>

#include "QcCore/io/Writer.h"

QC_NET_NAMESPACE_BEGIN

using io::Writer;

class QC_NET_PKG MimeHeaderSequence : public virtual QCObject
{
	friend class MimeHeaderParser;

public:

	String getHeader(const String& key) const;
	String getHeader(size_t index) const;
	String getHeaderKey(size_t index) const;
	int findHeader(const String& key) const;
	bool containsHeader(const String& key) const;
	size_t size() const;
	void clear();

	void setHeaderExclusive(const String& key, const String& value);
	void insertHeader(const String& key, const String& value);
	bool setHeaderIfAbsent(const String& key, const String& value); 
	void removeAllHeaders(const String& key);

	void writeHeaders(Writer* pWriter);

private:
	typedef std::pair<String, String> HeaderFieldEntry;
	typedef std::vector<HeaderFieldEntry> HeaderFieldVector;
	HeaderFieldVector m_headerFields;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_MimeHeaderSequence_h

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

#ifndef QC_SAX_InputSource_h
#define QC_SAX_InputSource_h

#ifndef QC_SAX_defs_h
#include "defs.h"
#endif //QC_SAX_defs_h

#include "QcXml/xml/XMLInputSource.h"

QC_SAX_NAMESPACE_BEGIN

using xml::XMLInputSource;

class QC_SAX_PKG InputSource : public virtual ManagedObject

#ifndef QC_DOCUMENTATION_ONLY
                               ,private XMLInputSource
#endif //QC_DOCUMENTATION_ONLY

{
	friend class SAXParser;

public:

	InputSource();
	InputSource(InputStream* pByteStream);
	InputSource(Reader* pCharacterStream);
	InputSource(const String& systemId);

	AutoPtr<InputStream> getByteStream() const;
	AutoPtr<Reader> getCharacterStream() const;
	String getSystemId() const;
	String getPublicId() const;

	void setCharacterStream(Reader* pCharacterStream);
	void setByteStream(InputStream* pByteStream);
	void setSystemId(const String& systemId);
	void setPublicId(const String& publicId);

	String getEncoding() const;
	void setEncoding(const String& encoding);
};

QC_SAX_NAMESPACE_END

#endif //QC_SAX_InputSource_h

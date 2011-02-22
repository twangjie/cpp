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

#ifndef QC_SAX_LocatorImpl_h
#define QC_SAX_LocatorImpl_h

#include "defs.h"
#include "Locator.h"

QC_SAX_NAMESPACE_BEGIN

class QC_SAX_PKG LocatorImpl : public Locator
{
public:
	LocatorImpl();
	LocatorImpl(const Locator& rhs);

	// Locator interface...
	virtual long getColumnNumber() const;
	virtual long getLineNumber() const;
	virtual String getPublicId() const;
	virtual String getSystemId() const;

	// Additional methods to update the Locator's state
	void setColumnNumber(long columnNumber);
	void setLineNumber(long lineNumber);
	void setPublicId(const String& publicId);
	void setSystemId(const String& systemId);

private:
	long m_colNo;
	long m_lineNo;
	String m_publicId;
	String m_systemId;
};

QC_SAX_NAMESPACE_END

#endif //QC_XML_LocatorImpl_h

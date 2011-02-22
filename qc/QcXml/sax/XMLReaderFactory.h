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
// Class: XMLReaderFactory
// 
//==============================================================================

#ifndef QC_SAX_XMLReaderFactory_h
#define QC_SAX_XMLReaderFactory_h

#ifndef QC_SAX_defs_h
#include "defs.h"
#endif //QC_SAX_defs_h

QC_SAX_NAMESPACE_BEGIN

class XMLReader;

class QC_SAX_PKG XMLReaderFactory : public virtual ManagedObject
{
public:

	static AutoPtr<XMLReaderFactory> GetInstance();
	static void SetInstance(XMLReaderFactory* pFactory);

	static AutoPtr<XMLReader> CreateXMLReader();

protected:
	virtual AutoPtr<XMLReader> doCreate() const;

private:
	static XMLReaderFactory* QC_MT_VOLATILE s_pInstance;
};

QC_SAX_NAMESPACE_END

#endif //QC_SAX_XMLReaderFactory_h

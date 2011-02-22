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
// Class: ParserFactory
// 
// Overview
// --------
// Returns an instance of an XML Parser.
//
//==============================================================================

#ifndef QC_XML_ParserFactory_h
#define QC_XML_ParserFactory_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

QC_XML_NAMESPACE_BEGIN

class Parser;

class QC_XML_PKG ParserFactory : public virtual ManagedObject
{
public:

	// Construction
	ParserFactory();
	virtual ~ParserFactory();

	static AutoPtr<ParserFactory> GetInstance();
	static void SetInstance(ParserFactory* pFactory);

	static AutoPtr<Parser> CreateXMLParser();

protected:

	virtual AutoPtr<Parser> doCreate() const;

private:
	static ParserFactory* QC_MT_VOLATILE s_pInstance;
};

QC_XML_NAMESPACE_END

#endif //QC_XML_ParserFactory_h


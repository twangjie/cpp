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
/**
	@namespace qc::sax

	Provides a standardized SAX 2.0 interface to the @QuickCPP XML parser.

    <p><b>SAX</b> is an acronym standing for "Simple API for XML".  It was originally 
	a Java API and has become the "de facto" standard for parsing XML using Java.</p>

    <p>There is not a recognized standard C++ representation for SAX.  However,
    the @QuickCPP SAX implementation is able to follow the Java API very closely
	by using the @QuickCPP framework facilities for Unicode, networking, IO
	and object lifetime management.</p>

    <p>Further details about SAX may be obtained from the SAX project
	home page: <a href="http://www.saxproject.org/">www.saxproject.org</a>.</p>
*/
//==============================================================================

#ifndef QC_SAX_DEFS_h
#define QC_SAX_DEFS_h

#include "QcCore/base/gendefs.h"
#include "QcCore/base/QCObject.h"
#include "QcCore/base/AutoPtr.h"
#include "QcCore/base/String.h"
#include "QcCore/io/InputStream.h"
#include "QcCore/io/Reader.h"

#include "QcXml/xml/defs.h"

#define QC_SAX_PKG QC_QCXML_PKG
#define QC_SAX_INCLUDE_INLINES QC_QCXML_INCLUDE_INLINES
#define QC_SAX_NAMESPACE_BEGIN QC_SUB_NAMESPACE_BEGIN(sax)
#define QC_SAX_NAMESPACE_END   QC_SUB_NAMESPACE_END(sax)

//
// import types into sax namespace
//
QC_SAX_NAMESPACE_BEGIN

using io::InputStream;
using io::Reader;

QC_SAX_NAMESPACE_END

#endif //QC_SAX_DEFS_h

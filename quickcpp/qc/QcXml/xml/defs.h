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
	@namespace qc::xml

	Contains classes representing the native interface to
	the @QuickCPP XML parser.  

	<p>Applications that do not require the additional
	features provided by the native interface are encouraged to use the
	standard interface provided by the qc::sax namespace.</p>
*/
//==============================================================================

#ifndef QC_XML_DEFS_h
#define QC_XML_DEFS_h

#include "QcCore/base/gendefs.h"
#include "QcCore/base/Character.h"
#include "QcCore/base/QCObject.h"
#include "QcCore/base/AutoPtr.h"
#include "QcCore/base/String.h"
#include "QcCore/io/InputStream.h"
#include "QcCore/io/Reader.h"

//
// Include XML Toolkit version macros
//
#include "version.h"

//
// Win32 DLL Versions of the library
//
#if defined(WIN32) && defined(QC_DLL)
	#if defined QC_XML_EXPORT
		#define QC_QCXML_PKG __declspec(dllexport)
		#define QC_QCXML_INCLUDE_INLINES 1
	#else
		#define QC_QCXML_PKG __declspec(dllimport)
		#if defined(_MSC_VER) && 1300 <= _MSC_VER
			#define QC_QCXML_INCLUDE_INLINES 0
		#else
			#define QC_QCXML_INCLUDE_INLINES 1
		#endif
	#endif
#else
	#define QC_QCXML_PKG
	#define QC_QCXML_INCLUDE_INLINES 1
#endif


#define QC_XML_PKG QC_QCXML_PKG
#define QC_XML_INCLUDE_INLINES QC_QCXML_INCLUDE_INLINES
#define QC_XML_NAMESPACE_BEGIN QC_SUB_NAMESPACE_BEGIN(xml)
#define QC_XML_NAMESPACE_END   QC_SUB_NAMESPACE_END(xml)

//
// Under MSVC, automatically link to requirted library
//
#if defined(_MSC_VER) && !defined(QC_XML_EXPORT)
#include "QcCore/base/libname.h"
#pragma comment(lib, QC_LIBNAME(qcxml))
#endif

//
// import types into the xml namespace:
//
QC_XML_NAMESPACE_BEGIN

using io::InputStream;
using io::Reader;

QC_XML_NAMESPACE_END

#endif //QC_XML_DEFS_h




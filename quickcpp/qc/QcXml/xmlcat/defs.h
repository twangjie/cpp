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
	@namespace qc::xmlcat

	Contains an implementation of the 
	<a href="http://www.oasis-open.org/">OASIS</a> XML Catalogs specification
	for XML entity resolution.  The full specification may be obtained
	from http://www.oasis-open.org/committees/entity/spec.html
*/
//==============================================================================

#ifndef QC_XMLCAT_DEFS_h
#define QC_XMLCAT_DEFS_h

#include "QcCore/base/gendefs.h"
#include "QcCore/base/QCObject.h"
#include "QcCore/base/AutoPtr.h"
#include "QcCore/base/String.h"
#include "QcCore/io/InputStream.h"
#include "QcCore/io/Reader.h"
#include "QcCore/net/URL.h"
#include "QcXml/xml/defs.h"

#define QC_XMLCAT_PKG QC_QCXML_PKG
#define QC_XMLCAT_INCLUDE_INLINES QC_QCXML_INCLUDE_INLINES
#define QC_XMLCAT_NAMESPACE_BEGIN QC_SUB_NAMESPACE_BEGIN(xmlcat)
#define QC_XMLCAT_NAMESPACE_END   QC_SUB_NAMESPACE_END(xmlcat)

//
// import required types into xmlcat namespace
//
QC_XMLCAT_NAMESPACE_BEGIN

using io::InputStream;
using io::Reader;
using net::URL;

QC_XMLCAT_NAMESPACE_END

#endif //QC_XMLCAT_DEFS_h




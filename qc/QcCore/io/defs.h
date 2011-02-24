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
	@namespace qc::io

	Provides flexible input/output abstractions for byte and Unicode
	character streams as well as concrete classes for reading, writing
	and manipulating files.
*/
//==============================================================================

#ifndef QC_IO_DEFS_h
#define QC_IO_DEFS_h

#include "QcCore/base/gendefs.h"
#include "QcCore/base/Character.h"
#include "QcCore/base/String.h"
#include "QcCore/base/QCObject.h"
#include "QcCore/base/AutoPtr.h"
#include "QcCore/base/AutoPtrMember.h"
#include "QcCore/cvt/CodeConverter.h"
#include "QcCore/util/DateTime.h"

#if defined(QC_IOS_EXPORT)
	#if defined(WIN32)

		#include "QcCore/base/winincl.h"
		#include <io.h>
		#include <direct.h>
		#define S_IWUSR S_IWRITE
		#define S_IRUSR S_IREAD

	#else // !WIN32

		#include <stddef.h>
		#include <unistd.h>

	#endif // WIN32
#endif // QC_IOS_EXPORT

#define QC_IO_PKG             QC_QUICKCPP_PKG
#define QC_IO_INCLUDE_INLINES QC_QUICKCPP_INCLUDE_INLINES
#define QC_IO_NAMESPACE_BEGIN QC_SUB_NAMESPACE_BEGIN(io)
#define QC_IO_NAMESPACE_END   QC_SUB_NAMESPACE_END(io)

QC_IO_NAMESPACE_BEGIN

using cvt::CodeConverter;
using util::DateTime;

QC_IO_NAMESPACE_END

#endif //QC_IO_DEFS_h


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

#include "CharTypeFacet.h"

QC_XML_NAMESPACE_BEGIN

#ifdef QC_ASCII
	#define QC_CHAR_TAB_SIZE 0x100
#else
	#define QC_CHAR_TAB_SIZE 0x10000
#endif

const size_t CharTypeFacet::s_CharTabSize = QC_CHAR_TAB_SIZE;

const UCS4Char CharTypeFacet::s_MaxChar = 
#if defined(QC_ASCII)
		0x7FUL;
#else
		0x10FFFFUL;
#endif

#ifndef QC_DOCUMENTATION_ONLY

unsigned char CharTypeFacet::s_XMLTable[QC_CHAR_TAB_SIZE] = {
#include "Unicode.tab"
};

#endif  //QC_DOCUMENTATION_ONLY

QC_XML_NAMESPACE_END




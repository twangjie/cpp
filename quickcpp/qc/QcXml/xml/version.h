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
// This file provides QuickCPP version information
//
//==============================================================================

#ifndef QC_XML_version_h
#define QC_XML_version_h

//
// The _QCXML_VERSION macro contains the version number for the
// installed version of the XML Toolkit.
// It consists of:
// - a major version number (multiplied by 100,000)
// - a minor version number (multiplied by 1000)
// - a minor release number
//
#undef  _QCXML_MAJOR_VERSION
#define _QCXML_MAJOR_VERSION 1                                                          

#undef  _QCXML_MINOR_VERSION
#define _QCXML_MINOR_VERSION 0                                                          

#undef  _QCXML_MINOR_RELEASE
#define _QCXML_MINOR_RELEASE 0                                                          

#undef  _QCXML_VERSION
#define _QCXML_VERSION    (_QCXML_MAJOR_VERSION*100000)\
                         + (_QCXML_MINOR_VERSION*1000)\
						 + (_QCXML_MINOR_RELEASE)

#endif // QC_XML_version_h

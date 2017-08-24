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

#ifndef QC_SAX_SAXFeatures_h
#define QC_SAX_SAXFeatures_h

#include "defs.h"

QC_SAX_NAMESPACE_BEGIN

class QC_SAX_PKG SAXFeatures
{
public:
	// Core sax features
	static const String namespaces;
	static const String namespace_prefixes;
	static const String validation;
	static const String external_general_entities;
	static const String external_parameter_entities;
	static const String lexical_handler_parameter_entities;
	static const String resolve_dtd_uris;
	static const String is_standalone;
	
	// QC-specific features
	static const String resolve_entity_uris;
	static const String interoperability;
	static const String warnings;
	static const String validate_namespace_declarations;
	static const String enable_relative_namespace_uri_test;
};

QC_SAX_NAMESPACE_END

#endif //QC_SAX_SAXFeatures_h

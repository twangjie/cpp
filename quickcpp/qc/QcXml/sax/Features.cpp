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

#include "Features.h"

OT_SAX_NAMESPACE_BEGIN

const String Features::namespaces = OT_T("http://xml.org/sax/features/namespaces");
const String Features::namespace_prefixes = OT_T("http://xml.org/sax/features/namespace-prefixes");
const String Features::validation = OT_T("http://xml.org/sax/features/validation");
const String Features::external_general_entities = OT_T("http://xml.org/sax/features/external-general-entities");
const String Features::external_parameter_entities = OT_T("http://xml.org/sax/features/external-parameter-entities");
const String Features::interoperability = OT_T("http://elcel.com/sax/features/interoperability");
const String Features::warnings = OT_T("http://elcel.com/sax/features/warnings");
const String Features::report_document_comments = OT_T("http://elcel.com/sax/features/report-document-comments");
const String Features::report_DTD_comments = OT_T("http://elcel.com/sax/features/report-DTD-comments");
const String Features::validate_namespace_declarations = OT_T("http://elcel.com/sax/features/validate-namespace-declarations");
const String Features::report_namespace_declarations = OT_T("http://elcel.com/sax/features/report-namespace-declarations");
const String Features::enable_relative_namespace_uri_test = OT_T("http://elcel.com/sax/features/enable_relative_namespace_uri_test");

OT_SAX_NAMESPACE_END


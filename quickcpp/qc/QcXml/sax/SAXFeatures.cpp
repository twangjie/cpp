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
// Class SAXFeatures
/**
    @class qc::sax::SAXFeatures

    @brief Helper class containing convenient constant values
	       for all the SAX features supported by the @QuickCPP
		   XMLReader.
	
	<p>For convenience, use the constant data members from this class when setting
	features on an XMLReader. Here is an example of how the SAXFeatures
	static data members can be used in place of the long feature names:</p>
	
    @code
    // Create a XMLReader
    AutoPtr<XMLReader> rpReader = XMLReaderFactory::CreateXMLReader();
    // Switch on validation
    rpReader->setFeature(SAXFeatures::validation, true);
    // Switch off namespace support
    rpReader->setFeature(SAXFeatures::namespaces, false);
    @endcode

    <hr><h4>SAX2 Standard Feature Flags</h4>

	<p>SAX defines standard feature URIs with the prefix
	@c http://xml.org/sax/features/ before an identifier such as
	@c validation. </p>

	The standard features supported by the @QuickCPP XMLReader are:-

    <table>
    <tr>
	<td>Feature ID</td>
	<td>Default</td>
	<td>Description</td>
	</tr>
    <tr>
	<td>external-general-entities</td>
	<td><tt>true</tt></td>
	<td> Reports whether this parser processes external
	    general entities; always @c true if validating.</td>
	</tr>
    <tr>
	<td>external-parameter-entities</td>
	<td><tt>true</tt></td>
	<td> Reports whether this parser processes external
	    parameter entities; always @c true if validating.</td>
	</tr>
    <tr>
	<td>is-standalone</td>
	<td><em>none</em></td>
	<td>May be examined only during a parse, after (but not during) the
	    <em>startDocument()</em> callback has been made; read-only.
	    The value is @c true if the document specified the
	    "standalone" flag in its XML declaration,
	    otherwise is @c false. </td>
	</tr>
    <tr>
	<td>lexical-handler/parameter-entities</td>
	<td><tt>false</tt></td>
	<td>@c true indicates that the LexicalHandler will report the
	    beginning and end of parameter entities.</td>
	</tr>
    <tr>
	<td>namespaces</td>
	<td><tt>true</tt></td>
	<td>@c true indicates namespace URIs and unprefixed local names
	    for element and attribute names will be available.</td>
	</tr>
    <tr>
	<td>namespace-prefixes</td>
	<td><tt>false</tt></td>
	<td>@c true indicates XML 1.0 names (with prefixes) and attributes
	    (including <em>xmlns*</em> attributes) will be available.</td>
	</tr>
	<tr>
	<td>resolve-dtd-uris</td>
	<td><tt>true</tt></td>
	<td>A value of @c true indicates that system IDs in declarations will
		be absolutized (relative to their base URIs) before reporting.
		(That is the default behavior for all SAX2 XML parsers.)
		A value of @c false indicates those IDs will not be absolutized;
		parsers will provide the base URI from
		Locator::getSystemId().
		This applies to system IDs passed in <ul>
		<li>DTDHandler::notationDecl(),</li>
		<li>DTDHandler::unparsedEntityDecl(), and</li>
		<li>DeclHandler::externalEntityDecl().</li>
		</ul>
		It does not apply to EntityResolver::resolveEntity(),
		which is not used to report declarations, or to
		LexicalHandler::startDTD(), which already provides
		the non-absolutized URI.
		</td>
	</tr>
    <tr>
	<td>validation</td>
	<td><tt>false</tt></td>
	<td>controls whether the parser is reporting all validity
	    errors; if @c true, all external entities will be read.</td>
	</tr>
   </table>

   <hr><h4>@QuickCPP Extended Feature Flags</h4>

	@QuickCPP provides some additions to the core SAX features.  They 
	are represented by absolute URIs starting with the prefix 
	@c http://qc.net/quickcpp/sax/features/. 
	The extended features supported by the @QuickCPP XMLReader are:-

  <table border="1" cellpadding="3" cellspacing="0" width="100%">
    <tr align="center" bgcolor="#ccccff">
	<td>Feature ID</td>
	<td>Default</td>
	<td>Description</td>
	</tr>
    <tr>
	<td>resolve-entity-uris</td>
	<td><tt>false</tt></td>
	<td>A value of @c true indicates that system IDs reported by Locator::getSystemId()
	    and EntityResolver::resolveEntity() will be absolutized
		(relative to their base URIs) before reporting.
		(That is the default behavior for all SAX2 XML parsers.)
		A value of @c false indicates those IDs will not be absolutized.</td>
	</tr>
    <tr>
	<td>interoperability</td>
	<td><tt>false</tt></td>
	<td>A value of @c true indicates that additional checks will be enabled
	    which test the XML input stream for interoperability with SGML-based systems.
		</td>
	</tr>
    <tr>
	<td>warnings</td>
	<td><tt>false</tt></td>
	<td>A value of @c true indicates that additional checks will be enabled
	    which perform tests that are deemed to be of low importance and result
		in ErrorHandler::warning() events when triggered.</td>
	</tr>
    <tr>
	<td>validate-namespace-declarations</td>
	<td><tt>true</tt></td>
	<td>A value of @c true indicates that namespace declaration attributes (those
	    beginning with <code>xmlns</code>) will be subject to DTD validation
		just like any other attributes.  A value of @c false makes the parser treat
		all namespace declarations as if they have been implicitly defined
		in the DTD.</td>
	</tr>
   </table>
*/
//==============================================================================

#include "SAXFeatures.h"

QC_SAX_NAMESPACE_BEGIN

/**
	Data member representing the constant URI @c http://xml.org/sax/features/namespaces.
*/
const String SAXFeatures::namespaces = QC_T("http://xml.org/sax/features/namespaces");

/**
	Data member representing the constant URI @c http://xml.org/sax/features/namespace-prefixes.
*/
const String SAXFeatures::namespace_prefixes = QC_T("http://xml.org/sax/features/namespace-prefixes");

/**
	Data member representing the constant URI @c http://xml.org/sax/features/validation.
*/
const String SAXFeatures::validation = QC_T("http://xml.org/sax/features/validation");

/**
	Data member representing the constant URI @c http://xml.org/sax/features/external-general-entities.
*/
const String SAXFeatures::external_general_entities = QC_T("http://xml.org/sax/features/external-general-entities");

/**
	Data member representing the constant URI @c http://xml.org/sax/features/external-parameter-entities.
*/
const String SAXFeatures::external_parameter_entities = QC_T("http://xml.org/sax/features/external-parameter-entities");

/**
	Data member representing the constant URI @c http://xml.org/sax/features/lexical-handler/parameter-entities.
*/
const String SAXFeatures::lexical_handler_parameter_entities = QC_T("http://xml.org/sax/features/lexical-handler/parameter-entities");

/**
	Data member representing the constant URI @c http://xml.org/sax/features/lexical-handler/resolve-dtd-uris.
*/
const String SAXFeatures::resolve_dtd_uris = QC_T("http://xml.org/sax/features/lexical-handler/resolve-dtd-uris");

/**
	Data member representing the constant URI @c http://xml.org/sax/features/lexical-handler/is-standalone.
*/
const String SAXFeatures::is_standalone = QC_T("http://xml.org/sax/features/lexical-handler/is-standalone");



// QC-specific members follow:-

/**
	Data member representing the constant URI @c http://qc.net/quickcpp/sax/features/resolve-entity-uris.
*/
const String SAXFeatures::resolve_entity_uris = QC_T("http://qc.net/quickcpp/sax/features/resolve-entity-uris");

/**
	Data member representing the constant URI @c http://qc.net/quickcpp/sax/features/interoperability.
*/
const String SAXFeatures::interoperability = QC_T("http://qc.net/quickcpp/sax/features/interoperability");

/**
	Data member representing the constant URI @c http://qc.net/quickcpp/sax/features/warnings.
*/
const String SAXFeatures::warnings = QC_T("http://qc.net/quickcpp/sax/features/warnings");

/**
	Data member representing the constant URI @c http://qc.net/quickcpp/sax/features/validate-namespace-declarations.
*/
const String SAXFeatures::validate_namespace_declarations = QC_T("http://qc.net/quickcpp/sax/features/validate-namespace-declarations");

/**
	Data member representing the constant URI @c http://qc.net/quickcpp/sax/features/enable-relative-namespace-uri-test.
*/
const String SAXFeatures::enable_relative_namespace_uri_test = QC_T("http://qc.net/quickcpp/sax/features/enable-relative-namespace-uri-test");

QC_SAX_NAMESPACE_END

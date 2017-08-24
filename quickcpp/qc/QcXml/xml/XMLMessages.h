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

#ifndef QC_XML_XMLMessages_h
#define QC_XML_XMLMessages_h

#include "defs.h"

QC_XML_NAMESPACE_BEGIN

enum MessageCodes {

EXML_DTDAFTERROOT         = 100, // DTD must appear before the root element
EXML_MULTIPLEROOTS        = 101, // there can be only one root element
EXML_ENCODINGEXCEPTION    = 102, // character encoding exception
EXML_STANDALONEWHITESPACE = 103, // element '{0}' contains white space and is declared externally which is invalid in a standalone document
EXML_ROOTNAMEMISMATCH     = 104, // the root element '{0}' must match the name '{1}' from the document type declaration
EXML_NODTD                = 105, // validation is not possible without a DTD
EXML_INVALIDCONTENT       = 106, // element content invalid. Element '{0}' is not expected here, expecting {1}
EXML_EMPTYSYNTAXUSED      = 107, // for interoperability, the empty element syntax <{0}/> should only be used for elements declared EMTPY
EXML_EMPTYSYNTAXNOTUSED   = 108, // for interoperability, the empty element syntax <{0}/> should be used for elements declared EMTPY
EXML_ELEMENTNOTDECLARED   = 109, // undeclared element '{0}'
EXML_ELEMENTENDMISSING    = 110, // element end tag </{0}> expected
EXML_PREMATUREELEMENTEND  = 111, // premature end to content of element '{0}'. Expecting child element {1}
EXML_BADSTAGTERMINATION   = 112, // element start tag not terminated correctly. Expected '>' or '/>'
EXML_ETAGSYNTAXWS         = 113, // unexpected white space before name in element end tag
EXML_ENDTAGMISMATCH       = 114, // end tag '</{0}>' does not match start tag.  Expected '</{1}>'
EXML_ETAGNOTTERMINATED    = 115, // element end tag not terminated correctly. Expected '>'
EXML_MULTIPLEXMLDECLS     = 116, // there can be only one XML declaration
EXML_XMLDECLNOTATSTART    = 117, // XML declaration not at start
EXML_UNSUPPORTEDVERSION   = 118, // XML version {0} is not supported
EXML_INVALIDSTANDALONE    = 119, // invalid standalone value '{0}'.  Must be 'yes' or 'no'
EXML_UNKNOWNXMLATTR       = 120, // unknown attribute '{0}' within XML declaration
EXML_MISSINGVERSION       = 121, // missing 'version' from XML declaration
EXML_XMLATTRORDER         = 122, // attribute order invalid within XML declaration
EXML_COMMENTDASHES        = 124, // comment text may not contain '--'
EXML_INVALIDPITARGET      = 125, // '{0}' is not a valid processing instruction target
EXML_DTDNOTTERMINATED     = 126, // DTD not terminated correctly

EXML_MISSINGWHITESPACE1   = 128, // white space expected before '{0}'
EXML_MISSINGWHITESPACE2   = 129, // white space expected before '{0}' in {1}
EXML_MISSINGWHITESPACE3   = 130, // white space expected before default declaration for attribute '{0}'
EXML_MISSINGWHITESPACE4   = 131, // white space expected before fixed value for attribute '{0}'
EXML_MISSINGWHITESPACE5   = 132, // white space expected after {0} in {1}

EXML_ATTRQUOTES           = 134, // attribute value must be enclosed by quotes
EXML_ATTRLT               = 135, // attribute value may not contain '<'
EXML_ATTRENTITYLT         = 136, // the replacement text of entity '&{0};' contains '<' which is illegal when used within an attribute value

EXML_ATTRIBUTEDECLARED    = 138, // attribute '{0}' has already been declared for element '{1}'
EXML_SYNTAXATTLIST        = 139, // white space expected after '{0}' in attribute list declaration for element '{1}'
EXML_ENTITYDECLARED       = 140, // entity '{0}' has already been declared
EXML_NDATAERROR           = 141, // the NDATA keyword is not permitted in parameter entity declarations
EXML_DECLNOTTERM          = 142, // {0} declaration not terminated correctly. Expected '{1}'
EXML_CHARCONTENT          = 143, // unexpected character content within element '{0}'
EXML_BADLYFORMEDENTITY    = 144, // the replacement text of entity '{0}' is badly formed: unexpected end tag
EXML_DUPLATTR             = 145, // duplicate attribute '{0}'
EXML_UNDECLATTR           = 146, // undeclared attribute '{0}' for element '{1}'
EXML_SANORMALIZATION      = 147, // the externally declared attribute '{0}' required normalization which is an error in a standalone document
EXML_ATTRSYNTAX           = 148, // expected '=' after attribute '{0}'
EXML_ATTRLISTPUNC         = 149, // unexpected punctuation within attribute list
EXML_MULTIPLEATTRTYPE     = 150, // element '{0}' has {1} attribute '{2}' therefore attribute '{3}' is not permitted also to have type '{1}'
EXML_ATTRENUMEMPTY        = 151, // at least one value must be provided in attribute {0} list
EXML_ATTRENUMSEPARATOR    = 152, // expected '|' separator in attribute {0} list
EXML_MISSINGSYSLITERAL    = 153, // missing system literal
EXML_IDATTRDEFAULT        = 154, // ID attribute '{0}' must have a declared default of #IMPLIED or #REQUIRED
EXML_INVALIDEXTID         = 155, // external identifier must start with 'PUBLIC' or 'SYSTEM'
EXML_DUPLELEMENTDECL      = 156, // duplicate declaration for element '{0}'
EXML_SYSTEMIDQUOTES       = 157, // system identifier must be enclosed by quotes
EXML_PUBLICIDQUOTES       = 158, // public identifier must be enclosed by quotes
EXML_MIXEDSPECEND         = 159, // mixed content specification with child elements must end with ')*'
EXML_CHARSPECMULTIPLICITY = 160, // invalid multiplicity for character content specification
EXML_ELEMENTCONTENTSTART  = 161, // element content specification must start with '('
EXML_MIXEDSUBGROUPS       = 162, // mixed content specification cannot contain sub groups
EXML_CONTENTMODELDELIM    = 163, // content model not correctly delimited
EXML_EMPTYCONTENTSPEC     = 164, // empty content specification in declaration of element type '{0}'
EXML_MIXEDDUPLELEMENT     = 165, // duplicate element name '{0}' is not permitted in the mixed content specification for element '{1}'
EXML_MIXEDMULTIPLICITY    = 166, // multiplicity may not be specified for individual elements in the mixed content specification for element '{0}'
EXML_MULTIPLICITYPOS      = 167, // multiplicity specification '{0}' must follow immediately after element name or ')'
EXML_CONDINTERNALSUBSET   = 168, // conditional sections are not permitted within the internal DTD subset
EXML_CONDKEYWORD          = 169, // missing conditional keyword: IGNORE/INCLUDE
EXML_CONDKEYWORDERR       = 170, // invalid conditional keyword '{0}' expecting 'IGNORE' or 'INCLUDE'
EXML_DUPLNOTATIONDECL     = 171, // duplicate declaration for notation '{0}'
EXML_INVALIDXMLCHAR       = 172, // character reference &{0}; is not a valid XML character
EXML_INVALIDXMLCHARCONF   = 173, // character reference &{0}; is not a valid XML character on this platform/configuration
EXML_MIXEDSEQUENCE        = 174, // element sequence is not permitted following '#PCDATA', use choice '|' instead
EXML_MIXSEQANDCHOICE      = 175, // illegal mix of choice and sequence elements within a single sub-element specification
EXML_INVALIDCHARREF       = 176, // invalid character reference
EXML_PEININTERNALSUBSET   = 177, // parameter entity reference '%{0};' must not be used within markup in the internal DTD subset
EXML_NAMESTART            = 178, // {0} must start with a letter, '_' or ':'
EXML_NAMESTARTNS          = 179, // {0} must start with a letter or '_'
EXML_NSCOLON              = 180, // {0} '{1}' must not contain ':' with namespaces enabled
EXML_NSENDCOLON           = 181, // {0} '{1}' must not end with a ':' with namespaces enabled
EXML_NSMULTICOLON         = 182, // {0} '{1}' must not contain multiple colons with namespaces enabled
EXML_REFNOTTERM           = 183, // entity reference not terminated correctly. Expected ';'
EXML_INVALIDTOKENEXP      = 184, // invalid token '{0}'. Probably intended {1}
EXML_UNEXPECTEDEOF        = 185, // unexpected end of file within {0}
EXML_UNEXPECTEDENTITYEND  = 186, // unexpected end of entity '{0}' within {1}
EXML_UNEXPECTEDCHAR       = 187, // unexpected character {0} within {1}
EXML_UNEXPECTEDTOKENEOF   = 188, // unexpected end of file, expecting {0}
EXML_UNEXPECTEDTOKENEOE   = 189, // unexpected end of entity '{0}', expecting {1}
EXML_UNEXPECTEDTOKEN      = 190, // unexpected {0}, expecting {1}
EXML_UNPARSEDENT          = 191, // illegal reference to unparsed entity '{0}'
EXML_EXTERNALENTINATTR    = 192, // reference to external entity '{0}' is forbidden within an attribute value
EXML_EXTERNALENTSA        = 193, // reference to externally declared entity '{0}' is not permitted from a standalone document
EXML_ENTOTYRECURSION      = 194, // recursive reference to entity '{0}'. Reference path: {1}
EXML_UNDECLENTITY         = 195, // reference to undeclared entity '{0}'
EXML_MISSINGID            = 196, // an IDREF attribute refers to the ID '{0}' which is not present in the document
EXML_RELATIVENSURI        = 197, // the namespace URI '{0}' represents a relative URI
EXML_NSURIBLANK           = 198, // the namespace URI for the prefix '{0}' must have a value
EXML_DUPLNSATTR           = 199, // duplicate attribute '{0}' within namespace URI '{1}'
EXML_UNDECLNS             = 200, // the namespace prefix '{0}' has not been declared
EXML_TEXTDECLATTR         = 201, // unknown attribute '{0}' within text declaration
EXML_TEXTDECLENC          = 202, // missing 'encoding' from text declaration
EXML_TEXTDECLATTRORDER    = 203, // attribute order invalid within text declaration
EXML_ENTITYNESTING        = 204, // {0} must start and end within the same entity
EXML_ATTRDEFAULTENUM      = 205, // default value for attribute '{0}' must be a value within the enumeration list
EXML_ATTRDUPLTOKEN        = 206, // duplicate token value '{0}' for attribute '{1}'
EXML_INTEROPENUM          = 207, // for interoperability, token value '{0}' must not appear more than once in attributes for element '{1}'
EXML_REQDEMPTY            = 208, // attribute '{0}' is declared as #REQUIRED but is empty
EXML_ATTRNOTINENUMLIST    = 209, // attribute '{0}' has invalid value, must have a value from the list: {1}
EXML_ATTRIDUNIQ           = 210, // attribute '{0}' has an ID value of '{1}' which is not unique within the document
EXML_ATTRFIXED            = 211, // attribute '{0}' with value '{1}' must equal the FIXED default value of '{2}'
EXML_ATTRNOTNMTOKEN       = 212, // value '{0}' for attribute '{1}' is not a valid {2}
EXML_UNDECLNOTN           = 213, // undeclared notation '{0}' referenced in attribute '{1}'
EXML_ENTITYNOTUNPARSED    = 214, // entity '{0}' is not an unparsed entity declared in the DTD
EXML_CHARCONTENTREF       = 215, // reference to character content not permitted within element '{0}'");
EXML_NOTDFA               = 216, // non-deterministic content model for element '{0}': more than one path leads to element '{1}'
EXML_EMPTYELEMNOTN        = 217, // attribute '{0}' of type NOTATION must not be declared on element '{1}' which has been declared EMPTY
EXML_ELEMUNDEFATTLIST     = 218, // attribute list declared for undefined element '{0}'
EXML_ATTRREQUIRED         = 219, // required attribute '{0}' has not been supplied for element '{1}'
EXML_ATTRDEFAULTNOTSA     = 220, // externally declared attribute '{0}' for element '{1}' has a default value of '{2}' which must be specified in a standalone document
EXML_UNDECLNOTNENTITY     = 221, // undeclared notation '{0}' referenced in declaration of entity '{1}'
EXML_NOCDSECTEND          = 222, // element content may not contain the literal ']]>'
EXML_ATTRDEFAULTEMPTY     = 223, // {0} attribute '{1}' may not have an empty default value
EXML_ATTREMPTY            = 224, // {0} attribute '{1}' may not have an empty value
EXML_MARKUPDECL           = 225, // parameter entity %{0}; does not contain complete markup declarations 
EXML_MAX
};

QC_XML_NAMESPACE_END

#endif //QC_XML_XMLMessages_h


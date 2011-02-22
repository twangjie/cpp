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
// Class: ParserImpl
// 
// Overview
// --------
// Private implementation of the Parser Interface.
//
// The QC Technology XML Parser is a recursive descent parser.  This means
// that each grammar production is coded as a function.  Although the XML 
// grammar does not require it, the parser enjoys the ability to perform
// unlimited back-tracking by virtue of the fact that the Scanner provides
// a flexible positioning interface.
//
// Early on, a decision had to be made: how much work should the Parser do?
// Should it simply tie parts of the input stream to XML grammar productions
// or should it check for constraints as well.  E.g. should the parser
// check that an end tag matches the start tag, or should that responsibility
// be handed up to a higher level?
//
// In making a design decision we are guided by constraints of our own:
// simplicity, maintainability, flexibility and extensibility versus
// perforance.  As parsing is quite a complex technique (even recursive
// descent parsing which is relatively easy), it makes sense to keep
// this class focused on the gramatical work.
//
// However, in the above example (mismatched start/end tags) we are dealing
// with a structural issue - which is something that the parser is inherently
// interested in.  In this case this parser will trap such exceptions and report
// them (afterall - it is very easy to do).  Perhaps a better split of 
// responsibility is between a validating / non validating parser.  We shall see...
//
// Also, how should the distinction between a scanner and a parser be handled?  Ideally
// the scanner should give us tokens that the parser forms into a grammar,  But what are
// xml tokens?  From the scanners perspective these are single unicode characters or
// delimited strings.  The scanner knows nothing about the various XML tags - just the types
// of characters that make them up.
//
// Interface Design
// ----------------
// This parser is a SAX-like event-based parser.  It performs the basic
// XML grammar recognition and calls virtual functions whenever an
// interesting event occurs or custom processing is required.
//
// We have not elected to write a native SAX parser for two reasons:
// 1) At the time of writing there is not a definitive C++ SAX specification
// 2) The SAX specification, being java centric, makes use of custom container
//    classes.  The QC Technology XML Parser is built from and around the
//    standard C++ library and therefore natively uses STL containers.
// 3) We want to provide a richer native API than canbe built on top of SAX
//
// We do intend to make a SAX2.0 interface available, but it will be built
// on top of the native C++ interface.  Therefore, by necessity, the native
// interface will be very similar to SAX2.
//
// 
// Interface Efficiency
// --------------------
// The [java] SAX inteface passes character arrays to callback methods
// to remove the need to copy the characters into internal strings.  If we
// adopted this approach it would have an impact on our scanner design - it
// would have to return offsets into a character buffer rather than returning
// strings.  This would be fine if we used either one buffer or buffers were
// arranged on logical boundaries, but our scanner will use fixed-length
// buffers [logically bounded buffers would require the buffer to be 
// arbitrarily large - especially if we wanted to break on a newline which may
// not exist].
//
// For the current implementation we trust to the efficiency of the 
// C++ Standard Library string class - which is no doubt much faster
// than the java equivalent - and call callbacks with a string.  The one 
// exception to this (currently) is onCharData() which takes a BufferRange
// object that can simply be converted to a String if required.
//
// Exceptions
// ----------
// Due to the relatively high cost of Exception processing, we avoid coding
// try/catch blocks in code that is called frequently.  Therefore exceptions
// are not used to report trivial error conditions that should be recovered
// from.  They are used however in low-level routines in other parts of 
// Celio, so programs need to be aware of the need to trap exceptions.
//
// Internal Parsing Functions
// --------------------------
// Parsing functions typically return a bool value to indicate if they were
// successful in recognising and consuming the next production.  The convention
// is that if they return true the token is consumed from the input stream, if
// they return false then the scanner position is maintained at the present
// location.
//
// This forward-looking ability is enabled by the Scanner's back-tracking
// functionality.  This is done primarily by the Scanner::Peekxxx functions.
//
// Error Detection
// ---------------
// At what point should an error be detected?
// Say for example an xml file had an incorrect XMLDecl.  Eg
// <?xml varsion="1.0" ?>
// This is not a valid XMLDecl (varsion instead of version), so should the
// parser back-track and try something else?  The answer in this case is
// definitely not, because we know there are no other productions starting
// with the string "<?xml".  But how can we fit this into a common approach
// to error handling/reporting?
//
// The answer is that once a token is accepted for a grammar production,
// we are committed to creating that production - and failure is an error.
// With hindsight we can state that for XML 1.0 this is true - but it may
// change at some stage in the future.
//
// Error Recovery
// --------------
// After an error, we always try to recover to the next element.  For example,
// errors within the XMLDecl shouldn't prevent the rest of the document being 
// parsed.
//
// Error recovery generally involves scanning to the end of the grammar production or
// the next "<" character - which ever comes first.
// 
// Using the XML Grammar
// ---------------------
// The xml grammar is good for showing what is legal XML, but it is not useful for
// writing a formal recursive descent parser with back-tracking.  The reason is a
// positive one - only one token of lookahead is required [with some minor exceptions]
// so we don't need to back-track to find the next grammar production.  The scanner
// is only incremented when we know what whe're looking for.
//
//==============================================================================

#include "ParserImpl.h"
#include "Scanner.h"
#include "BufferRange.h"
#include "CharTypeFacet.h"
#include "InternalEntity.h"
#include "ExternalEntity.h"
#include "ElementType.h"
#include "AttributeType.h"
#include "ElementContentSpec.h"
#include "XMLParsingException.h"
#include "AttributeSetImpl.h"
#include "XMLMessages.h"
#include "XMLNames.h"

#include "ContentEventHandler.h"
#include "DTDEventHandler.h"
#include "EntityEventHandler.h"
#include "EntityResolutionHandler.h"
#include "ErrorEventHandler.h"

#include "QcCore/base/Character.h"
#include "QcCore/base/System.h"
#include "QcCore/base/debug.h"
#include "QcCore/base/AutoBuffer.h"
#include "QcCore/base/NullPointerException.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/util/MessageFormatter.h"
#include "QcCore/util/ValueRestorer.h"
#include "QcCore/io/StringReader.h"
#include "QcCore/io/File.h"
#include "QcCore/io/CharacterCodingException.h"
#include "QcCore/net/URL.h"
#include "QcCore/net/URLConnection.h"
#include "QcCore/net/URLEncoder.h"

#include <algorithm>  
#include <stdio.h>  

QC_XML_NAMESPACE_BEGIN

using namespace io;
using namespace net;
using namespace util;

const Character chSpace(' ');

const CharType cSpace = ' ';
const CharType cDash = '-';
const CharType cOpenAngle = '<';
const CharType cOpenBrace = '[';
const CharType cOpenBracket = '(';
const CharType cCloseAngle = '>';
const CharType cCloseBrace = ']';
const CharType cCloseBracket = ')';
const CharType cEqual = '=';
const CharType cQuote = '"';
const CharType cApos = '\'';

const char* szCloseAngle = ">";
const char* szCloseBrace = "]";
const char* szCloseBracket = ")";
const char* szPIEnd = "?>";
const char* szEmptyElemClose = "/>";
const char* szSemicolon = ";";

const String sNull;

const String sApos = QC_T("'");
const String sAmpersand = QC_T("&");
const String sCloseAngle = QC_T(">");
const String sEmptyElemClose = QC_T("/>");
const String sXMLDecl(QC_T("<?xml"));
const String sAttributeDeclaration = QC_T("attribute declaration");
const String sCondSection = QC_T("conditional DTD section");
const String sCDATAEnd = QC_T("]]>");
const String sDashDash = QC_T("--");
const String sDTDMarkup = QC_T("DTD markup");
const String sRootElementName = QC_T("/");
const String sStartETag = QC_T("</");
const String sCommentStart = QC_T("<!--");
const String sCommentEnd = QC_T("-->");
const String sEntityDeclaration = QC_T("entity declaration");
const String sNotationDeclaration = QC_T("notation declaration");
const String sPEName = QC_T("parameter entity name");
const String sPIStart = QC_T("<?");
const String sPIEnd = QC_T("?>");
const String sXML = QC_T("xml");
const String sDoctypeDecl = QC_T("<!DOCTYPE");
const String sNDATA = QC_T("NDATA");
const String sCDSectStart = QC_T("<![CDATA[");
const String sCondDecl = QC_T("<![");
const String sCDSectEnd = QC_T("]]>");
const String sSystem = QC_T("SYSTEM");
const String sPublic = QC_T("PUBLIC");
const String sNotation = QC_T("NOTATION");
const String sAttlistDecl = QC_T("<!ATTLIST");
const String sNotationDecl = QC_T("<!NOTATION");
const String sNSPrefix = QC_T("xmlns");
const String sRequired = QC_T("#REQUIRED");
const String sImplied = QC_T("#IMPLIED");
const String sFixed = QC_T("#FIXED");
const String sElementDecl = QC_T("<!ELEMENT");
const String sEmpty = QC_T("EMPTY");
const String sAny = QC_T("ANY");
const String sPCData = QC_T("#PCDATA");
const String sInclude = QC_T("INCLUDE");
const String sIgnore = QC_T("IGNORE");
const String sCharRefStart = QC_T("&#");
const String sColon = QC_T(":");

const String sPercent = QC_T("%");
const String sDoctypeDeclaration = QC_T("DOCTYPE declaration");
const String sDTD = QC_T("DTD");
const String sBeforePEName = QC_T("before PE name");
const String sAfterEntityName = QC_T("after entity name");
const String sBetweenNDATAandNotation = QC_T("between NDATA and notation name");
const String sAfterNOTATION = QC_T("after NOTATION");
const String sBeforeSystemLiteral = QC_T("before system literal");
const String sBeforePublicLiteral = QC_T("before public literal");
const String sAttribute = QC_T("attribute");
const String sCDATA = QC_T("CDATA");
const String sVersion = QC_T("version");
const String sEncoding = QC_T("encoding");
const String sAttributeName = QC_T("attribute name");
const String sElementName = QC_T("element name");
const String sElementType = QC_T("element type");
const String sStandalone = QC_T("standalone");
const String sComment = QC_T("comment");
const String sPITarget = QC_T("processing instruction target");
const String sPI = QC_T("processing instruction");
const String sDTDName = QC_T("DOCTYPE name");
const String sEntityDecl = QC_T("<!ENTITY");
const String sEntityName = QC_T("entity name");
const String sNotationName = QC_T("notation name");
const String sElementDeclaration = QC_T("element declaration");
const String sEntityReference = QC_T("entity reference");
const String strApos = QC_T("'");
const String strQuote = QC_T("\"");

const int EndOfEntity = -1;

const char* szCharContentErr       = "unexpected character content within element '{0}'";
const char* szMultiplAttrTypeErr   = "element '{0}' has {1} attribute '{2}' therefore attribute '{3}' is not permitted also to have type '{1}'";
const char* szBadContentModelDelim = "content model not correctly delimited";
const char* szNSNameStart          = "{0} must start with a letter or '_'";

struct TokenTableEntry
{
	int m_tokenType;
	const char* m_string;
	size_t m_length;
	const char* m_altString;
};

enum TokenType {Unknown,
	          StartElement,
	          EndElement,
	          PI,
	          XMLDecl,
			  CDSect,
	          DTD,
	          Comment,
			  ElementDecl,
			  EntityDecl,
			  NotationDecl,
			  AttlistDecl,
			  ConditionalDecl,
			  Reference,
			  PEReference,
	          maxTokens };

//
// Note: the PEReference enum works below because it is larger than
// the largest AttributeType
//
TokenTableEntry AttributeTable[] = 
{
	{ AttributeType::CDATA,        "CDATA",      5, 0},
	{ AttributeType::ID,           "ID",         2, 0},
	{ AttributeType::IDREF,        "IDREF",      5, 0},
	{ AttributeType::IDREFS,       "IDREFS",     6, 0},
	{ AttributeType::ENTITY,       "ENTITY",     6, 0},
	{ AttributeType::ENTITIES,     "ENTITIES",   8, 0},
	{ AttributeType::NMTOKEN,      "NMTOKEN",    7, 0},
	{ AttributeType::NMTOKENS,     "NMTOKENS",   8, 0},
	{ AttributeType::NOTATION,     "NOTATION",   8, 0},
	{ AttributeType::ENUMERATION,  "(",          1, 0},
	{ PEReference,                 "%n",         2, "%pe-name;"},
	{ 0, 0, 0, 0                                     }
};

TokenTableEntry PrologTable[] = 
{
	{ StartElement,    "<n",           2, "<element-name"     },
	{ EndElement,      "</",           2, 0                   },
	{ PI,              "<?",           2, "<?pi-target"       },
	{ XMLDecl,         "<?xmlw",       6, "<?xml"             },
	{ CDSect,          "<![CDATA[",    9, 0                   },
	{ DTD,             "<!DOCTYPEw",  10, "<!DOCTYPE"         },
	{ Comment,         "<!--",         4, 0                   },
	{ ElementDecl,     "<!ELEMENT",    9, 0                   },
	{ EntityDecl,      "<!ENTITY",     8, 0                   },
	{ NotationDecl,    "<!NOTATION",  10, 0                   },
	{ AttlistDecl,     "<!ATTLIST",    9, 0                   },
	{ ConditionalDecl, "<![",          3, "<![INCLUDE/IGNORE" },
	{ PEReference,     "%n",           2, "%pe-name;"         },
	{ 0, 0, 0, 0                                              }
};

TokenTableEntry ContentTable[] = 
{
	{ StartElement,    "<n",           2, "<element-name"  },
	{ EndElement,      "</",           2, 0                },
	{ PI,              "<?n",          3, "<?pi-target"    },
	{ CDSect,          "<![CDATA[",    9, 0                },
	{ Comment,         "<!--",         4, 0                },
	{ Reference,       "&",            1, 0                },
	{ 0, 0, 0, 0                                           }
};

const char* tokenDescriptionTable[maxTokens] =
{
	"unknown element",
	"start element",
	"end element",
	"processing instruction",
	"xml declaration",
	"CDATA section",
	"DTD",
	"comment",
	"element declaration",
	"entity declaration",
	"notation declaration",
	"attribute list declaration",
	"conditional declaration",
	"reference",
	"parameter entity reference"
};


//=============================================================================
// ParserImpl::NamespaceFrame constructor.
//
// Note: Creates a default frame containing the "xml" namespace prefix.
//=============================================================================
ParserImpl::NamespaceFrame::NamespaceFrame()
{
	m_prefixMap[QC_T("xml")] = XMLNames::XMLNamespaceURI;
}

//==============================================================================
// NamespaceFrame::NamespaceFrame
//
// Constructor which leaves the delta list empty
//==============================================================================
ParserImpl::NamespaceFrame::NamespaceFrame(const PrefixMap& prefixMap,
                                           const String& defaultURI) :
	m_defaultURI(defaultURI),
	m_prefixMap(prefixMap)
{
}

//=============================================================================
// Parser constructor
// 
//=============================================================================
ParserImpl::ParserImpl() :
	m_npContentEventHandler(0),
	m_npDTDEventHandler(0),
	m_npEntityEventHandler(0),
	m_npEntityResolutionHandler(0),
	m_npErrorEventHandler(0)
{
	//
	// Add the built-in entities to the entity map
	// These are defined in "XML 1.0 - 4.6"
	//
	m_stdEntityMap[QC_T("amp")] = QC_T("&");
	m_stdEntityMap[QC_T("lt")] = QC_T("<");
	m_stdEntityMap[QC_T("gt")] = QC_T(">");
	m_stdEntityMap[QC_T("apos")] = QC_T("'");
	m_stdEntityMap[QC_T("quot")] = QC_T("\"");

	//
	// Prevent needless re-allocation of the namespace frame vector
	//
	const int DefaultNamespaceFrameSize = 5;
	m_namespaceFrameVector.reserve(DefaultNamespaceFrameSize);

	resetParser();
}

ParserImpl::~ParserImpl()
{
}

//==============================================================================
// ParserImpl::parseURL
//
// One of two methods available to start parsing a document.
// This method converts the passed systemId into an XMLInputSource and parses
// that.
//==============================================================================
void ParserImpl::parseURL(const String& systemId)
{
	//
	// Note: We do not call the entity resolver to resolve the passed systemId,
	// we assume the application knows what it wants to parse.
	//
	AutoPtr<XMLInputSource> rpInput = new XMLInputSource(systemId);

	parseInputSource(rpInput.get());
}

//==============================================================================
// ParserImpl::parseInputSource
//
// Parse a document represented by an XMLInputSource.
// The XMLInputSource should be thought of as an abstract class 
// representing a number of different ways of reading an XML entity.
//==============================================================================
void ParserImpl::parseInputSource(XMLInputSource* pInputSource)
{
	if(!pInputSource) throw NullPointerException();

	//
	// prepare for a new document
	//
	resetParser();

	//
	// Set the document-level indentifiers
	// (used by our internal Position interface)
	//
	m_docSystemId = pInputSource->getSystemId();
	m_docPublicId = pInputSource->getPublicId();

	AutoPtr<ExternalEntity> rpDocEntity;

	try
	{
		// This is wrapped in a try block so that we can guarantee
		// to call the onEndDocument() method to signal the end of the document
		// even when parsing ceases prematurely due to a fatal error.
		//

		// Note: This must be called now because events (notably error events)
		// can start to occur as soon as we try to read the document entity
		// and before we call parseDocument()
		//
		if(m_npContentEventHandler)
		{
			m_npContentEventHandler->onStartDocument(pInputSource->getSystemId());
		}

		//
		// We define "parse in progress" to be the time between calling
		// startDocument() and endDocument() which is the equivalent
		// to the current scope
		//
		ValueRestorer<bool> flagRestorer(m_bParseInProgress, true);

		//
		// Turn the (abstract) XMLInputSource into a (tangible) ExternalEntity
		// This will do all sorts of nice things, like creating a Character
		// reader (if required) and sorting out the encoding headache.
		//
		rpDocEntity = new ExternalEntity(*this,
										 EntityType::Document,
										 pInputSource);
		//
		// Get a ScannerPosition at the start of the entity.  Note that
		// this will attempt to load the entity, and therefore may
		// give rise to exceptions
		//
		m_scannerPos = rpDocEntity->getStartPosition();

		//
		// And now parse the document
		//
		parseDocument();

		m_bParseInProgress = false;

		//
		// onEndDocument() is placed here and in each catch block so that it is always
		// called once - even after an exception is thrown.
		//
		if(m_npContentEventHandler)
		{
			m_npContentEventHandler->onEndDocument();
		}
	}
	catch(CharacterCodingException& e)
	{
		//
		// In order to create a nice context string, we need
		// to override the Reader's CodeConverter to not throw
		// an exception on further malformed input, but rather to
		// replace malformed character with a replacement character.
		//
		e.getCodeConverter()->setInvalidCharAction(CodeConverter::replace);

		if(m_scannerPos.isValid())
			errorDetected(Fatal, e.getMessage(), EXML_ENCODINGEXCEPTION);

		if(m_npContentEventHandler)
		{
			m_npContentEventHandler->onEndDocument();
		}

		if(!m_scannerPos.isValid())
		{
			throw;
		}
	}
	catch(IOException& /*e*/)
	{
		//
		// IOExceptions are not given to the error handler
		// as the application expects to deal with them all by itself
		//
		if(m_npContentEventHandler)
		{
			m_npContentEventHandler->onEndDocument();
		}

		//
		// propagate the exception back
		//
		throw;
	}
	catch(Exception& e)
	{
		if(!m_bInErrorHandler)
		{
			try
			{
				errorDetected(Fatal, e.toString(), 0);
			}
			catch(...)
			{
			}
		}

		if(m_npContentEventHandler)
		{
			m_npContentEventHandler->onEndDocument();
		}

		//
		// If we did not return cleanly from the error handler, then the
		// exception must be propogated to the application
		//
		if(m_bInErrorHandler)
		{
			throw;
		}
	}
	catch(...)
	{
		if(m_npContentEventHandler)
		{
			m_npContentEventHandler->onEndDocument();
		}

		//
		// propagate the exception back
		//
		throw;
	}


	//
	// And release resources...
	//
	m_rpDTDEntity.release();
}

//==============================================================================
// ParserImpl::parseDTD
//
//==============================================================================
void ParserImpl::parseDTD(const String& systemId)
{
	//
	// Prepare for a new document
	//
	resetParser();

	try
	{
		// This is wrapped in a try block so that we can guarantee
		// to call the onEndDocument() method to signal the end of the document
		// even when parsing ceases prematurely due to a fatal error.
		//
		if(m_npContentEventHandler)
		{
			m_npContentEventHandler->onStartDocument(systemId);
		}

		//
		// Note: We do not call the entity resolver to resolve the passed systemId,
		// we assume the application knows what it wants to parse.
		//
		AutoPtr<XMLInputSource> rpInput = new XMLInputSource(systemId);

		m_rpDTDEntity = new ExternalEntity(*this, EntityType::DTD, rpInput.get());

		if(m_npDTDEventHandler)
		{
			m_npDTDEventHandler->onStartDTD(sNull, sNull, systemId);
		}

		parseExtDTDEntity();
	}
	catch(...)
	{
		if(m_npDTDEventHandler)
		{
			m_npDTDEventHandler->onEndDTD();
		}

		if(m_npContentEventHandler)
		{
			m_npContentEventHandler->onEndDocument();
		}

		//
		// propagate the exception back
		//
		throw;
	}


	if(m_npDTDEventHandler)
	{
		m_npDTDEventHandler->onEndDTD();
	}

	if(m_npContentEventHandler)
	{
		m_npContentEventHandler->onEndDocument();
	}

	m_rpDTDEntity.release();
}

//=============================================================================
// ParserImpl::parseDocument()
// 
// [01] document ::= prolog element Misc*
//
//=============================================================================
void ParserImpl::parseDocument()
{
	//
	// And now... parse the document!
	// 
	parseProlog();

	String strToken;
	bool bErrorReported(false);
	const int nextToken = testNextTokenType(ContentTable, strToken, bErrorReported);
	if(nextToken == StartElement)
	{
		// parse the root element
		parseElement(true /*root*/);
		//
		// Now that we have parsed the root element,
		// there are some additional validity tests to  perform
		//
		if(m_features.m_bDoValidityChecks)
			postRootValidityChecks();
	}
	else if(!bErrorReported)
	{
		unexpectedToken((TokenType)nextToken, strToken, QC_T("root element"));
	}

	parseMisc();
	
	//
	// There, that wasn't too hard was it!
	//

	// Let's just ensure that we are at EOF - if not then we have an
	// error of some kind...
	const int lastToken = testNextTokenType(PrologTable, strToken, bErrorReported);
	
	if(lastToken == DTD)
	{
		errorDetected(Fatal, System::GetSysMessage(sXML, EXML_DTDAFTERROOT,
		"DTD must appear before the root element"), EXML_DTDAFTERROOT);
	}
	if(lastToken == StartElement)
	{
		errorDetected(Fatal, System::GetSysMessage(sXML, EXML_MULTIPLEROOTS,
		"there can be only one root element"), EXML_MULTIPLEROOTS);
	}
	else if(lastToken != EndOfEntity && !bErrorReported)
	{
		unexpectedToken(lastToken, strToken, QC_T("end of document"));
	}
}

//=============================================================================
// ParserImpl::parseProlog
// 
// According to the document grammar production [1], Anything that can legally
// appear before the root element is part of the document prolog.
//
// [22] prolog ::= XMLDecl? Misc* (doctypedecl Misc*)?
//
// At the end of this function the Scanner should be positioned at the
// start of the root element tag.
//=============================================================================
bool ParserImpl::parseProlog()
{
	bool bRet = true;
	
	parseMisc();
	parseDocTypeDecl();	// only one of these allowed!
	parseMisc();

	//
	// If no DOCTYPE declaration was found, and a DTD override has been
	// requested then now is the time to load it in.
	//
	if(!m_bHasDTD && m_rpDTDOverride)
	{
		m_bHasDTD = true;
		parseExtDTDSubsetOverride();
	}

	return bRet;
}

//=============================================================================
// ParserImpl::parseWhitespace
// 
// White-space that occurs outside of markup is part of the document and
// therefore we need to be able to call onWhitespace().  This is 
// not required for white-space that occurs within markup.
//
//=============================================================================
bool ParserImpl::parseWhitespace()
{
	bool bWhitespaceFound = false;
	Character nextChar;
	
	const BufferRange& range = Scanner::GetNextContigString(m_scannerPos,
	                                                        CharTypeFacet::Whitespace,
	                                                        0, sNull, nextChar);
	if(range.getSize())
	{
		bWhitespaceFound = true;
		if(areCallbacksPermitted() && m_npContentEventHandler)
		{
			for(size_t i=0; i<range.getNumFragments(); i++)
			{
				const BufferRange::Fragment& fragment = range.getFragment(i);
				if(fragment.length)
				{
					m_npContentEventHandler->onWhitespace(fragment.pData, fragment.length);
				}
			}
		}
	}

	return bWhitespaceFound;
}

//=============================================================================
// ParserImpl::parseIgnorableWhitespace
// 
// White-space that occurs within element content where the element contains
// 'element content' (ie no character data) is deemed "ignorable white-space".
//
//=============================================================================
bool ParserImpl::parseIgnorableWhitespace()
{
	bool bWhitespaceFound = false;
	Character nextChar;
		
	const BufferRange& range = Scanner::GetNextContigString(m_scannerPos,
	                                                        CharTypeFacet::Whitespace,
	                                                        0, sNull, nextChar);
	if(range.getSize())
	{
		QC_DBG_ASSERT(m_pCurrentElementType!=0);

		//
		// Validity Constraint: Standalone Document Declaration
		//
		// externally declared element type with element content cannot contain
		// white-space between elements (because it is treated differently by a
		// validating parser)
		//
		if(m_features.m_bDoValidityChecks && m_bStandaloneDoc && m_pCurrentElementType->isExternallyDeclared())
		{
			const String& err = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_STANDALONEWHITESPACE,
				"element '{0}' contains white-space and is declared externally which is invalid in a standalone document"),
				m_pCurrentElementType->getName().getRawName());

			errorDetected(Error, err, EXML_STANDALONEWHITESPACE);
		}

		bWhitespaceFound = true;

		if(areCallbacksPermitted() && m_npContentEventHandler)
		{
			for(size_t i=0; i<range.getNumFragments(); i++)
			{
				const BufferRange::Fragment& fragment = range.getFragment(i);
				if(fragment.length)
				{
					m_npContentEventHandler->onIgnorableWhitespace(fragment.pData, fragment.length);
				}
			}
		}
	}

	return bWhitespaceFound;
}

//=============================================================================
// ParserImpl::parseMisc
// 
// Misc is the grammar production that can appear either with the document prolog
// or after the root element.  It consists of comments, processing instructions
// and white-space.
//
// [27] Misc ::= Comment | PI | S
//
// We can parse this production deterministically because each terminal has a 
// different start pattern:-
//   comments start <!--
//   PIs start <?Name
//   we all know what white-space looks like!
//
// Note: Misc is always used in a "zero or more" context.  To avoid unnecessary
// calling we implement this loop ourselves.
//=============================================================================
bool ParserImpl::parseMisc()
{
	bool bMiscFound = false;
	bool bContinue = true;
	while(bContinue)
	{
		//
		// First check for whitespace.  White-space is valid within Misc
		// but testNextTokenType() doesn't recognize it.
		//
		if(parseWhitespace())
			bMiscFound = true;

		String strToken;
		bool bErrorReported(false);
		const int nextToken = testNextTokenType(PrologTable, strToken, bErrorReported);

		//
		// We process according to the grammar here - if the next tag
		// is a comment or a PI we deal with it, otherwise
		// we assume that it belongs to the next grammar production.  Whether
		// it does or not is up to that function to decide.
		//

		//
		// Note, however, than xmlDecl has been added to Misc.  The rules
		// to check that it is at the beginning of the document are included
		// within the parseXMLDecl.
		// 
		switch(nextToken)
		{
		case XMLDecl:
			bMiscFound = parseXMLDecl();
			QC_DBG_ASSERT(bMiscFound);
			break;
		case Comment:
			bMiscFound = parseComment();
			QC_DBG_ASSERT(bMiscFound);
			break;
		case PI:
			bMiscFound = parsePI();
			QC_DBG_ASSERT(bMiscFound);
			break;
		case EndOfEntity:
			bContinue = false;
			break;
		case 0:
		case PEReference:
			if(!bErrorReported)
				unexpectedToken(nextToken, strToken, QC_T("processing instruction or comment"));
			Scanner::SkipNextStringConstant(m_scannerPos, strToken);
			recoverPosition(1, &szCloseAngle, 0);
			break;
		default:
			bContinue = false;
		}
	}
	return bMiscFound;
}

//=============================================================================
// ParserImpl::parseElement
// 
// [39] element ::= EmptyElemTag | Stag content ETag
// [40] STag ::= '<' Name (S Attribute)* S? '>'
// [42] ETag ::= '</' Name S? '>'
// [43] content ::= CharData? ((element | Reference | CDSect | PI | Comment)
//                  CharData?)*
// [44] EmptyElemTag ::= '<' Name (S Attribute)* S? '/>'
//
// Notice that STag and EmptyElemTag are non-deterministic - we only 
// discover the difference at the end of the tag - therefore we cannot parse
// these seperately (even with unlimited back-tracking, we would still 
// be in the position of having reported errors and possibly called call-backs
// during the first pass.  Also, it is unnecessarily inefficient to parse
// these productions separately.
//
// We believe the grammar is better described as:-
// [et] element ::= '<' Name (S Attribute)* S? ('/>' | ('>' content ETag))
//
// Namespaces
// ----------
// In XML 1.0, element and attribute names are simple Name productions
// consisting of a string of name characters and starting with a name-start
// character.  With namespace support, however, attribute and element names
// use the QName production - which allows for a prefix and a local name
// separated by a single colon.  The prefix is then resolved to a namespace
// URI.
//
// Namespaces are declared using a special reserved attribute: xmlns.  The
// namespace declaration has a scope entailing the element in which it is
// declared and all its child elements.  Therefore, we cannot decide what the
// qualified name of the element is until all the attributes have been 
// processed (including attributes defaulted from the DTD/Schema)
//
//=============================================================================
bool ParserImpl::parseElement(bool bRootElement)
{
	QName elementName;

	bool bEmptyElement = false;
	AttributeSetImpl attrSet;
	const ElementType* pElementType = NULL;
	bool bElementFound = parseSTag(elementName, bEmptyElement, pElementType, attrSet);

	if(bElementFound)
	{
		const ElementContentSpec* pSubElementSpec = NULL;
	
		if(pElementType)
		{
			//
			// Get the element content spec (an object which is used
			// to determine what elements may follow this one)
			// (This will be null if the element has not been defined)
			//
			pSubElementSpec = pElementType->getContentSpec();

			//
			// Use the element definition to validate the attributes
			// that are missing (processes both defaulted attributes
			// and checks for missing required attributes)
			// Note: This does not require the element to have been "defined" - only the
			// attributes for the element need to have been defined.
			//
			pElementType->validateMissingAttributes(attrSet, m_features.m_bDoValidityChecks, *this);
		}

		if(m_features.m_bDoValidityChecks)
		{
			elementStartValidityChecks(elementName, bRootElement, bEmptyElement, pElementType);
		}

		//
		// Namespace processing
		//
		// This comprises of:-
		// 1) locating the xmlns attributes (if any)
		// 2) optionally - removing the xmlns attributes from the list so that
		//    the application doesn't need to worry about them
		// 3) updating the namespace prefix map with any newly declared prefixes
		// 4) qualifying any prefixed attributes
		// 5) testing for duplicate atrtributes (those with same local name, 
		//    different namespace prefixes but same namespace uri)
		// 6) qualifying the element name
		// 7) after processing all child elements, restoring the prefix map to its
		//    prior state.

		//
		// If namespace support is enabled then we need to iterate thru the
		// attribute list searching for (and optionally removing) namespace
		// declaration attributes (xmlns...)
		//
		bool bAnyNamespaceDeclarations = false;
		if(m_features.m_bNamespaceSupport)
		{
			bAnyNamespaceDeclarations = processNamespacePrefixes(elementName, attrSet);
			if(bAnyNamespaceDeclarations)
			{
				reportStartNamespacePrefixes();
			}
		}

		//
		// These need to be at this block level
		// to satisfy the isEndElementPermitted() call later
		ValueRestorer<String> elementRestorer(m_currentElementName, elementName.getRawName());
		ValueRestorer<const ElementType*> typeRestorer(m_pCurrentElementType, pElementType);
		ValueRestorer<const ElementContentSpec*> specRestorer(m_pCurrentElementContentSpec, pSubElementSpec);

		if(areCallbacksPermitted() && m_npContentEventHandler)
		{
			m_npContentEventHandler->onStartElement(elementName, bEmptyElement, attrSet);
		}

		if(!bEmptyElement)
		{
			// Elements contain elements contain elments.....
			parseContent();

			if(!parseETag(elementName))
			{
				// note: unlikely to ever see this message because
				// parseContent() will have reported the error at this location.
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_ELEMENTENDMISSING,
					"element end tag </{0}> expected"),
					elementName.getRawName());

				errorDetected(Fatal, errMsg, EXML_ELEMENTENDMISSING);
			}
		}

		if(m_features.m_bDoValidityChecks && m_pCurrentElementContentSpec)
		{
			if(!m_pCurrentElementContentSpec->validated())
			{
				m_pCurrentElementContentSpec->validate(*this);
			}

			if(!m_pCurrentElementContentSpec->isEndElementPermitted())
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_PREMATUREELEMENTEND,
					"premature end to content of element '{0}'. Expecting child element {1}"),
					elementName.getRawName(),
					m_pCurrentElementContentSpec->getDisplayableRequiredList());

				errorDetected(Error, errMsg, EXML_PREMATUREELEMENTEND);
			}
		}

		if(areCallbacksPermitted() && m_npContentEventHandler)
		{
			m_npContentEventHandler->onEndElement(elementName);
		}

		//
		// Restore the namespace stack to what it was (if it was changed)
		//
		if(bAnyNamespaceDeclarations)
		{
			reportEndNamespacePrefixes();
			m_namespaceFrameVector.pop_back();
		}
	}

	return bElementFound;
}

//==============================================================================
// ParserImpl::elementStartValidityChecks
//
//==============================================================================
void ParserImpl::elementStartValidityChecks(const QName& elementName, bool bRootElement, bool bEmptyElement, const ElementType* pElementType)
{
	QC_DBG_ASSERT(m_features.m_bDoValidityChecks);

	// Validity Constraint: Root Element Type
	// Note: we check for an empty DTD name to accomodate an injected
	// DTD that will not have a name.
	if(bRootElement && m_bHasDTD && !m_DTDName.empty() && elementName.getRawName() != m_DTDName)
	{
		const String& errMsg = MessageFormatter::Format(
			System::GetSysMessage(sXML, EXML_ROOTNAMEMISMATCH,
			"the root element '{0}' must match the name '{1}' from the document type declaration"),
			elementName.getRawName(), m_DTDName);

		errorDetected(Error, errMsg, EXML_ROOTNAMEMISMATCH);
	}
	else if(bRootElement && !m_bHasDTD)
	{
		const String& errMsg = System::GetSysMessage(sXML, EXML_NODTD,
			"validation is not possible without a DTD");

		errorDetected(Error, errMsg, EXML_NODTD);
	}

	if(m_pCurrentElementContentSpec)
	{
		if(!m_pCurrentElementContentSpec->validated())
		{
			m_pCurrentElementContentSpec->validate(*this);
		}

		const ElementContentSpec* pNextSpec = m_pCurrentElementContentSpec->validateNextElement(elementName.getRawName());
		if(pNextSpec == NULL)
		{
			String nextList = m_pCurrentElementContentSpec->getDisplayableNextList(m_currentElementName);

			String errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_INVALIDCONTENT,
				"element content invalid. Element '{0}' is not expected here, expecting {1}"),
				elementName.getRawName(),
				nextList);

			errorDetected(Error, errMsg, EXML_INVALIDCONTENT);
		}
		m_pCurrentElementContentSpec = pNextSpec;
	}

	if(pElementType)
	{
		//
		// From XML 1.0, 3.1: "for interoperability, the empty-element tag
		// should be used, and should only be used, for elements which are
		// declared EMPTY".
		//
		if(m_features.m_bDoInteroperabilityChecks)
		{
			if(bEmptyElement && pElementType->getContentType() != ElementType::EMPTY)
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_EMPTYSYNTAXUSED,
					"for interoperability, the empty element syntax <{0}/> should only be used for elements declared EMTPY"),
					elementName.getRawName());

				errorDetected(Error, errMsg, EXML_EMPTYSYNTAXUSED);
			}
			else if(!bEmptyElement && pElementType->getContentType() == ElementType::EMPTY)
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_EMPTYSYNTAXNOTUSED,
					"for interoperability, the empty element syntax <{0}/> should be used for elements declared EMTPY"),
					elementName.getRawName());

				errorDetected(Error, errMsg, EXML_EMPTYSYNTAXNOTUSED);
			}
		}
	}
	else if(m_bHasDTD)
	{
		//
		// Validity Constraint: Element Valid
		//
		const String& errMsg = MessageFormatter::Format(
			System::GetSysMessage(sXML, EXML_ELEMENTNOTDECLARED,
			"undeclared element '{0}'"),
			elementName.getRawName());

		errorDetected(Error, errMsg, EXML_ELEMENTNOTDECLARED);
	}
}

//=============================================================================
// ParserImpl::parseSTag
// 
// Parse an STag or EmptyElemTag
//
// This function just does the parsing, it doesn't do any validation or
// namespace expansion - that is performed within parseElement()
//=============================================================================
bool ParserImpl::parseSTag(QName& elementName,
                           bool& bEmptyElement,
                           const ElementType*& pElementType,
                           AttributeSet& attrSet)
{
	bool bSTagFound = false;
	bEmptyElement = false;

	//
	// An element tag must start '<' followed by Name, otherwise it could
	// be some other markup.
	//
	if(Scanner::SkipNextCharConstant(m_scannerPos, cOpenAngle))
	{
		bSTagFound = true;
		//
		// parseQName() must return something because we have already
		// tested the input stream for a name char
		//
		parseQName(elementName, sElementName, true, false);

		ElementMap::const_iterator elementIter = m_elementMap.find(elementName.getRawName());
		if(elementIter != m_elementMap.end())
		{
			pElementType = (*elementIter).second.get();
		}

		parseAttributeList(pElementType, attrSet, '/');

		//
		// parseAttributeList() eats white-space, so we should now be pointing
		// at the end of this tag
		//
		// We now expect either a "/>" for an empty element or ">" for a tag
		// with content.
		//

		if(Scanner::SkipNextStringConstant(m_scannerPos, sEmptyElemClose))
		{
			bEmptyElement = true;
		}
		else if(!Scanner::SkipNextCharConstant(m_scannerPos, cCloseAngle))
		{
			// Yikes! here we go again.  The xml author has made a mistake
			// and we have to pick up the pieces

			// report the error

			const String& errMsg = System::GetSysMessage(sXML, EXML_BADSTAGTERMINATION,
				"element start tag not terminated correctly. Expected '>' or '/>'");

			errorDetected(Fatal, errMsg, EXML_BADSTAGTERMINATION);

			//
			// Luckily this is standard stuff and there is a function we can call to
			// do the dirty work!
			const char* endTokens[] = {szEmptyElemClose, szCloseAngle};
			recoverPosition(2, endTokens, 0);
		}
	}

	return bSTagFound;
}

//=============================================================================
// ParserImpl::parseETag
// 
// [42] ETag ::= '</' Name S? '>'
//
//=============================================================================
bool ParserImpl::parseETag(const QName& elementName)
{
	bool bEtokenFound = false;

	if(Scanner::SkipNextStringConstant(m_scannerPos, sStartETag))
	{
		bEtokenFound = true;

		//
		// additional (friendly) test to see if illegal white-space has
		// been provided where the element name is expected
		//
		if(CharTypeFacet::IsWhiteSpace(Scanner::PeekNextCharacter(m_scannerPos)))
		{
			const String& errMsg = System::GetSysMessage(sXML, EXML_ETAGSYNTAXWS,
				"unexpected white space before name in element end tag");
			
			errorDetected(Fatal, errMsg, EXML_ETAGSYNTAXWS);
			Scanner::SkipWhiteSpace(m_scannerPos);
		}

		QName eTagName;
		/*bool bNameFound = */ parseQName(eTagName, sElementName, true, false);

		if(eTagName != elementName)
		{
			// namePos may not be at the right position, but we dont want to keep getting
			// these things do we?
			const String& errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_ENDTAGMISMATCH,
				"end tag '</{0}>' does not match start tag.  Expected '</{1}>'"),
				eTagName.getRawName(),
				elementName.getRawName());

			errorDetected(Fatal, errMsg, EXML_ENDTAGMISMATCH);
		}

		Scanner::SkipWhiteSpace(m_scannerPos);

		//
		// Next character in the stream should be '>'
		//
		if(!Scanner::SkipNextCharConstant(m_scannerPos, cCloseAngle))
		{
			// yikes! Here we go again!
			// report the error
			const String& errMsg = System::GetSysMessage(sXML, EXML_ETAGNOTTERMINATED,
				"element end tag not terminated correctly. Expected '>'");
			
			errorDetected(Fatal, errMsg, EXML_ETAGNOTTERMINATED);

			// .. and recover position
			recoverPosition(1, &szCloseAngle, 0);
		}
	}
	
	return bEtokenFound;
}

//=============================================================================
// ParserImpl::parseXMLDecl
// 
// Prrior to calling, all white-space has been removed from the start of
// the input stream.
//
// [23] XMLDecl ::= '<?xml' VersionInfo EncodingDecl? SDDecl? S? '?>'
// [24] VersionInfo ::= S 'version' Eq (' VersionNum ' | " VersionNum ")
// [25] Eq ::= S? '=' S?
// [26] VersionNum ::= ([a-zA-Z0-9_.:] | '-')+
//
// You will notice that special processing is not really required to parse
// the contents of the XMLDecl - because it is the same syntax as for an
// atribute list.  However, attribue order is not normally important
// whereas it is defined by the standard so we will check it.
//
//=============================================================================
bool ParserImpl::parseXMLDecl()
{
	bool bXmlDeclFound = false;
	
	String version, encoding;
	bool bStandalone=false;

	size_t startOffset = m_scannerPos.getStreamPosition().getOffset();

	if(Scanner::SkipNextStringConstant(m_scannerPos, sXMLDecl))
	{
		bXmlDeclFound = true;

		if(m_bXMLDeclSeen)
		{
			const String& errMsg = System::GetSysMessage(sXML, EXML_MULTIPLEXMLDECLS,
				"there can be only one XML declaration");
			
			errorDetected(Fatal, errMsg, EXML_MULTIPLEXMLDECLS);
		}
		else
		{
			m_bXMLDeclSeen = true;
		}

		if(startOffset > 0)
		{
			const String& errMsg = System::GetSysMessage(sXML, EXML_XMLDECLNOTATSTART,
				"XML declaration not at start");

			errorDetected(Fatal, errMsg, EXML_XMLDECLNOTATSTART);
		}

		//
		// The next thing we want is the list of attributes associated with
		// the xml declaration.
		//
		// Note: parseAttributeList() eats white space
		//
		AttributeSetImpl attrSet;
		if(parseAttributeList(NULL, attrSet, '?'))
		{
			int nVersionIndex, nEncodingIndex, nStandaloneIndex;
			nVersionIndex = nEncodingIndex = nStandaloneIndex = -1;

			for(size_t i=0; i<attrSet.size(); i++)
			{
				const AutoPtr<Attribute>& rpAttr = attrSet.getAttribute(i);
				const String& name = rpAttr->getName().getRawName();

				if(name == sVersion)
				{
					nVersionIndex = rpAttr->getIndex();
					version = rpAttr->getValue();
					
					//
					// Even though XML 1.0 specifies the permitted characters
					// in [26] VersionNum ::= ([a-zA-Z0-9_.:] | '-')+
					// We are required to output a fatal error if the version is not
					// recognized.
					// We currently only recognize version 1.0, so test for that...
					//
					if(version != QC_T("1.0"))
					{
						const String& errMsg = MessageFormatter::Format(
							System::GetSysMessage(sXML, EXML_UNSUPPORTEDVERSION,
							"XML version {0} is not supported"),
							version);

						errorDetected(Fatal, errMsg, EXML_UNSUPPORTEDVERSION);
					}
				}
				else if(name == sEncoding)
				{
					nEncodingIndex = rpAttr->getIndex();
					encoding = rpAttr->getValue();
				}
				else if(name == sStandalone)
				{
					nStandaloneIndex = rpAttr->getIndex();
					if(rpAttr->getValue() == QC_T("yes"))
						bStandalone = true;
					else if (rpAttr->getValue() != QC_T("no"))
					{
						const String& errMsg = MessageFormatter::Format(
							System::GetSysMessage(sXML, EXML_INVALIDSTANDALONE,
							"invalid standalone value '{0}'.  Must be 'yes' or 'no'"),
							rpAttr->getValue());

						errorDetected(Fatal, errMsg, EXML_INVALIDSTANDALONE);
					}
				}
				else
				{
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_UNKNOWNXMLATTR,
						"unknown attribute '{0}' within XML declaration"),
						rpAttr->getName().getRawName());
					
					errorDetected(Fatal, errMsg, EXML_UNKNOWNXMLATTR);
				}
			}

			if(nVersionIndex == -1)
			{
				const String& errMsg = System::GetSysMessage(sXML, EXML_MISSINGVERSION,
					"missing 'version' from XML declaration");
				
				errorDetected(Fatal, errMsg, EXML_MISSINGVERSION);
			}

			//
			// Check that attributes are declared in the required order
			//
			if ((nEncodingIndex != -1 && nEncodingIndex < nVersionIndex)
			||  (nStandaloneIndex != -1 && nStandaloneIndex < nEncodingIndex)
			||  (nStandaloneIndex != -1 && nStandaloneIndex < nVersionIndex))
			{
				const String& errMsg = System::GetSysMessage(sXML, EXML_XMLATTRORDER,
					"attribute order invalid within XML declaration");
				
				errorDetected(Fatal, errMsg, EXML_XMLATTRORDER);
			}
		}

		//
		// We have parsed the entire xml declaration up to the point of the
		// terminator "?>".
		//
		// Should be simple but... who knows what's really lurking ahead in the
		// input stream?
		//
		if(!Scanner::SkipNextStringConstant(m_scannerPos, sPIEnd))
		{
			// report the error
			reportDeclTermError(sXML, sPIEnd);

			// and attempt to recover position...
			recoverPosition(1, &szPIEnd, 0);
		}

		//
		// okay, the xml declaration has been parsed.  We're not too sure at this point
		// if an error was found in the processing of the declaration - but that doesn't
		// matter, we are only interested in whether or not any serious errors have been
		// found in the document.  If not, then call the declaration callback.
		//
		m_bStandaloneDoc = bStandalone;

		if(areCallbacksPermitted() && m_npContentEventHandler)
		{
			m_npContentEventHandler->onXMLDeclaration(version, encoding, bStandalone);
		}
	}

	return bXmlDeclFound;
}


//=============================================================================
// ParserImpl::parseComment
// 
// [15] Comment ::= '<!--' ... '-->'
// 
// Comments can contain any characters except for two consecutive dashes.
//
// This has a bearing on attempting to recover position - we simply can't.  We
// have to keep scanning until the '-->' terminator is found.
//
// Note: comments can appear within DTDs, but Parameter Entity references
// are not recognized within comments.
//
// While parsing the actual comment we use Scanner::GetNextContigString which
// will only return characters from a single entity. 
//=============================================================================
bool ParserImpl::parseComment()
{
	bool bCommentFound = false;

	if(Scanner::SkipNextStringConstant(m_scannerPos, sCommentStart))
	{
		bCommentFound = true;

		if((m_features.m_bReportDTDComments && m_bParsingDTD || m_features.m_bReportDocumentComments && !m_bParsingDTD))
		{
			if(areCallbacksPermitted() && m_npContentEventHandler)
			{
				m_npContentEventHandler->onCommentStart(m_bParsingDTD);
			}
		}

		//
		// Even though we are using std Scanner functions, it is still possible
		// that a comment is started in a PE but unfinished.   So we do normal
		// PE nesting tests.
		//
		const Entity& startEntity = m_scannerPos.getEntity();

		while(true)
		{
			Character nextChar;

			const BufferRange& range = Scanner::GetNextContigString(m_scannerPos,
																	CharTypeFacet::ValidChar,
																	0,
																	sDashDash,
																	nextChar);

			
			if(range.getSize() && ( m_features.m_bReportDTDComments && m_bParsingDTD || m_features.m_bReportDocumentComments && !m_bParsingDTD ))
			{
				if(areCallbacksPermitted() && m_npContentEventHandler)
				{
					for(size_t i=0; i<range.getNumFragments(); i++)
					{
						const BufferRange::Fragment& fragment = range.getFragment(i);
						if(fragment.length)
						{
							m_npContentEventHandler->onComment(fragment.pData, fragment.length);
						}
					}
				}
			}

			if(nextChar == cDash)
			{
				if(Scanner::SkipNextStringConstant(m_scannerPos, sCommentEnd))
				{
					break;
				}
				else
				{
					// [E63] For compatability it is illegal for a comment to 
					// contain the string "--"
					const String& errMsg = System::GetSysMessage(sXML, EXML_COMMENTDASHES,
						"comment text may not contain '--'");

					errorDetected(Fatal, errMsg, EXML_COMMENTDASHES);

					Scanner::Skip(m_scannerPos, 1); //skip one dash
				}
			}
			else if(!m_features.m_bReportPEBoundaries && nextChar.isEOF() && Scanner::SkipSoftEOF(m_scannerPos))
			{
				// do nothing, this represents a change of PE which
				// may be caught by the nested entity VC.
			}
			else
			{
				unexpectedChar(nextChar, sComment);
				if(nextChar.isEOF())
				{
					break;
				}
				else
				{
					Scanner::Skip(m_scannerPos, 1); //skip the bad character
				}
			}
		}
		
		if(m_features.m_bDoValidityChecks)
		{
			validatePENesting(startEntity, sComment);
		}

		if((m_features.m_bReportDTDComments && m_bParsingDTD || m_features.m_bReportDocumentComments && !m_bParsingDTD))
		{
			if(areCallbacksPermitted() && m_npContentEventHandler)
			{
				m_npContentEventHandler->onCommentEnd(m_bParsingDTD);
			}
		}
	}
	
	return bCommentFound;
}

//=============================================================================
// ParserImpl::parsePI
// 
// [16] PI ::= '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>'
// [17] PITarget ::= Name - (xml/XML/etc)
// 
//=============================================================================
bool ParserImpl::parsePI()
{
	//
	// I think it's safe to say that if we have the string '<?' in the input
	// stream then we are looking at a processing indstruction.  The only
	// other production that looks like this is the xmldecl, and we will have
	// already processed that.
	//
	// The specification states that PITarget is a name - but it should
	// not start with "xml" - as those names are reserved.  However, as
	// this parser may be used to parse documents with valid xml extensions
	// then it really is not up to use to police this.
	//
	bool bPIFound = false;
	
	if(Scanner::SkipNextStringConstant(m_scannerPos, sPIStart))
	{
		bPIFound = true;

		const Entity& startEntity = m_scannerPos.getEntity();

		const String& PITarget = parseName(sPITarget, true, false, false);

		if(0 == PITarget.size())
		{
			// oops, into error territory.  parseName will already have reported the error
			// attempt to recover to next grammar production
			recoverPosition(1, &szPIEnd, 0);
		}
		else
		{
			if(StringUtils::CompareNoCase(PITarget, sXML) == 0)
			{
				const String& errMsg = System::GetSysMessage(sXML, EXML_INVALIDPITARGET,
					"'{0}' is not a valid processing instruction target");
				
				errorDetected(Fatal, errMsg, EXML_INVALIDPITARGET);
			}

			// The grammar states that a PI should have leading
			// white space - so lets check it.  The SAX and DOM APIs state
			// that the data does not contain the white space that separates the
			// data from the target.
			Character nextChar = Scanner::PeekNextCharacter(m_scannerPos);

			if(Scanner::SkipWhiteSpace(m_scannerPos) == 0 &&
			   nextChar != '?')
			{
				unexpectedChar(nextChar, QC_T("PITarget"));
			}

			const String& PIValue = 
				Scanner::GetNextStringDelimited(m_scannerPos,
				                                CharTypeFacet::ValidChar,
												0,
												sPIEnd,
												nextChar);

			//
			// IF the nextChar is the first char of our delimiter then
			// we know that we found a valid string (even if the string
			// is of zero size!)
			//
			if(nextChar == '?')
			{
				// point past the PI end tag
				// Note: this cannot fail!
				Scanner::SkipNextStringConstant(m_scannerPos, sPIEnd);

				if(areCallbacksPermitted() && m_npContentEventHandler)
				{
					m_npContentEventHandler->onPI(PITarget, PIValue);
				}
			}
			else
			{
				unexpectedChar(nextChar, QC_T("processing instruction"));
				// .. and attempt to recover to next grammar production
				recoverPosition(1, &szPIEnd, 0);
			}
		}

		if(m_features.m_bDoValidityChecks)
		{
			validatePENesting(startEntity, sPI);
		}
	}

	return bPIFound;
}

//=============================================================================
// ParserImpl::parseDocTypeDecl
// 
// This parses the DTD for a document - and is therefore quite involved
// 
// [28]  doctypedecl ::= '<!DOCTYPE' S Name (S ExternalID)? S?
//                      ('[' intSubset ']' S?)? '>'
//
// [28a] DeclSep     ::= PEReference | S
//
// [28b] intSubset   ::= (markupdecl | DeclSep)*  
//
// [29] markupdecl ::= elementdecl | AttlistDecl | EntityDecl
//                     | NotationDecl | PI | Comment
//
// [75] ExternalID ::= 'SYSTEM' S SystemLiteral | 
//                     'PUBLIC' S PubidLiteral S SystemLiteral
//
// The DTD may consist of an Internal and an External subset.  If there is
// an Internal subset then that is processed first.  This means, amongst
// other things, that parameter entities declared in the External subset cannot be
// referenced in the internal subset (because they don't exist yet).
//
//=============================================================================
bool ParserImpl::parseDocTypeDecl()
{
	bool bDTDFound = false;

	//
	// Scanner::SkipNextStringConstant will return true and advance the position
	// if the passed string is at the current location...
	//
	if(Scanner::SkipNextStringConstant(m_scannerPos, sDoctypeDecl))
	{
		bDTDFound = true;

		m_bHasDTD = true;
		
		QName dtdQName;
		if(parseQName(dtdQName, sDTDName, true, true))
		{
			m_DTDName = dtdQName.getRawName();
		}

		Scanner::SkipWhiteSpace(m_scannerPos);

		//
		// An external ID is required if the next character is not "[" or ">"
		//
		Character nextChar = Scanner::PeekNextCharacter(m_scannerPos);
		bool bExternalIDReqd = !(nextChar == cOpenBrace || nextChar == cCloseAngle);
		//
		String systemId, publicId;
		bool bExternalDTD = parseExternalID(systemId, publicId, bExternalIDReqd, false, sDoctypeDeclaration);

		//
		// Call the DTD event handler if one has been registered
		//
		if(m_npDTDEventHandler)
		{
			m_npDTDEventHandler->onStartDTD(m_DTDName, publicId, systemId);
		}

		Scanner::SkipWhiteSpace(m_scannerPos);

		// See if we have an Internal Subset
		if(Scanner::SkipNextCharConstant(m_scannerPos, cOpenBrace))
		{
			parseDTDMarkup(true /*internal subset*/,
			               false /* conditional section */,
			               false /* expect EOF during parse*/);

			Scanner::SkipNextCharConstant(m_scannerPos, cCloseBrace);
		}

		Scanner::SkipWhiteSpace(m_scannerPos);

		if(!Scanner::SkipNextCharConstant(m_scannerPos, cCloseAngle))
		{
			// yikes, report and recover time
			nextChar = Scanner::PeekNextCharacter(m_scannerPos);
			unexpectedChar(nextChar, sDTD);
			recoverPosition(1, &szCloseAngle, 0);
		}

		//
		// Now that we have parsed the internal subset, test to see
		// if an External DTD subset has been referenced, and if it
		// has then process it
		//
		// However, if a DTD override has been specified then that takes
		// precedence over the values from the input document
		// 
		if(m_rpDTDOverride)
		{
			parseExtDTDSubsetOverride();
		}
		else if(bExternalDTD)
		{
			parseExtDTDSubset(publicId, systemId);
		}
		
		//
		// If we are a validating parser, then test that the DTD
		// is valid!
		//
		if(m_features.m_bDoValidityChecks)
		{
			validateDTD();
		}

		//
		// Call the DTD event handler if one has been registered
		//
		if(m_npDTDEventHandler)
		{
			m_npDTDEventHandler->onEndDTD();
		}

	}

	return bDTDFound;
}

//==============================================================================
// ParserImpl::parseExtDTDSubset
//
// Parse the external DTD subset (as declared in the document's
// DOCTYPE declaration).
//
//==============================================================================
void ParserImpl::parseExtDTDSubset(const String& publicId, const String& systemId)
{
	m_bExternalDTDSubsetDeclared = true;

	//
	// The External DTD Subset will not be read if PE processing
	// is disabled
	//
	if(m_features.m_bProcessExternalParameterEntities)
	{
		m_rpDTDEntity = 
				new ExternalEntity(*this, EntityType::DTD, &m_scannerPos.getEntity(),
				                   !parsingInternalDTDSubset(),
				                   m_DTDName, 
				                   publicId, systemId, sNull);

		parseExtDTDEntity();
	}
}

//==============================================================================
// ParserImpl::parseExtDTDSubsetOverride
//
// Parse the external overridden external DTD
//
//==============================================================================
void ParserImpl::parseExtDTDSubsetOverride()
{
	m_bExternalDTDSubsetDeclared = true;
	//
	// The External DTD Subset will not be read if PE processing
	// os disabled
	//
	if(m_features.m_bProcessExternalParameterEntities)
	{
		m_rpDTDEntity = new ExternalEntity(*this, EntityType::DTD, m_rpDTDOverride.get());

		parseExtDTDEntity();
	}
}

//==============================================================================
// ParserImpl::parseExtDTDEntity
//
// Once the DTD External Entity has been resolved, parse it...
//
// [30] extSubset ::= TextDecl? extSubsetDecl
// [31] extSubsetDecl ::= (markupdecl | conditionalSect | PEReference | S)*
//
//==============================================================================
void ParserImpl::parseExtDTDEntity()
{
	//
	// Replace the ScannerPosition with another one for the external Entity
	//
	ValueRestorer<ScannerPosition> 
		marker(m_scannerPos, m_rpDTDEntity->getStartPosition());

	if(areCallbacksPermitted() && m_npEntityEventHandler)
	{
		m_npEntityEventHandler->onStartEntity(m_DTDName, EntityType::DTD);
	}

	//
	// And parse it
	// 
	parseDTDMarkup(false /*internal subset*/,
	               false /* conditional section */,
	               true /* expect EOF during parse*/);

	if(areCallbacksPermitted() && m_npEntityEventHandler)
	{
		m_npEntityEventHandler->onEndEntity(m_DTDName, EntityType::DTD);
	}


}

//=============================================================================
// ParserImpl::parseDTDMarkup
// 
// Parse the interesting bits within the DTD starting 
//
// [28] doctypedecl ::= '<!DOCTYPE' S Name (S ExternalID)? S?
//                      ('[' (markupdecl | PEReference | S)* ']' S?)? '>'
//
// [29] markupdecl ::= elementdecl | AttlistDecl | EntityDecl
//                     | NotationDecl | PI | Comment
//
// [69] PEReference ::= '%' Name ';'
//
//=============================================================================
bool ParserImpl::parseDTDMarkup(bool bInternalSubset,
								bool bConditionalSection,
								bool bExpectEOF)
{
	bool bDTDMarkupFound = false;

	ValueRestorer<bool> marker2(m_bParsingDTD, true);

	bool bContinue = true;
	
	// setting bExpandPEs to true will cause the get/peekxxxEx() functions
	// to expand parameter entities in-place

	while(bContinue)
	{
		Scanner::SkipWhiteSpace(m_scannerPos);

		if(Scanner::PeekNextCharacter(m_scannerPos) == cCloseBrace && (bInternalSubset | bConditionalSection))
		{
			// hurrah! We have completed parsing the internal subset
			bContinue = false;
		}
		else
		{
			String strToken;
			bool bErrorReported(false);
			const int nextToken = testNextTokenType(PrologTable, strToken, bErrorReported);
			bool bTokenFound;

			switch(nextToken)
			{
			case Comment:
				bTokenFound = parseComment();
				QC_DBG_ASSERT(bTokenFound);
				break;
			case PI:
				bTokenFound = parsePI();
				QC_DBG_ASSERT(bTokenFound);
				break;
			case EntityDecl:
				bTokenFound = parseEntityDecl();
				QC_DBG_ASSERT(bTokenFound);
				break;
			case NotationDecl:
				bTokenFound = parseNotationDecl();
				QC_DBG_ASSERT(bTokenFound);
				break;
			case ElementDecl:
				bTokenFound = parseElementDecl();
				QC_DBG_ASSERT(bTokenFound);
				break;
			case AttlistDecl:
				bTokenFound = parseAttlistDecl();
				QC_DBG_ASSERT(bTokenFound);
				break;
			case ConditionalDecl:
				bTokenFound = parseConditionalDecl();
				QC_DBG_ASSERT(bTokenFound);
				break;
			case StartElement:
				{
					const String& errMsg = System::GetSysMessage(sXML, EXML_DTDNOTTERMINATED,
						"DTD not terminated correctly");
					
					errorDetected(Fatal, errMsg, EXML_DTDNOTTERMINATED);
				}
				bContinue = false;
				break;
			case EndOfEntity:
				// EOFs are expected for the External Subset, but not the internal one
				if(!bExpectEOF)
				{
					const String& expected = bConditionalSection 
					                       ? sCDATAEnd
					                       : sDTDMarkup;
					unexpectedToken(nextToken, strToken, expected);
					// not recovery necessary at eof
				}
				bContinue = false;
				break;
			case PEReference:
				parsePEReference(true /*markup decl*/,
				                 false /*return a soft EOF at end*/,
								 true /* name must follow % */);
				break;
			default:
				bContinue = false;
				// and fall thru to report error
			case 0:
				if(!bErrorReported)
				{
					unexpectedToken(nextToken, strToken, QC_T("DTD markup"));
				}
				Scanner::SkipNextStringConstant(m_scannerPos, strToken);
				const char* endDelimiters[2] = {szCloseAngle, szCloseBrace};
				recoverPosition(2, endDelimiters, 0); // eat them all
				break;
			}
		}
	}

	return bDTDMarkupFound;
}

//==============================================================================
// ParserImpl::parsePEDeclarations
//
// Called when a PE reference is detected at the declaration level within a
// DTD.  For example:-
// <!DOCTYPE x [
// <!ENTITY % pe SYSTEM "pe.ent">
// %pe; ]>
//
//==============================================================================
void ParserImpl::parsePEDeclarations()
{
	//
	// Remember the entity in effect before the pe reference
	//
	const Entity& startEntity = m_scannerPos.getEntity();

	if(parsePEReference(true /*markup decl*/,
	                    true /*return a soft EOF at end*/,
						true /* name mustr follow %*/))
	{
		const Entity& PEEntity = m_scannerPos.getEntity();

		//
		// If ParsePEreference has updated the position, then we either have
		// an internal pe entity, or its external and we have been configured
		// to read external entities.
		//
		if(PEEntity != startEntity)
		{
			if(areCallbacksPermitted() && m_npEntityEventHandler)
			{
				m_npEntityEventHandler->onStartEntity(PEEntity.getName(),
													  EntityType::Parameter);
			}

			parseDTDMarkup(false /*internal subset*/,
						   false /* conditional section */,
						   true /* expect EOF during parse*/);

			//
			// Erratum E21 introduced production 28a for DeclSep
			// which requires PEs used as DeclSep to contain complete
			// markup declarations
			//
			if(m_scannerPos.getEntity() != PEEntity)
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_MARKUPDECL,
					"parameter entity %{0}; does not contain complete markup declarations"),
					PEEntity.getName());

				errorDetected(Fatal, errMsg, EXML_MARKUPDECL);
			}

			Scanner::SkipSoftEOF(m_scannerPos);

			if(areCallbacksPermitted() && m_npEntityEventHandler)
			{
				m_npEntityEventHandler->onEndEntity(PEEntity.getName(),
													EntityType::Parameter);
			}
		}
	}
}

//=============================================================================
// ParserImpl::parseEntityDecl
// 
// [70] EntityDecl ::= GEDecl | PEDecl
// [71] GEDecl ::= '<!ENTITY' S Name S EntityDef S?'>'
// [72] PEDecl ::= '<!ENTITY' S '%' S Name S PEDef S?'>'
// [73] EntityDef ::= EntityValue | (ExternalID NDataDecl?)
// [74] PEDef ::= EntityValue | ExternalID
// [09] EntityValue ::= '"' ([^%&"] | PEReference | Reference)* '"'
//                    | "'" ([^%&'] | PEReference | Reference)* "'"
// [76] NDataDecl ::= S 'NDATA' S Name
//
//=============================================================================
bool ParserImpl::parseEntityDecl()
{
	bool bEntityDeclFound = false;

	if(Scanner::SkipNextStringConstant(m_scannerPos, sEntityDecl))
	{
		bEntityDeclFound = true;

		bool bParameterEntity = false;

		//
		// This is a particularly strange place to do parsing.  If we come across
		// a "%" followed by white-space then we have a PE declaration, otherwise
		// we have a PE reference.
		//
		// set a scope for the value restorer
		{
			ValueRestorer<bool> marker(m_bParsingEntityDeclName, true);
			if(!skipWhiteSpaceEx())
			{
				reportMissingWhiteSpaceBefore(sEntityName, sEntityDeclaration);
			}

			if(skipNextCharConstantEx('%'))
			{
				bParameterEntity = true;
				skipRequiredWhitespaceBefore(sPEName, sEntityDeclaration);
			}
		}

		EntityMap& map = (bParameterEntity) ? m_peMap
		                                    : m_geMap;

		EntityType type = (bParameterEntity) ? EntityType::Parameter
		                                     : EntityType::General;

		const String& name = parseName(sEntityName, true, false, false);

		if(name.size() && (map.find(name) != map.end()))
		{
			// Note: duplicate declarations are nothing more than a Warning
			// From XML 1.0 Section 4.2
			//
			// It is a little difficult to create a useful warning here.  It is
			// common for dulicate entity declarations to occur - especially
			// parameter entities that are declared with a default value
			// somewhere and overriden elsewhere - perhaps in the internal
			// subset.
			//
			// We could warn if the same entity is declared twice within the
			// same external entity, but that isn't particularly useful, and would
			// require us to store the entity (id) of where each entity was declared.
			// An alternative, is to warn if we are processing the Internal DTD
			// subset, as most users would expect the first declaration to appear
			// here.
			//
			// This warning will be triggered if the entity is declared first
			// by an external PE, envoked from the internal subset, and then 
			// re-declared in the internal DTD subset.
			//
			
			if(m_features.m_bDoValidityChecks && m_features.m_bDoWarningChecks && 
			   parsingInternalDTDSubset())
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_ENTITYDECLARED,
					"entity '{0}' has already been declared"),
					getDisplayEntityName(name, type));

				errorDetected(Warning, errMsg, EXML_ENTITYDECLARED);
			}
		}

		skipRequiredWhitespaceAfter(sEntityName, sEntityDeclaration);

		//
		// Okay, what follows may be an EntityValue, or an ExternalID ( followed 
		// by an optional NDataDecl )
		//
		// An external ID is required if the next character is not '"' or "'"
		//
		Character nextChar = peekNextCharacterEx();
		bool bExternalIDReqd = !(nextChar == cQuote || nextChar == cApos);

		String systemLiteral, pubidLiteral, notation;
		bool bParsedEntity = true;
		if(parseExternalID(systemLiteral, pubidLiteral, bExternalIDReqd, false, sEntityDeclaration))
		{
			const size_t wsCount = skipWhiteSpaceEx();
			ScannerPosition wsPos = m_scannerPos;
			
			// General Entities with ExternalIDs may have an optional NDataDecl
			if(skipNextStringConstantEx(sNDATA))
			{
				// Parameter Entities must conform to PEDef which doesn't allow
				// for an NDATA declaration
				if(bParameterEntity)
				{
					const String& errMsg = System::GetSysMessage(sXML, EXML_NDATAERROR,
						"the 'NDATA' keyword is not permitted in parameter entity declarations");
					
					errorDetected(Fatal, errMsg, EXML_NDATAERROR);
				}
				else
				{
					bParsedEntity = false;
				}

				if(wsCount == 0)
				{
					reportMissingWhiteSpaceBefore(sNDATA, sEntityDeclaration);
				}

				skipRequiredWhitespaceAfter(sNDATA, sEntityDeclaration);
				
				notation = parseName(sNotationName, true, false, false);

				// Note: the notation cannot be checked until the entire DTD is loaded

				skipWhiteSpaceEx();
			}

			//
			// The first reference to an entity is the master - so we must check to
			// see if it has been declared before.  This is not an error.
			// 
			if(map.find(name) == map.end())
			{
				//
				// If there is a DTDEventHandler, then inform it of the entity declaration
				//
				if(m_npDTDEventHandler)
				{
					if(bParsedEntity)
					{
						m_npDTDEventHandler->onExternalEntityDecl(name, type, pubidLiteral, systemLiteral);
					}
					else
					{
						m_npDTDEventHandler->onUnparsedEntityDecl(name, pubidLiteral, systemLiteral, notation);
					}
				}

				// xml 1.0, section 2.9 states that a decl is externally declared
				// if it is not internal subset or PE.  This means in effect that
				// the entity must be the document entity for it to be internal for the
				// standalone rules.
				bool bExternalDeclaration = (m_scannerPos.getEntity().getType() != EntityType::Document);

				ExternalEntity* pExtEntity = 
					new ExternalEntity(*this, type, &m_scannerPos.getEntity(),
					                   bExternalDeclaration,
					                   name, pubidLiteral,
					                   systemLiteral, notation);

				map[name] = pExtEntity;
			}
		}
		else if(parseEntityValue(name, bParameterEntity))
		{
			// nothing more needs to be done, the entity will have been
			// stored away in the entity map for use later
			skipWhiteSpaceEx();
		}
		else
		{
			// Error: unrecognized Entity syntax
			nextChar = peekNextCharacterEx();
			unexpectedChar(nextChar, sEntityDeclaration);
			// .. and attempt to recover to next grammar production
			recoverPosition(1, &szCloseAngle, 1); // don't eat the close angle though
		}

		// Finally, test if the Entity declaration is correctly terminated
		if(!skipNextCharConstantEx(cCloseAngle))
		{
			// Yikes! here we go again. 
			// report the error
			reportDeclTermError(QC_T("entity"), sCloseAngle);
			
			//
			// Luckily this is standard stuff and there is a function we can call to
			// do the dirty work!
			recoverPosition(1, &szCloseAngle, 0); // and eat it!
		}
	}
	
	return bEntityDeclFound;
}

//=============================================================================
// ParserImpl::parseEntityValue
// 
// [09] EntityValue ::= '"' ([^%&"] | PEReference | Reference)* '"'
//                    | "'" ([^%&'] | PEReference | Reference)* "'"
//
// Entities are complex creatures.  They are probably the most complicated
// piece of xml.  Entities constructed from EntityValues are "parsed entities".
// They are invoked by name using Entity References (&entity;).
//
// Entity values can contain Parameter Entities and character references
// - both of which are resolved immediatly.  They can also contain references
// to other general entities - these are not resolved until the entity is invoked.
//
// Note: Parameter Entities are resolved automatically by using the
// Peek/GetxxxExpanded methods so there is no need to test for "%" here.
//
// Note: There is a WFC in section 2.8 of the spec:
// "in the internal DTD subset, pe references can occur only where markup
//  declarations can occur, not within markup declarations".
// In other words, PEReferences are not allowed if we are parsing the
// internal DTD subset.
//
// Also, the closing quote must come from the same entity as the starting 
// quote, i.e. an embedded PE cannot terminate a quoted string.
//
//=============================================================================
bool ParserImpl::parseEntityValue(const String& name, bool bParameterEntity)
{
	bool bEntityDeclFound = false;

	const Character startQuote = peekNextCharacterEx();

	if(startQuote == '\'' || startQuote == '"')
	{
		ValueRestorer<bool> marker(m_bParsingEntityValue, true);
		
		bool bSimpleData = true;

		getNextCharacterEx(); // skip over start quote

		const Entity& startEntity = m_scannerPos.getEntity();

		bEntityDeclFound = true;
		StreamPosition startPos = m_scannerPos.getStreamPosition();

		AutoBuffer<CharType> entityBuffer;

		Character nextChar;
		
		while(true)
		{
			nextChar = peekNextCharacterEx();

			if(nextChar != startQuote)
			{
				if(nextChar == '&')
				{
					// Parse (but do not resolve) the reference name
					bSimpleData = false;

					const String& reference = 
						parseReference(false, /*do not resolve entities*/
						               false  /* not an attribute */);

					// Add the reference to the entity's stream
					entityBuffer.append(reference.data(), reference.size());
				}
				else if(CharTypeFacet::IsValidChar(nextChar))
				{
					nextChar = getNextCharacterEx();

					entityBuffer.append(nextChar.data(), nextChar.length());

					if(nextChar == cOpenAngle)
					{
						bSimpleData = false;
					}
				}
				else 
				{
					unexpectedChar(nextChar, QC_T("entity value"));
					break;
				}
			}
			else
			{
				// eat the closing quote
				nextChar = getNextCharacterEx();
				// check we are in the same entity that we start in
				if(m_scannerPos.getEntity() == startEntity)
				{
					break;
				}
				else
				{
					entityBuffer.append(nextChar.data(), nextChar.length());
				}
			}
		}

		//
		// The first reference to an entity is the master - so we must check to
		// see if it has been declared before.  This is not an error.
		// 
		EntityMap& map = (bParameterEntity) ? m_peMap : m_geMap;
		EntityType type = (bParameterEntity) ? EntityType::Parameter : EntityType::General;
		if(map.find(name) == map.end())
		{
			if(m_npDTDEventHandler)
			{
				m_npDTDEventHandler->onInternalEntityDecl(name, type, String(entityBuffer.data(), entityBuffer.size()));
			}

			// xml 1.0, section 2.9 states that a decl is externally declared
			// if it is not internal subset or PE.  This means in effect that
			// the entity must be the document entity for it to be internal for the
			// standalone rules.
			bool bExternalDeclaration = (m_scannerPos.getEntity().getType() != EntityType::Document);

			AutoPtr<Reader> rpReader(new StringReader(entityBuffer.data(), entityBuffer.size()));
			map[name] = new InternalEntity(type, &m_scannerPos.getEntity(),
			                               bExternalDeclaration,
										   name, rpReader.get(), startPos,
										   bSimpleData);
		}
	}

	return bEntityDeclFound;
}

//=============================================================================
// ParserImpl::parseContent
// 
// [43] content ::= CharData? ((element | Reference | CDSect | PI | Comment)
//                  CharData?)*
//
//=============================================================================
bool ParserImpl::parseContent(bool bEntityContent)
{
	bool bContinue=true;
	bool bCharDataAllowed = (!m_pCurrentElementType || m_pCurrentElementType->allowsCharData());
	bool bWhitespaceAllowed = (!m_pCurrentElementType || m_pCurrentElementType->allowsWhitespace());

	while(bContinue)
	{
		//
		// EMPTY elements should not have any content.  That includes white space!
		//
		if(!bCharDataAllowed && bWhitespaceAllowed)
		{
			parseIgnorableWhitespace();
		}

		if(parseCharData())
		{
			bContinue = true;
			//
			// Validity constraint: Element valid
			//
			if(!bCharDataAllowed && m_features.m_bDoValidityChecks)
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_CHARCONTENT,
					szCharContentErr),
					m_pCurrentElementType->getName().getRawName());

				errorDetected(Error, errMsg, EXML_CHARCONTENT);
			}
		}
		else
		{
			String strToken;
			bool bErrorReported(false);
			const int nextToken = testNextTokenType(ContentTable, strToken, bErrorReported);
			bool bTokenFound; // just for debugging really
			switch (nextToken)
			{
				case StartElement:
					bTokenFound = parseElement(false);
					QC_DBG_ASSERT(bTokenFound);
					break;
				case EndElement:
					bContinue = false;
					//
					// If the next token is an end element tag "</xx>", then that's
					// fine so long as we aren't parsing an entity's replacement
					// text.  Due to the recusrive nature of this function, only
					// element start tags are permissible.
					//
					if(bEntityContent)
					{
						const String& errMsg = MessageFormatter::Format(
							System::GetSysMessage(sXML, EXML_BADLYFORMEDENTITY,
							"the replacement text of entity '{0}' is badly formed: unexpected end tag"),
							getDisplayEntityName(m_scannerPos.getEntity().getName(), EntityType::General));

						errorDetected(Fatal, errMsg, EXML_BADLYFORMEDENTITY);
					}
					break;
				case CDSect:
					//
					// Validity constraint: Element valid
					//
					if(!bCharDataAllowed && m_features.m_bDoValidityChecks)
					{
						const String& errMsg = MessageFormatter::Format(
							System::GetSysMessage(sXML, EXML_CHARCONTENT,
							szCharContentErr),
							m_pCurrentElementType->getName().getRawName());

						errorDetected(Error, errMsg, EXML_CHARCONTENT);
					}
					bTokenFound = parseCDSect();
					QC_DBG_ASSERT(bTokenFound);
					break;
				case PI:
					bTokenFound = parsePI();
					QC_DBG_ASSERT(bTokenFound);
					break;
				case Comment:
					bTokenFound = parseComment();
					QC_DBG_ASSERT(bTokenFound);
					break;
				case Reference:
					bTokenFound = parseReferenceInContent();
					QC_DBG_ASSERT(bTokenFound);
					break;
				case EndOfEntity:
					bContinue = false;
					if(bEntityContent)
						break;
					// otherwise drop thru...
				case Unknown:
				default:
					if(!bErrorReported)
					{
						String expected = QC_T("element content or </");
						expected += m_currentElementName;
						expected += sCloseAngle;
						unexpectedToken(nextToken, strToken, expected);
					}
					Scanner::SkipNextStringConstant(m_scannerPos, strToken);
					recoverPosition(0, NULL, 0);
					break;
			}
		}
	}

	return true;
}

//==============================================================================
// ParserImpl::parseReferenceInContent
//
//==============================================================================
bool ParserImpl::parseReferenceInContent()
{
	//
	// Now, entity references are funny creatures.  They can be simple
	// character or build-in references OR they can be downright
	// sneaky and contain mark-up of their own.
	//
	// Simple references are returned to us, complex ones
	// are dealt with internally and we receive back an empty string.
	//
	// note: parseReference() eats the &xxx;
	//
	const String& reference = parseReference(true /*resolve entities*/,
											 false /*attribute*/);

	if(!reference.empty())
	{
		bool bWhitespaceAllowed = (!m_pCurrentElementType || m_pCurrentElementType->allowsWhitespace());
		bool bCharDataAllowed = (!m_pCurrentElementType || m_pCurrentElementType->allowsCharData());
		
		if(!bCharDataAllowed && bWhitespaceAllowed && CharTypeFacet::IsCharType(reference, CharTypeFacet::Whitespace))
		{
			if(areCallbacksPermitted() && m_npContentEventHandler)
			{
				m_npContentEventHandler->onIgnorableWhitespace(reference.data(), reference.size());
			}
		}
		else
		{
			//
			// Validity constraint: Element content
			//
			if(!bCharDataAllowed && m_features.m_bDoValidityChecks)
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_CHARCONTENTREF,
					"reference to character content not permitted within element '{0}'"),
					m_pCurrentElementType->getName().getRawName());

				errorDetected(Error, errMsg, EXML_CHARCONTENTREF);
			}
			else
			{
				if(areCallbacksPermitted() && m_npContentEventHandler)
				{
					m_npContentEventHandler->onCharData(reference.data(), reference.size());
				}
			}
		}
	}

	return true;
}

//=============================================================================
// ParserImpl::parseCharData
//
// [14] CharData ::= [^<&]* - ([^<&]* ']]>' [^<&]*)
//
// Note: The XML 1.0 recommendation is a little unclear in places about
// what is permitted in CharData.  For compatability the literal ]]> is not
// permitted within char data - except to mark the end of a CDATA section.
//
// Return values: True is character data is found  and processed, otherwise
//                false (even if a reference is found).
// 
//=============================================================================
bool ParserImpl::parseCharData()
{
	bool bCharDataFound = false;

	while(true)
	{
		Character nextChar;

		const BufferRange& range = Scanner::GetNextContigString(m_scannerPos,
			                                                    CharTypeFacet::ValidChar,
			                                                    CharTypeFacet::NonDataChar,
																sCDATAEnd,
			                                                    nextChar);

		if(range.getSize())
		{
			bCharDataFound = true;
			reportCharData(range);
		}

		if(nextChar.isEOF() || nextChar == cOpenAngle || nextChar == '&')
		{
			break;
		}
		else if(nextChar == ']')
		{
			const String& errMsg = System::GetSysMessage(sXML, EXML_NOCDSECTEND,
				"element content may not contain the literal ']]>'");

			errorDetected(Fatal, errMsg, EXML_NOCDSECTEND);

			Scanner::SkipNextStringConstant(m_scannerPos, sCDATAEnd);
		}
		else
		{
			String where = QC_T("text content of element '");
			where += m_currentElementName;
			where += sApos;
			unexpectedChar(nextChar, where);
			recoverPosition(0, NULL, 0);
			break;
		}
	}

	return bCharDataFound;
}

//==============================================================================
// ParserImpl::reportCharData
//
// Simple helper function to convert a BufferRange into CharType* strings
// and report them to the ContentEventHandler
//==============================================================================
void ParserImpl::reportCharData(const BufferRange& range) const
{
	if(areCallbacksPermitted() && m_npContentEventHandler)
	{
		for(size_t i=0; i<range.getNumFragments(); i++)
		{
			const BufferRange::Fragment& fragment = range.getFragment(i);
			if(fragment.length)
			{
				m_npContentEventHandler->onCharData(fragment.pData, fragment.length);
			}
		}
	}
}

//=============================================================================
// ParserImpl::parseCDSect
// 
// [18] CDSect ::= CDStart CData CDEnd
// [19] CDStart ::= '<![CDATA['
// [20] CData ::= (Char* - (Char* ']]>' Char*))
// [21] CData ::= ']]>'
//
//=============================================================================
bool ParserImpl::parseCDSect()
{
	bool bCDSectFound = false;

	if(Scanner::SkipNextStringConstant(m_scannerPos, sCDSectStart))
	{
		bCDSectFound = true;

		if(areCallbacksPermitted() && m_npContentEventHandler)
		{
			m_npContentEventHandler->onStartCData();
		}
		
		Character nextChar;
		const BufferRange& range = Scanner::GetNextContigString(m_scannerPos,
		                                                        CharTypeFacet::ValidChar,
		                                                        0,
		                                                        sCDSectEnd,
		                                                        nextChar);

		if(areCallbacksPermitted() && m_npContentEventHandler)
		{
			for(size_t i=0; i<range.getNumFragments(); i++)
			{
				const BufferRange::Fragment& fragment = range.getFragment(i);
				if(fragment.length)
				{
					m_npContentEventHandler->onCharData(fragment.pData, fragment.length);
				}
			}
		}

		//
		// Now, we should be in a position to just skip over the terminator - 
		// but you know how things are... We have to check and skip over the
		// rubbish if we find it.
		//
		if(!Scanner::SkipNextStringConstant(m_scannerPos, sCDSectEnd))
		{
			unexpectedChar(nextChar, QC_T("CDATA section"));
			if(!nextChar.isEOF())
			{
				Scanner::SkipToDelimiter(m_scannerPos, sCDSectEnd, CharTypeFacet::Any);
				Scanner::SkipNextStringConstant(m_scannerPos, sCDSectEnd);
			}
		}

		if(areCallbacksPermitted() && m_npContentEventHandler)
		{
			m_npContentEventHandler->onEndCData();
		}
	}

	return bCDSectFound;
}

//=============================================================================
// ParserImpl::parseAttributeList
// 
// An attribute list is normally imbedded within a start tag or an 
// empty element tag
//
// [et] Attlist ::= (S Attribute)*
// [41] Attribute ::= Name Eq AttValue
// [05] Name ::= (Letter | '_' | ':') (NameChar)*
//
// White space within tags is not pertinent to the document so we are
// free to discard it at will.
//
// This function will be called with the Scanner positioned immediately after
// the tag name, therefore any attributes should be preceded by white space.
//
// Error Handling
// --------------
// To enable error recovery, if we find an illegal
// attribute definition then we always report it.  Following this
// we attempt to continue processing, continuing to scan until we reach either
// the desired delimiter, a '<' (representing the start of another tag) or EOF.
//=============================================================================
bool ParserImpl::parseAttributeList(const ElementType* pElementType,
                                    AttributeSet& retSet, CharType delimiter)
{
	bool bSeenUnexpected=false;
	
	for(int nIndex=0;;nIndex++)
	{
		size_t wsCount = Scanner::SkipWhiteSpace(m_scannerPos);
		Character nextChar = Scanner::PeekNextCharacter(m_scannerPos);

		//
		// This test checks for name chars rather than name-start chars
		// to capture and report user faults more effectively
		//
		if(CharTypeFacet::IsNameChar(nextChar))
		{
			//
			// okay, we know we are within an attribute list because we have a 
			// name start character.
			// Check the doc is well-formed because we should have a white space
			// char before the name
			//
			if(0 == wsCount)
			{
				reportMissingWhiteSpaceBefore(Scanner::PeekNextContigString(m_scannerPos, CharTypeFacet::NameChar, 0));
			}

			QName attrName;
			parseQName(attrName, sAttribute, true, false);
			
			Scanner::SkipWhiteSpace(m_scannerPos);

			AutoPtr<AttributeType> rpAttrType = (pElementType 
			                                 ? (AttributeType*)pElementType->getAttributeType(attrName.getRawName()).get()
			                                 : 0);

			// If the attribute is not defined then assume it is a CDATA type
			String attrType = (rpAttrType) ? rpAttrType->getTypeAsString()
			                               : sCDATA;

			AutoPtr<Attribute> rpAttr = new Attribute(attrName, attrType, nIndex);

			//
			// Check this isn't a duplicate attribute value
			//
			// Well-formedness constraint: Unique Att Spec
			//
			if(!retSet.addAttribute(rpAttr.get()))
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_DUPLATTR,
					"duplicate attribute '{0}'"),
					attrName.getRawName());

				errorDetected(Fatal, errMsg, EXML_DUPLATTR);
			}

			if(Scanner::SkipNextCharConstant(m_scannerPos, cEqual))
			{
				Scanner::SkipWhiteSpace(m_scannerPos);

				bool bCDATA = (attrType == sCDATA);
				bool bNormalized = false;

				//
				// NOTE
				// ----
				// This uses direct access to the m_value member of Attribute
				// and updates that value in place.
				//
				parseAttValue(bCDATA, bNormalized, rpAttr->m_value); // eats quotes

				if(m_features.m_bDoValidityChecks && pElementType)
				{
					if(rpAttrType)
					{
						rpAttrType->validateAttribute(*rpAttr.get(), *this);
					}
					else // attribute not declared
					{
						//
						// Validity constraint: Attribute declared
						//
						// If we are validating and the attribute is not declared then
						// we have an error unless... the attribute prefix is "xmlns"
						// and we have the optional NamespaceDeclarationValidation
						// processing switched off
						//
						bool bNamespaceAttr = (attrName.getPrefix() == sNSPrefix || attrName.getRawName() == sNSPrefix);
						if(m_features.m_bValidateNamespaceDeclarations || !bNamespaceAttr)
						{
							const String& errMsg = MessageFormatter::Format(
								System::GetSysMessage(sXML, EXML_UNDECLATTR,
								"undeclared attribute '{0}' for element '{1}'"),
								attrName.getRawName(),
								pElementType->getName().getRawName());

							errorDetected(Error, errMsg, EXML_UNDECLATTR);
						}
					}
				}

				//
				// Validity constraint: Standalone document
				//
				if(m_features.m_bDoValidityChecks && m_bStandaloneDoc && bNormalized &&
				   !rpAttrType.isNull() && rpAttrType->isExternallyDeclared())
				{
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_SANORMALIZATION,
						"the externally declared attribute '{0}' required normalization which is an error in a standalone document"),
						attrName.getRawName());

					errorDetected(Error, errMsg, EXML_SANORMALIZATION);
				}
			}
			else // not "="
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_ATTRSYNTAX,
					"expected '=' after attribute '{0}'"),
					attrName.getRawName());

				errorDetected(Fatal, errMsg, EXML_ATTRSYNTAX);
			}
		}
		else if(nextChar == delimiter || nextChar == cCloseAngle)
		{
			// this is the normal case. hurrah!
			break;
		}
		else if(nextChar == cOpenAngle)
		{
			// yikes! we seem to have overstepped the mark here, rather than getting a 
			// valid delimiter, we have found a tag start character.
			// We could report the error here, but that is best left to the caller 
			// (a grammar production parsing function) who should find that he isn't
			// correctly terminated - his error message will make more sense than ours.
			//
			//errorDetected(Fatal, QC_T("premature termination of attribute list with tag start character"));
			break;
		}
		else if(nextChar == ',')
		{
			//
			// Report the use of "," as it is a common error
			//
			const String& errMsg = System::GetSysMessage(sXML, EXML_ATTRLISTPUNC,
				"unexpected punctuation within attribute list");

			errorDetected(Fatal, errMsg, EXML_ATTRLISTPUNC);
			Scanner::Skip(m_scannerPos, 1); // skip past the little terror
		}
		else
		{
			if(!bSeenUnexpected)
			{
				bSeenUnexpected = true;
				unexpectedChar(nextChar, QC_T("attribute list"));
			}
			if(nextChar.isEOF())
			{
				break;
			}
			else
			{
				Scanner::Skip(m_scannerPos, nextChar.length()); // skip past the little terror
			}
		}
	}
	
	return (retSet.size() > 0);
}

//=============================================================================
// ParserImpl::parseExternalID
// 
// [75] ExternalID ::= 'SYSTEM' S SystemLiteral | 
//                     'PUBLIC' S PubidLiteral S SystemLiteral
//
// [11] SystemLiteral ::= ('"' [^"]* '"') | ("'" [^']* "'")
//
// [12] PubidLiteral ::= '"' PubidChar* '"' | "'" (PubidChar - "'")* "'"
//
// [13] PubidChar ::= white-space, alphanumeric and special symbols
//
// Note: PubidChar includes "%", so it goes without saying that we cannot expand
//       PE references when dealing with PubidLiterals!  This is confirmed
//       in the XML 1.0 rec, sectio n4.4 "Reference in DTD"
//
// For convenience we also deal with the strange NotationDecl problem of
// having an optional SystemLiteral following PUBLIC
//=============================================================================
bool ParserImpl::parseExternalID(String& systemLiteral, String& pubidLiteral,
                                 bool bReqd, bool bOptionalSystemLiteral,
                                 const String& where)
{
	bool bExternalID = false;

	if(skipNextStringConstantEx(sSystem))
	{
		bExternalID = true;
		skipRequiredWhitespaceAfter(sSystem, where);
		systemLiteral = parseSystemLiteral();
	}
	else if(skipNextStringConstantEx(sPublic))
	{
		bExternalID = true;
		skipRequiredWhitespaceAfter(sPublic, where);
		pubidLiteral = parsePubidLiteral();
		size_t wsCount = skipWhiteSpaceEx();
		Character nextChar = peekNextCharacterEx();
		if(nextChar == '\'' || nextChar == '"')
		{
			if(0 == wsCount)
			{
				reportMissingWhiteSpaceBefore(QC_T("system literal"));
			}
			systemLiteral = parseSystemLiteral();
		}

		if(systemLiteral.empty() && !bOptionalSystemLiteral)
		{
			const String& errMsg = System::GetSysMessage(sXML, EXML_MISSINGSYSLITERAL,
				"missing system literal");

			errorDetected(Fatal, errMsg, EXML_MISSINGSYSLITERAL);
		}
	}
	else if(bReqd)
	{
		const String& errMsg = System::GetSysMessage(sXML, EXML_INVALIDEXTID,
			"external identifier must start with 'PUBLIC' or 'SYSTEM'");

		errorDetected(Fatal, errMsg, EXML_INVALIDEXTID);
	}

	return bExternalID;
}

//=============================================================================
// ParserImpl::parseSystemLiteral
// 
// [11] SystemLiteral ::= ('"' [^"]* '"') | ("'" [^']* "'")
//
// Basically, anything goes in this quoted string.  Note that
// parameter entities cannot not be expanded during this process because
// the "%" percent sign is a valid character.
//
// Note also that XML 1.0 section 4.2.2 states that system literals are in fact
// URIs and they must not contain fragment identifiers.  This means that they
// may not contain the '#' character.  This also means that they need to be
// converted to URIs by converting and escaping certain characters, but that
// conversion process occurs only when the URI is dereferenced.
//
// There is an open question whether local file URLs should be escaped.
//=============================================================================
String ParserImpl::parseSystemLiteral()
{
	String strRet;
	const Character startQuote = Scanner::PeekNextCharacter(m_scannerPos);

	if(startQuote == '\'' || startQuote == '"')
	{
		Scanner::GetNextCharacter(m_scannerPos); // skip past the quote
		
		while(true)
		{
			Character nextChar = Scanner::GetNextCharacter(m_scannerPos);

			if(nextChar == startQuote)
			{
				break;
			}
			else if(CharTypeFacet::IsValidChar(nextChar) && nextChar != '#')
			{
				nextChar.appendToString(strRet);
			}
			else
			{
				unexpectedChar(nextChar, QC_T("system identifier"));
				if(nextChar.isEOF())
				{
					break;
				}
				else
				{
					Scanner::SkipToDelimiter(m_scannerPos, startQuote.first(), CharTypeFacet::ValidChar);
				}
			}
		}
	}
	else
	{
		const String& errMsg = System::GetSysMessage(sXML, EXML_SYSTEMIDQUOTES,
			"system identifier must be enclosed by quotes");

		errorDetected(Fatal, errMsg, EXML_SYSTEMIDQUOTES);
	}
	
	return strRet;
}

//=============================================================================
// ParserImpl::parsePubidLiteral
// 
// [12] PubidLiteral ::= ('"' PubidChar* '"') | ("'" PubidChar* "'")
//
// Note: interestingly, the quote (") character is not a PubidChar.
// Note: perhaps morte interestingly, the percent character is a PubidChar
//       so we must not attempt to expand PE references when dealing with this.
//       This is confirmed in XML 1.0, 4.4 "Reference in DTD".
// Note: XML 1.0 4.2.2 External Entities describes how public identifiers
//       should be normalized before they can be used to lookup catalog entries.
//       It makes sense to perform this normalisation here and now.  This
//       involves stripping leading and trailing white-space, and converting
//       white-space sequences to a single 0x20.
//=============================================================================
String ParserImpl::parsePubidLiteral()
{
	String strRet;
	const Character startQuote = Scanner::PeekNextCharacter(m_scannerPos);

	if(startQuote == '\'' || startQuote == '"')
	{
		getNextCharacterEx(); // skip past the quote
		bool bWhitespacePending = false;
		while(true)
		{
			Character nextChar = Scanner::GetNextCharacter(m_scannerPos);

			if(nextChar == startQuote)
			{
				break;
			}
			else if(CharTypeFacet::IsCharType(nextChar, CharTypeFacet::PubidChar))
			{
				if(CharTypeFacet::IsWhiteSpace(nextChar))
				{
					bWhitespacePending = true;
				}
				else
				{
					if(bWhitespacePending && strRet.size())
					{
						strRet += ' ';
					}
					bWhitespacePending = false;
					nextChar.appendToString(strRet);
				}
			}
			else
			{
				unexpectedChar(nextChar, QC_T("public identifier"));
				const char sQuote[2] = {char(startQuote.first()), 0};
				const char* delims[] = {sQuote};
				recoverPosition(1, delims, 0);
				break;
			}
		}
	}
	else
	{
		const String& errMsg = System::GetSysMessage(sXML, EXML_PUBLICIDQUOTES,
			"public identifier must be enclosed by quotes");

		errorDetected(Fatal, errMsg, EXML_PUBLICIDQUOTES);
	}
	
	return strRet;
}

//=============================================================================
// ParserImpl::parseAttValue
// 
// Called during attribute list processing.  On entry to this function
// the scanner position will have been incremented past any leading white space
// so we expect to see a quoted string.  We eat the quotes on both sides
// of the string.
//
// The processing is complicated by the fact that attribute values can
// contain references, which must be expanded.
//
// [10] AttValue ::= '"' ([^<&"] | Reference)* '"' |
//                   "'" ([^<&'] | Reference)* "'" 
//
// Default attribute values cannot contain PE Refeences. The % is not
// recognized as a PEReference start - even for default attribute
// values declared in a DTD.
//
// It is further complicated by the fact that attribute values require
// normalisation (see XML 1.0, 3.3.3).  The degree of normalisation is
// different depending on if this is a CDATA (string) attribute or not.
//
// To handle both of these situations, this function delegates the work
// to parseAndNormalizeAttValue(), which can actually be called
// recursively if the attribute value contains references to entities.
//
// Note: we haven't declared a String to return because we wish to make
// use of the anonymous string returned from parseAndNormalizeAttValue()
//
// Note: The return parameter bNormalized is used to inform the caller if
//       any further normalisation occurred to non-CDATA attributes.
//=============================================================================
bool ParserImpl::parseAttValue(bool bCData, bool& bNormalized, String& strRet)
{
	//
	// We must use peekNextCharacterEx() because an attribute value may be entirely
	// contained with a PE.  PEs are not recognized within an attribute
	// value, but the whole attribute may be contained in one
	//
	const Character startQuote = peekNextCharacterEx();

	if(startQuote == '\'' || startQuote == '"')
	{
		getNextCharacterEx(); // skip past the quote

		parseAndRecurseAttributeValue(startQuote.first(), strRet);
		if(!bCData)
		{
			normalizeAttributeValue(strRet, bNormalized);
		}
		else
		{
			bNormalized = false;
		}
	}
	else
	{
		// yikes! An attribute value didn't begin with quotes.
		// This is an error obviously, but what should the recovery be?
		// It seems that the best choice is scan name characters
		// (this will allow the parser to parse some HTML that would otherwise
		// be impossible)
		const String& errMsg = System::GetSysMessage(sXML, EXML_ATTRQUOTES,
			"attribute value must be enclosed by quotes");

		errorDetected(Fatal, errMsg, EXML_ATTRQUOTES);

		Character nextChar;
		Scanner::GetNextContigString(m_scannerPos, CharTypeFacet::NameChar, 0, sNull, nextChar).appendToString(strRet);
	}

	return true; // what else can we say?
}

//==============================================================================
// ParserImpl::normalizeAttributeValue
//
// Perform the final part of the attribute value normalization algorithm
// as described in XML 1.0 section 3.3.3.
//==============================================================================
void ParserImpl::normalizeAttributeValue(String& strRet, bool& bNormalized) const
{
	String strWork;
	const size_t bufferSize = 256;
	CharType buffer[bufferSize];

	// first remove trailing spaces by locating the last non space
	size_t lastws = strRet.find_last_not_of(cSpace);
	size_t strSize = (lastws == String::npos) ? strRet.length() : lastws+1;

	// to aid performance, reserve the maximum required length for the string
	strWork.reserve(strSize);

	bool bSpace = true; // has the effect of stripping leading space
	size_t buffIx=0;
	
	for(size_t i=0; i<strSize; ++i)
	{
		const CharType nextChar = strRet[i];
		if(nextChar == cSpace)
		{
			if(!bSpace)
			{
				bSpace = true;
				buffer[buffIx++] = nextChar;
			}
		}
		else
		{
			bSpace = false;
			buffer[buffIx++] = nextChar;
		}
		if(buffIx == bufferSize)
		{
			strWork.append(buffer, buffIx);
			buffIx = 0;
		}
	}

	if(buffIx)
	{
		strWork.append(buffer, buffIx);
	}

	bNormalized = (strWork.length() != strRet.length());

	strRet.swap(strWork);
}

//=============================================================================
// ParserImpl::parseAndRecurseAttributeValue
// 
// [10] AttValue ::= '"' ([^<&"] | Reference)* '"' |
//                   "'" ([^<&'] | Reference)* "'" 
//
// Called to process the data within an attribute value.  This may be character
// data or (expanded) entity references - but will not need to expand
// parameter entities even when parsing a default attribute value.
//
// This is open to some interpretation, but see xml 1.0 4.4 for some
// clarification.  This is backed up by the Oasis test suite and James Clarks
// test valid/sa/094.xml which tests for this case.
//
// Default attribute values containing General Entity references are a sore
// point - we require them to be declared before they are used - which isn't
// usually required is it?
//
// Attribute values require normalisation (see XML 1.0, 3.3.3).
// In this function we perform just apply step 3 of the normalization 
// algorithm.  The final stage is performed by the calling function.
//
// Note: as its name suggests, this function may be called recursively.  This
// will occur whenever a general entity reference is contained within an
// attribute value.  If the entity replacement text also contains an entity
// reference the recursion gets deeper.
//=============================================================================
void ParserImpl::parseAndRecurseAttributeValue(CharType delimiter, String& strRet)
{
	const String& sDelim = delimiter
	                     ? ((delimiter == '\'') ? strApos : strQuote)
	                     : sNull;

	while(true)
	{
		Character nextChar;

		const BufferRange& range = Scanner::GetNextContigString(m_scannerPos,
			                                                    CharTypeFacet::ValidChar,
			                                                    CharTypeFacet::NonDataChar | CharTypeFacet::Whitespace,
																sDelim,
			                                                    nextChar);

		if(range.getSize())
		{
			range.appendToString(strRet);

			// If the next char is EOF this may be due to the end of
			// an entity in which case we want to keep on trucking
			if(nextChar.isEOF() && !delimiter)
			{
				continue;
			}
		}
		
		if(delimiter && nextChar == delimiter)
		{
			//getNextCharEx(); // eat it
			Scanner::GetNextCharacter(m_scannerPos);
			// we have eaten the delimiter, so now lets push off
			break;
		}
		else if(nextChar == '&')
		{
			//
			// Note: by specifying bAttribute=true, we get all entities
			// back in the returned string, even internal ones.
			//
			const String& entityRef = parseReference(true /*resolve entities*/,
					                                 true /*attribute*/);
			strRet.append(entityRef);
		}
		else if(nextChar == cOpenAngle)
		{
			//
			// If we come across a "<" it could mean that the xml author has
			// a) Forgotten to terminate the string.  In this case recovery
			//    is automatic because we just don't eat the character, and the
			//    next production will start with it
			// b) The replacement text of an entity contains "<".  This has
			//    been deemed illegal by the W3C, so no worries there.
			// c) Simply tried to code an < in the attribute!
			//
			if(m_scannerPos.getEntity().getType() == EntityType::Document)
			{
				const String& errMsg = System::GetSysMessage(sXML, EXML_ATTRLT,
					"attribute value may not contain '<'");

				errorDetected(Fatal, errMsg, EXML_ATTRLT);
			}
			else
			{
				//
				// well-formedness constraint: No < in Attribute Values
				//
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_ATTRENTITYLT,
					"the replacement text of entity '&{0};' contains '<' which is illegal when used within an attribute value"),
					m_scannerPos.getEntity().getName());

				errorDetected(Fatal, errMsg, EXML_ATTRENTITYLT);
			}
			// break; We don't break, because there is nothing stopping us from
			//        processing this as a valid attribute string 
			//        We assume the author didn't intend this to be a control char
			Scanner::GetNextCharacter(m_scannerPos); // skip past the "<"
		}
		else if(CharTypeFacet::IsWhiteSpace(nextChar))
		{
			//
			// This normalizes white-space (#x20, #xD, #xA. #x9) into #x20
			// 
			strRet += cSpace;
			Scanner::GetNextCharacter(m_scannerPos); // skip past the w/s
		}
		else if(nextChar.isEOF() && !delimiter)
		{
			//
			// If we have been presented with an EOF this either means
			// the end of an entity reference (which is okay) or the
			// end of the document (which isn't okay)
			//
			break;
		}
		else if(!CharTypeFacet::IsValidChar(nextChar))
		{
			unexpectedChar(nextChar, QC_T("attribute value"));
			if(delimiter)
			{
				char szDelim[2] = {char(delimiter), 0};
				const char* endTokens[] = {szCloseAngle, szDelim};
				recoverPosition(2, endTokens, 1); //eat delims, leave closeangle
				break;
			}
			else // !delimiter
			{
				// if we are not delimited it means that we are processing
				// entity replacement text.  How do we recover?
				// we don't.  Just breaking out is enough
				break;
			}
		}
		else
		{
			// this should be impossible!
			QC_DBG_ASSERT(false);
			Scanner::GetNextCharacter(m_scannerPos); // skip past the impossible
		}
	}
}

//=============================================================================
// ParserImpl::parseAttlistDecl
// 
// Part of DTD processing, this function parses the AttlistDecl and adds
// the declaration into the 
// 
// [52] AttlistDecl ::= '<!ATTLIST' S Name AttDef* S? '>'
//
//=============================================================================
bool ParserImpl::parseAttlistDecl()
{
	bool bAttlistDeclFound = false;
	
	if(Scanner::SkipNextStringConstant(m_scannerPos, sAttlistDecl))
	{
		bAttlistDeclFound = true;

		const Entity& startEntity = m_scannerPos.getEntity();

		QName elementName;
		/*bool bElementNameFound = */ parseQName(elementName, sElementType, true, true);

		AutoPtr<ElementType> rpElementType = getElement(elementName);

		// If the element has not yet been declared then that's just fine.
		// There is no law about the order of declaration within a DTD, and it is
		// possible that the attrib is being declared in the internal subset whereas
		// the element is declared in the external subset.
		// The XML 1.0 recommendation does allow us to issue a warning if the
		// element is not declared at all - but we will not be in a possition to
		// do that until the entire DTD has been processed
		if(rpElementType.isNull())
		{
			rpElementType = addElement(elementName);
		}

		if(m_npDTDEventHandler)
		{
			m_npDTDEventHandler->onStartAttlist(elementName.getRawName());
		}

		parseAttDefs(*rpElementType.get());

		if(!skipNextCharConstantEx(cCloseAngle))
		{
			// This error may never get reported because to get here without ">"
			// and earlier error must have occurred
			reportDeclTermError(QC_T("attribute list"), sCloseAngle);

			recoverPosition(1, &szCloseAngle, 0);
		}

		if(m_npDTDEventHandler)
		{
			m_npDTDEventHandler->onEndAttlist();
		}

		if(m_features.m_bDoValidityChecks)
		{
			validatePENesting(startEntity, QC_T("attribute list declaration"));
		}
	}

	return bAttlistDeclFound;
}

//=============================================================================
// ParserImpl::parseAttDefs
// 
// Called when parsing an AttributeList, this function
// parses all the individual attribute definitions.
// 
//=============================================================================
bool ParserImpl::parseAttDefs(ElementType& element)
{
	bool bContinue = true;

	while(bContinue)
	{
		QName attName;
		bool bAttNameFound = parseQName(attName, sAttributeName, false, true);

		if(!bAttNameFound && peekNextCharacterEx() == cCloseAngle)
		{
			// normal end of the list...
			bContinue = false;
		}
		else
		{
			bContinue = parseAttDef(attName, element);
		}
	}
	return true;
}


//=============================================================================
// ParserImpl::parseAttDef
// 
// Called when parsing an AttributeList, this function
// parses all the individual attribute definitions.
// 
// [53] AttDef      ::= S Name S AttType S DefaultDecl
// [54] AttType     ::= StringType | TokenizedType | EnumeratedType
// [55] StringType  ::= 'CDATA'
// [56] TokenizedType ::= 'ID' | 'IDREF' | 'IDREFS' | 'ENTITY' | 'ENTITIES'
//                       | 'NMTOKEN' | 'NMTOKENS' 
// [57] EnumeratedType ::= NotationType | Enumeration
// [58] NotationType ::= 'NOTATION' S '(' S? Name (S? '|' S? Name)* S? ')'
// [59] Enumeration  ::= '(' S? NmToken (S? '|' S? NmToken)* S? ')'
// [60] DefaultDecl ::= '#REQUIRED' | '#IMPLIED' 
//                       | (('#FIXED' S)? AttValue)
//
//=============================================================================
bool ParserImpl::parseAttDef(const QName& attName, ElementType& element)
{
	bool bRet = true;
	bool bNewAttribute = true;
	
	AutoPtr<AttributeType> rpAttr = element.getAttributeType(attName);
	
	// xml 1.0, section 2.9 states that a decl is externally declared
	// if it is not internal subset or PE.  This means in effect that
	// the entity must be the document entity for it to be internal for the
	// standalone rules.
	bool bExternalDeclaration = (m_scannerPos.getEntity().getType() != EntityType::Document);

	if(rpAttr)
	{
		// point at a new AttributeType that will simply be deleted
		// when it goes out of scope.
		rpAttr = new AttributeType(element, bExternalDeclaration, attName);

		// as this is not the first definition of this attribute
		// there is little point in doing any validation of it
		bNewAttribute = false;

		// Duplicate attribute declarations are permitted, and we
		// don't have a useful way of distinguishing between multiple
		// occurences within the same entity (to do so would require that we
		// store the declaring entity - which we don't at present)
		//
		// Versions up to 0.14.1 used to test for duplicates in the same DTD
		// subset - but the definition of DTD subset was dubious and therefore
		// the test was worthless
		//
		// We now just issue a warning if we find a duplicate when processing the
		// internal DTD subset (ie the declarations contained within the document
		// entity), as the author would generally expect these declarations to be
		// read first.
		if(m_features.m_bDoValidityChecks && m_features.m_bDoWarningChecks && 
		   parsingInternalDTDSubset())
		{
			const String& errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_ATTRIBUTEDECLARED,
				"attribute '{0}' has already been declared for element '{1}'"),
				attName.getRawName(),
				element.getName().getRawName());

			errorDetected(Warning, errMsg, EXML_ATTRIBUTEDECLARED);
		}
	}
	else
	{
		rpAttr = element.addAttributeType(attName, bExternalDeclaration);
	}

	//
	// White-space is required - and there is a test condition to check for it!
	//
	if(!skipWhiteSpaceEx())
	{
		const String& errMsg = MessageFormatter::Format(
			System::GetSysMessage(sXML, EXML_SYNTAXATTLIST,
			"white space expected after '{0}' in attribute list declaration for element '{1}'"),
			attName.getRawName(),
			element.getName().getRawName());

		errorDetected(Fatal, errMsg, EXML_SYNTAXATTLIST);
	}

	bool bExpandedPE = true;
	while(bExpandedPE)
	{
		bExpandedPE = false;
		//
		// Find out which AttributeType is being defined using the
		// multi-purpose testNextTokenType() function.
		// 
		String strType;
		bool bErrorReported(false);
		const int attType = testNextTokenType(AttributeTable, strType, bErrorReported);

		switch(attType)
		{
		case AttributeType::ID:
			if(m_features.m_bDoValidityChecks && bNewAttribute && element.hasIdAttribute())
			{
				//
				// Validity Constraint: One ID per Element Type
				//
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_MULTIPLEATTRTYPE,
					szMultiplAttrTypeErr),
					element.getName().getRawName(),
					QC_T("ID"),
					element.getIdAttributeName(),
					attName.getRawName());

				errorDetected(Error, errMsg, EXML_MULTIPLEATTRTYPE);
			}
			else
			{
				element.setIdAttributeName(attName.getRawName());
			}
		// and drop through...
		case AttributeType::CDATA:
		case AttributeType::IDREF:
		case AttributeType::IDREFS:
		case AttributeType::ENTITY:
		case AttributeType::ENTITIES:
		case AttributeType::NMTOKEN:
		case AttributeType::NMTOKENS:
			rpAttr->setType((AttributeType::Type)attType);
			Scanner::SkipNextStringConstant(m_scannerPos, strType);
			parseAttDefaultDecl(true, *rpAttr.get()); 
			break;

		case AttributeType::NOTATION:
			if(m_features.m_bDoValidityChecks && bNewAttribute && 
			   element.hasNotationAttribute())
			{
				//
				// Validity Constraint: One NOTATION per Element Type
				//
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_MULTIPLEATTRTYPE,
					szMultiplAttrTypeErr),
					element.getName().getRawName(),
					QC_T("NOTATION"),
					element.getNotationAttributeName(),
					attName.getRawName());

				errorDetected(Error, errMsg, EXML_MULTIPLEATTRTYPE);
			}
			else
			{
				element.setNotationAttributeName(attName.getRawName());
			}
			rpAttr->setType(AttributeType::NOTATION);
			parseAttNotation(*rpAttr.get()); // skips "NOTATION" string and parses (..)
			parseAttDefaultDecl(true, *rpAttr.get()); // handles leading w/s
			break;

		case AttributeType::ENUMERATION:
			rpAttr->setType((AttributeType::Type)attType);
			parseAttEnumeration(*rpAttr.get());
			parseAttDefaultDecl(true, *rpAttr.get()); // handles leading w/s
			break;

		case EndOfEntity:
			bRet = false;
			break;

		case PEReference:
			// If we detect a PE reference at this point then it will be
			// processed and the ScannerPosition bumped up to include it
			bExpandedPE = parsePEReference(false /*markup decl*/,
			                               false /* hard eof */,
										   true  /* name is reqd following % */);
			break;

		case AttributeType::UNKNOWN:
			if(!bErrorReported)
				unexpectedToken(0, strType, QC_T("attribute type"));
			skipNextStringConstantEx(strType);
			recoverPosition(1, &szCloseAngle, 1); //dont eat anything
			bRet = false;
			break;
		default:
			QC_DBG_ASSERT(false); // should not happen
			bRet = false;
		}
	}

	if(bRet && bNewAttribute && m_npDTDEventHandler)
	{
		String sDefaultType;
		
		switch(rpAttr->getDefaultType())
		{
		case AttributeType::REQUIRED:
			sDefaultType = QC_T("#REQUIRED");
			break;
		case AttributeType::IMPLIED:
			sDefaultType = QC_T("#IMPLIED");
			break;
		case AttributeType::FIXED:
			sDefaultType = QC_T("#FIXED");
			break;
		default:
			break;
		}

		m_npDTDEventHandler->onAttributeDecl(
			element.getName().getRawName(),
			attName.getRawName(),
			rpAttr->getTypeAsString(),
			sDefaultType,
			rpAttr->getEnumListAsString(),
			rpAttr->getDefaultValue());
	}
	return bRet;
}

//=============================================================================
// ParserImpl::parseAttNotation
// 
// [58] NotationType ::= 'NOTATION' S '(' S? Name (S? '|' S? Name)* S? ')'
//
//=============================================================================
bool ParserImpl::parseAttNotation(AttributeType& attr)
{
	if(skipNextStringConstantEx(sNotation))
	{
		skipRequiredWhitespaceAfter(sNotation, sAttributeDeclaration);
		return parseAttEnumList(false, QC_T("notation"), attr);
	}
	else
		return false;
}

//=============================================================================
// ParserImpl::parseAttEnumeration
// 
// [59] Enumeration  ::= '(' S? NmToken (S? '|' S? NmToken)* S? ')'
//
//=============================================================================
bool ParserImpl::parseAttEnumeration(AttributeType& attr)
{
	return parseAttEnumList(true, QC_T("enumeration"), attr);
}

//=============================================================================
// ParserImpl::parseAttEnumList
// 
// Helper function to parse a list of Names or NmTokens separated by "|"
// characters.
// [et] enum-list ::= '(' S? NmToken (S? '|' S? NmToken)* S? ')'
//
//=============================================================================
bool ParserImpl::parseAttEnumList(bool bNmToken,
                                  const String& what, AttributeType& attribute)
{
	bool bListFound = false;
	bool bValueFound = false;

	if(skipNextCharConstantEx('('))
	{
		bListFound = true;
		bool bFirstTime = true;

		while(true)
		{
			skipWhiteSpaceEx();

			if (skipNextCharConstantEx(')'))
			{
				// normal end of list
				// check that at least one value has been provided
				if(!bValueFound)
				{
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_ATTRENUMEMPTY,
						"at least one value must be provided in attribute {0} list"),
						what);

					errorDetected(Fatal, errMsg, EXML_ATTRENUMEMPTY);
				}
				break;
			}
			else
			{
				if(bFirstTime)
				{
					bFirstTime = false;
				}
				else
				{
					if(!skipNextCharConstantEx('|'))
					{
						const String& errMsg = MessageFormatter::Format(
							System::GetSysMessage(sXML, EXML_ATTRENUMSEPARATOR,
							"expected '|' separator in attribute {0} list"),
							what);

						errorDetected(Fatal, errMsg, EXML_ATTRENUMSEPARATOR);
					}

					skipWhiteSpaceEx();
				}
				const String& name = parseName(what, true, false, bNmToken);
				if(name.empty())
				{
					// need to recover somehow
					const char* endTokens[] = {szCloseAngle, szCloseBracket};
					recoverPosition(2, endTokens, 1); // dont eat the cCloseAngle
					break;
				}
				else
				{
					// Note: Validity and interoperability tests are performed
					// by the AttributeType, so we don't need to.
					attribute.addEnum(name, *this);
					bValueFound = true;
				}
			}
		}
	}

	return bListFound;
}

//=============================================================================
// ParserImpl::parseAttDefaultDecl
// 
// [60] DefaultDecl ::= '#REQUIRED' | '#IMPLIED' 
//                       | (('#FIXED' S)? AttValue)
//
// Note: For convenience we require leading white-space (this prevents the 
//       calling functions from having this code in several places
//
//=============================================================================
bool ParserImpl::parseAttDefaultDecl(bool bRequireWhiteSpace, AttributeType& attr)
{
	size_t wsCount = skipWhiteSpaceEx();

	if(bRequireWhiteSpace && wsCount == 0)
	{
		const String& errMsg = MessageFormatter::Format(
			System::GetSysMessage(sXML, EXML_MISSINGWHITESPACE3,
			"white-space expected before default declaration for attribute '{0}'"),
			attr.getName().getRawName());

		errorDetected(Fatal, errMsg, EXML_MISSINGWHITESPACE3);
		// no recovery required
	}

	if(skipNextStringConstantEx(sRequired))
	{
		attr.setDefaultType(AttributeType::REQUIRED);
	}
	else if(skipNextStringConstantEx(sImplied))
	{
		attr.setDefaultType(AttributeType::IMPLIED);
	}
	else
	{
		// Validity Constraint: ID Attribute Default
		if(m_features.m_bDoValidityChecks && attr.getType() == AttributeType::ID)
		{
			const String& errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_IDATTRDEFAULT,
				"ID attribute '{0}' must have a declared default of #IMPLIED or #REQUIRED"),
				attr.getName().getRawName());

			errorDetected(Error, errMsg, EXML_IDATTRDEFAULT);

			// no recovery required
			attr.setDefaultType(AttributeType::IMPLIED);
		}

		if(skipNextStringConstantEx(sFixed))
		{
			attr.setDefaultType(AttributeType::FIXED);
			if(!skipWhiteSpaceEx())
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_MISSINGWHITESPACE4,
					"white-space expected before fixed value for attribute '{0}'"),
					attr.getName().getRawName());

				errorDetected(Fatal, errMsg, EXML_MISSINGWHITESPACE4);
			}
		}
		else
		{
			attr.setDefaultType(AttributeType::DEFAULT);
		}

		bool bCDATA = (attr.getType() == AttributeType::CDATA);
		bool bNormalized = false;

		String attValue;
		parseAttValue(bCDATA, bNormalized, attValue);

		if(!attValue.empty())
		{
			attr.setDefaultValue(attValue, *this);
		}
		else if(m_features.m_bDoValidityChecks && !bCDATA)
		{
			//
			// Only CDATA attributes are permitted to have an empty default value
			//
			const String& errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_ATTRDEFAULTEMPTY,
				"{0} attribute '{1}' may not have an empty default value"),
				attr.getTypeAsString(),
				attr.getName().getRawName());

			errorDetected(Error, errMsg, EXML_IDATTRDEFAULT);
		}
	}

	return true;
}

//=============================================================================
// ParserImpl::parseElementDecl
// 
// Part of DTD processing, this function parses the ElementDecl and adds
// the declaration into the DTD model
// 
// [45] elementdecl ::= '<!ELEMENT' S Name S contentspec S? '>'
//
//=============================================================================
bool ParserImpl::parseElementDecl()
{
	bool bElementDeclFound = false;

	if(skipNextStringConstantEx(sElementDecl))
	{
		bElementDeclFound = true;
		const Entity& startEntity = m_scannerPos.getEntity();

		QName name;
		bool bNameFound = parseQName(name, sElementName, true, true);

		if(bNameFound)
		{
			AutoPtr<ElementType> rpElement = getElement(name);
			if(rpElement.isNull())
			{
				rpElement = addElement(name);
			}
			
			bool bNewElement = !(rpElement->isDefined());
			
			if(!bNewElement)
			{
				// duplicate element!
				if(m_features.m_bDoValidityChecks)
				{
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_DUPLELEMENTDECL,
						"duplicate declaration for element '{0}'"),
						name.getRawName());

					errorDetected(Error, errMsg, EXML_DUPLELEMENTDECL);
				}
				// The first definition of an element is king.
				// To avoid any funny business we will have to
				// substitute the element for a new one which will
				// simply be deleted when it goes out of scope
				rpElement = new ElementType(name);
			}
			
			// xml 1.0, section 2.9 states that a decl is externally declared
			// if it is not internal subset or PE.  This means in effect that
			// the entity must be the document entity for it to be internal for the
			// standalone rules.
			bool bExternalDeclaration = (m_scannerPos.getEntity().getType() != EntityType::Document);

			rpElement->setDefined(true, bExternalDeclaration);

			skipRequiredWhitespaceAfter(sElementName, sElementDeclaration);

			parseContentSpec(*rpElement.get());
			
			skipWhiteSpaceEx();

			if(!skipNextCharConstantEx(cCloseAngle))
			{
				// Yikes! here we go again. 
				// report the error
				reportDeclTermError(QC_T("element"), sCloseAngle);
		
				//
				// Luckily this is standard stuff and there is a function we can call to
				// do the dirty work!
				recoverPosition(1, &szCloseAngle, 0);
			}
			
			if(bNewElement && m_npDTDEventHandler)
			{
				m_npDTDEventHandler->onElementDecl(name.getRawName(), rpElement->getContentTypeAsString());
			}

		}
		else
		{
			// the error will already have been reported, so just recover
			recoverPosition(1, &szCloseAngle, 0);
		}

		if(m_features.m_bDoValidityChecks)
		{
			validatePENesting(startEntity, sElementDeclaration);
		}
	}

	return bElementDeclFound;
}

//=============================================================================
// ParserImpl::parseContentSpec
// 
// Part of DTD processing, this function parses the interesting part
// of the ElementDecl.  contentSpec is required by the grammar so there
// is no need to backtrack if we don't find one.
// 
// [46] contentspec ::= 'EMPTY' | 'ANY' | Mixed | children
// [47] children ::= (choice | seq) ('?' | '*' | '+')?
// [48] cp     ::= (Name | choice | seq) ('?' | '*' | '+')?
// [49] choice ::= '(' S? cp ( S? '|' S? cp )* S? ')'
// [50] seq    ::= '(' S? cp ( S? ',' S? cp )* S? ')'
// [51] Mixed ::= '(' S? '#PCDATA' ( S? '|' S? Name)* S? ')*'
//             |  '(' S? '#PCDATA' S? ')'
//
// Let's cut through the confusion a little.  Here are a few valid examples:
// <!ELEMENT Rob EMPTY>
// <!ELEMENT Rob ANY>
// <!ELEMENT Rob (#PCDATA | Ellie | Celia)*>
// <!ELEMENT Rob (#PCDATA)>
// <!ELEMENT Rob (((((Ellie)))))>
// <!ELEMENT Rob ((Ellie, Celia) | (Ellie* | Celia))>    [ambiguous]
// <!ELEMENT Rob ((Ellie, Celia), (Celia, Ellie))
// <!ELEMENT rob (el3+, (el2 | el3)*)>                   [ambiguous]
// 
//=============================================================================
bool ParserImpl::parseContentSpec(ElementType& element)
{
	if(skipNextStringConstantEx(sEmpty))
	{
		element.setContentType(ElementType::EMPTY);
	}
	else if(skipNextStringConstantEx(sAny)) 
	{
		element.setContentType(ElementType::ANY);
	}
	else if(parseElementContentSpecList(element, NULL))
	{
		ElementContentSpec* pContentSpec = element.getContentSpec();
		QC_DBG_ASSERT(pContentSpec!=0);
		bool bMultiplicityFound = parseContentMultiplicity(element, pContentSpec);
		bool bMixedSpec = (element.getContentType() == ElementType::MIXED);
		if(bMixedSpec && pContentSpec->getMultiplicity() != ElementContentSpec::ZeroOrMore)
		{
			if(!pContentSpec->isEmpty())
			{
				const String& errMsg = System::GetSysMessage(sXML, EXML_MIXEDSPECEND,
					"mixed content specification with child elements must end with ')*'");

				errorDetected(Fatal, errMsg, EXML_MIXEDSPECEND);
			}
			else if(bMultiplicityFound)
			{
				const String& errMsg = System::GetSysMessage(sXML, EXML_CHARSPECMULTIPLICITY,
					"invalid multiplicity for character content specification");

				errorDetected(Fatal, errMsg, EXML_CHARSPECMULTIPLICITY);
			}
		}
	}
	else
	{
		// report the error
		const String& errMsg = System::GetSysMessage(sXML, EXML_ELEMENTCONTENTSTART,
			"element content specification must start with '('");

		errorDetected(Fatal, errMsg, EXML_ELEMENTCONTENTSTART);

		// and recover so that the caller thinks everything is hunky dorey
		recoverPosition(1, &szCloseAngle, 1); // dont eat the delimiter
		element.setContentType(ElementType::ANY);
		return false;
	}

	//
	// Validate the content specification
	// Note: this only needs to be done if the element is used in an 
	// instance document.  Some DTDs are very large (eg DocBook) and it is
	// a real waste of cycles to validate unused elements.
	//
	/*
	if(m_features.m_bDoValidityChecks && element.getContentSpec())
	{
		element.getContentSpec()->validate(*this);
	}
	*/

	return true; // grammar demands it is present!
}

//=============================================================================
// ParserImpl::parseElementContentSpecList
// 
// Part of DTD processing, this function parses the list of elements
// that make up the children of an element.
//
// If bMixedSpec = true then the list must adhere to the list following
// '#PCDATA' which limits the list to a set of choices without further child
// elements.
//
// [48] cp     ::= (Name | choice | seq) ('?' | '*' | '+')?
// [49] choice ::= '(' S? cp ( S? '|' S? cp )* S? ')'
// [50] seq    ::= '(' S? cp ( S? ',' S? cp )* S? ')'
//
// These grammar productions do not adequately explain what this function
// does.  It is a recursive function that splits the list into its constituent
// parts.  If an opening bracket is found then a sub-list is found and this
// function is called to process it.
//
//=============================================================================
bool ParserImpl::parseElementContentSpecList(ElementType& element,
                                             ElementContentSpec* pContentSpec)
{
	bool bContentSpecFound = false;

	if(skipNextCharConstantEx(cOpenBracket))
	{
		bContentSpecFound = true;

		const Entity& startEntity = m_scannerPos.getEntity();

		enum ListType {choice, seq, any};
		enum FSM {token, separator};
		FSM state = separator;

		skipWhiteSpaceEx();

		if(pContentSpec == 0)
		{
			if(skipNextStringConstantEx(sPCData))
			{
				element.setContentType(ElementType::MIXED);
				pContentSpec = element.getContentSpec();
				QC_DBG_ASSERT(pContentSpec!=0);
				pContentSpec->setListType(ElementContentSpec::Choice);
				state = token;
			}
			else
			{
				element.setContentType(ElementType::SPECIFIED);
				pContentSpec = element.getContentSpec();
				QC_DBG_ASSERT(pContentSpec!=0);
			}
		}

		bool bMixedSpec = (element.getContentType() == ElementType::MIXED);
		
		ListType theListType = bMixedSpec ? choice : any;

		while(true)
		{
			skipWhiteSpaceEx();

			Character nextChar = peekNextCharacterEx();

			if(nextChar == cOpenBracket)
			{
				if(state != separator)
				{
					const String& errMsg = System::GetSysMessage(sXML, EXML_CONTENTMODELDELIM,
						szBadContentModelDelim);

					errorDetected(Fatal, errMsg, EXML_CONTENTMODELDELIM);
				}
				state = token;

				if(bMixedSpec)
				{
					const String& errMsg = System::GetSysMessage(sXML, EXML_MIXEDSUBGROUPS,
						"mixed content specification cannot contain sub groups");

					errorDetected(Fatal, errMsg, EXML_MIXEDSUBGROUPS);
				}

				ElementContentSpec& child = pContentSpec->addChild();
				parseElementContentSpecList(element, &child);
				parseContentMultiplicity(element, &child);
			}
			else if(nextChar == '|' || nextChar == ',')
			{
				if(state != token)
				{
					const String& errMsg = System::GetSysMessage(sXML, EXML_CONTENTMODELDELIM,
						szBadContentModelDelim);

					errorDetected(Fatal, errMsg, EXML_CONTENTMODELDELIM);
				}
				state = separator;

				if(bMixedSpec && nextChar == ',')
				{
					const String& errMsg = System::GetSysMessage(sXML, EXML_MIXEDSEQUENCE,
						"element sequence is not permitted following '#PCDATA', use choice '|' instead");

					errorDetected(Fatal, errMsg, EXML_MIXEDSEQUENCE);
				}
				else if((theListType == choice && nextChar == ',') ||
						(theListType == seq && nextChar == '|') )
				{
					const String& errMsg = System::GetSysMessage(sXML, EXML_MIXSEQANDCHOICE,
						"illegal mix of choice and sequence elements within a single sub-element specification");

					errorDetected(Fatal, errMsg, EXML_MIXSEQANDCHOICE);
				}
				else if(theListType == any)
				{
					if(nextChar == '|')
					{
						pContentSpec->setListType(ElementContentSpec::Choice);
						theListType = choice;
					}
					else
					{
						pContentSpec->setListType(ElementContentSpec::Seq);
						theListType = seq;
					}
				}
				getNextCharacterEx();     // skip past the ',/|'
			}
			else if(nextChar == cCloseBracket)
			{
				if(state != token)
				{
					const String& errMsg = System::GetSysMessage(sXML, EXML_CONTENTMODELDELIM,
						szBadContentModelDelim);

					errorDetected(Fatal, errMsg, EXML_CONTENTMODELDELIM);
				}

				// check that this isn't an empty specification
				if(!bMixedSpec && pContentSpec->isEmpty())
				{
					//
					// This is unlikely to be reported due to the earlier error
					// with ()
					//
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_EMPTYCONTENTSPEC,
						"empty content specification in declaration of element type '{0}'"),
						element.getName().getRawName());

					errorDetected(Fatal, errMsg, EXML_EMPTYCONTENTSPEC);
				}
				getNextCharacterEx();     // skip past the ')'
				break; // we have eaten the closing bracket
			}
			else if(CharTypeFacet::IsNameChar(nextChar))
			{
				if(state != separator)
				{
					const String& errMsg = System::GetSysMessage(sXML, EXML_CONTENTMODELDELIM,
						szBadContentModelDelim);

					errorDetected(Fatal, errMsg, EXML_CONTENTMODELDELIM);
				}
				state = token;

				QName elementQName;
				parseQName(elementQName, sElementName, true, false);
				const String& elementName = elementQName.getRawName();
				
				ElementContentSpec* pChild = 0;

				if(theListType != choice || !pContentSpec->containsTerminal(elementName))
				{
					ElementContentSpec& child = pContentSpec->addTerminal(elementName);
					pChild = &child;
				}
				else if(bMixedSpec && m_features.m_bDoValidityChecks)
				{
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_MIXEDDUPLELEMENT,
						"duplicate element name '{0}' is not permitted in the mixed content specification for element '{1}'"),
						elementName,
						element.getName().getRawName());

					errorDetected(Error, errMsg, EXML_MIXEDDUPLELEMENT);
				}
				
				bool bMutliplicity = parseContentMultiplicity(element, pChild);
				if(bMutliplicity && bMixedSpec)
				{
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_MIXEDMULTIPLICITY,
						"multiplicity may not be specified for individual elements in the mixed content specification for element '{0}'"),
						element.getName().getRawName());

					errorDetected(Fatal, errMsg, EXML_MIXEDMULTIPLICITY);
				}
			}
			else if(nextChar == '*' || nextChar == '?' || nextChar == '+')
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_MULTIPLICITYPOS,
					"multiplicity specification '{0}' must follow immediately after element name or ')'"),
					nextChar.toString());

				errorDetected(Fatal, errMsg, EXML_MULTIPLICITYPOS);
				getNextCharacterEx(); // skip it
			}
			else
			{
				// oops, don't know what to do with the input
				// report the error
				unexpectedChar(nextChar, QC_T("element content specification"));
				// and recover
				const char* endTokens[] = {szCloseBracket, szCloseAngle};
				recoverPosition(2, endTokens, 2); // don't eat any delimiters
				break;
			}
		}

		if(m_features.m_bDoValidityChecks)
		{
			validatePENesting(startEntity, QC_T("element content specification"));
		}
	}

	return bContentSpecFound;
}

//=============================================================================
// ParserImpl::parseContentMultiplicity
// 
// Part of DTD processing, this function parses the multiplicity
// of a content specification
//
//=============================================================================
bool ParserImpl::parseContentMultiplicity(ElementType& /*element*/, ElementContentSpec* pSpec)
{
	bool bMultiplicityFound = false;
	if(skipNextCharConstantEx('*'))
	{
		if(pSpec) pSpec->setMultiplicity(ElementContentSpec::ZeroOrMore);
		bMultiplicityFound = true;
	}
	else if(skipNextCharConstantEx('+'))
	{
		if(pSpec) pSpec->setMultiplicity(ElementContentSpec::OneOrMore);
		bMultiplicityFound = true;
	}
	else if(skipNextCharConstantEx('?'))
	{
		if(pSpec) pSpec->setMultiplicity(ElementContentSpec::Optional);
		bMultiplicityFound = true;
	}

	return bMultiplicityFound;
}

//=============================================================================
// ParserImpl::parseConditionalDecl
// 
// Part of DTD processing, this function parses the conditional 
// declaration
//
// [61] conditionalSect ::= includeSect | ignoreSect
// [62] includeSect ::= '<![' S? 'INCLUDE' S? '[' extSubsetDecl ']]>'
// [63] ignoreSect ::= '<![' S? 'IGNORE' S? '[' ignoreSectContents* ']]>'
// [64] ignoreSectContents ::= Ignore ('<![' ignoreSectContents ']]>' Ignore)*
// [65] Ignore ::= Char* - (Char* ('<![' | ']]>') Char*)
//
//=============================================================================
bool ParserImpl::parseConditionalDecl()
{
	bool bConditionalDeclFound = false;

	if(skipNextStringConstantEx(sCondDecl))
	{
		const Entity& startEntity = m_scannerPos.getEntity();

		bConditionalDeclFound = true;

		if(parsingInternalDTDSubset())
		{
			const String& errMsg = System::GetSysMessage(sXML, EXML_CONDINTERNALSUBSET,
				"conditional sections are not permitted within the internal DTD subset");

			errorDetected(Fatal, errMsg, EXML_CONDINTERNALSUBSET);
		}

		skipWhiteSpaceEx();
		
		const String& type = getNextStringTokenEx(CharTypeFacet::NameChar);

		if(type.empty())
		{
			const String& errMsg = System::GetSysMessage(sXML, EXML_CONDKEYWORD,
				"missing conditional keyword: IGNORE/INCLUDE");

			errorDetected(Fatal, errMsg, EXML_CONDKEYWORD);
		}

		skipWhiteSpaceEx();

		Character nextChar = peekNextCharacterEx();
		if(nextChar != cOpenBrace)
		{
			unexpectedChar(nextChar, QC_T("conditional DTD section"));
			if(!nextChar.isEOF())
			{
				Scanner::SkipToDelimiter(m_scannerPos, QC_T("]]>"), CharTypeFacet::Any);
				Scanner::SkipNextStringConstant(m_scannerPos, QC_T("]]>"));
			}
		}
		else
		{
			getNextCharacterEx(); //get the opening brace

			const Entity& startEntity2 = m_scannerPos.getEntity();

			if(type.compare(sInclude)==0)
			{
				parseDTDMarkup(false /*internal subset*/,
				               true  /* conditional section */,
				               false /* expect EOF during parse*/);

			}
			else
			{
				if(type.compare(sIgnore)!=0)
				{
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_CONDKEYWORDERR,
						"invalid conditional keyword '{0}' expecting 'IGNORE' or 'INCLUDE'"),
						type);

					errorDetected(Fatal, errMsg, EXML_CONDKEYWORDERR);

					// and we assume IGNORE!
				}

				int ignoreCount = 1;
				while(ignoreCount > 0)
				{
					// Note: when in "ignore" mode we do not need to (in fact must not)
					// expand parameter entities - so we can use the basic scanner
					// functions
					const char* szCondSectStart = "<![";
					const char* szCondSectEnd   = "]]>";

					const char* conditionSectionTokens[2] = {szCondSectStart, szCondSectEnd};

					int delimFound = Scanner::SkipToDelimiters(m_scannerPos, 2, conditionSectionTokens);

					// If none of the delimiters were found, SkipToDelimiters() returns -1

					if(delimFound == -1)
					{
						break;
					}
					else
					{
						if(delimFound == 0) // section start
						{
							ignoreCount++;
						}
						else // section end
						{
							ignoreCount--;
						}

						//
						// It is not our job to skip over the terminating
						// section end, but we are allowed to skip over
						// intervening start/end pairs
						//
						if(ignoreCount)
						{
							Scanner::Skip(m_scannerPos, 3);
						}
					}
				}
			}

			//
			// Now, we should be in a position to just skip over the terminator - 
			// but you know how things are... We have to check and skip over the
			// rubbish if we find it.
			//
			if(!skipNextStringConstantEx(sCDSectEnd))
			{
				nextChar = Scanner::PeekNextCharacter(m_scannerPos);
				unexpectedChar(nextChar, sCondSection);
				if(!nextChar.isEOF())
				{
					Scanner::SkipToDelimiter(m_scannerPos, sCDATAEnd, CharTypeFacet::Any);
					Scanner::SkipNextStringConstant(m_scannerPos, sCDATAEnd);
				}
			}

			if(m_features.m_bDoValidityChecks)
			{
				validatePENesting(startEntity2, sCondSection);
				validatePENesting(startEntity, sCondSection);
			}
		}
	}

	return bConditionalDeclFound;
}

//=============================================================================
// ParserImpl::parseNotationDecl
// 
// This parses the DTD for a document - and is therefore quite involved
// 
// [82] NotationDecl ::= '<!NOTATION' S Name S (ExternalID | PublicID) S? '>'
//
// [75] ExternalID ::= 'SYSTEM' S SystemLiteral | 
//                     'PUBLIC' S PubidLiteral S SystemLiteral
//
// [83] PublicID ::= 'PUBLIC' S PubidLiteral
//
//
// Note: The above grammar is a little strange.  A better representation
// would be:
// [et] NotationDecl ::= '<!NOTATION' S Name S NotationID S? '>'
// [et] NotationID ::= 'SYSTEM' S SystemLiteral
//                   | 'PUBLIC' S PubidLiteral (S SystemLiteral)?
//
//=============================================================================
bool ParserImpl::parseNotationDecl()
{
	bool bNotationDeclFound = false;

	if(skipNextStringConstantEx(sNotationDecl))
	{
		bNotationDeclFound = true;

		const Entity& startEntity = m_scannerPos.getEntity();

		const String& name = parseName(QC_T("notation"), true, true, false);

		bool bNewNotation = true;

		if(name.size() && (m_notationSet.find(name) != m_notationSet.end()))
		{
			bNewNotation = false;

			if(m_features.m_bDoValidityChecks && m_features.m_bDoWarningChecks)
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_DUPLNOTATIONDECL,
					"duplicate declaration for notation '{0}'"),
					name);

				errorDetected(Warning, errMsg, EXML_DUPLNOTATIONDECL);
			}
		}

		skipRequiredWhitespaceAfter(sNotationName, sNotationDeclaration);

		//
		// Okay, what follows may be an EnternalId or PublicID
		//
		String systemLiteral, pubidLiteral;
		parseExternalID(systemLiteral, pubidLiteral, true, true, sNotationDeclaration);
		
		skipWhiteSpaceEx();

		//
		// now add the notation to the set
		// 
		if(bNewNotation)
		{
			m_notationSet.insert(name);
			
			if(m_npDTDEventHandler)
			{
				m_npDTDEventHandler->onNotationDecl(name, pubidLiteral, systemLiteral);
			}
		}

		// Finally, test if the Entity declaration is correctly terminated
		if(!skipNextCharConstantEx(cCloseAngle))
		{
			// Yikes! here we go again. 
			// report the error
			reportDeclTermError(QC_T("notation"), sCloseAngle);
			
			//
			// Luckily this is standard stuff and there is a function we can call to
			// do the dirty work!
			recoverPosition(1, &szCloseAngle, 0);
		}

		if(m_features.m_bDoValidityChecks)
		{
			validatePENesting(startEntity, QC_T("notation declaration"));
		}
	}

	return bNotationDeclFound;
}

//=============================================================================
// ParserImpl::parseReference
// 
// While scanning through the input stream, the parser has come across an "&"
// designating the start of a Reference.
//
// [67] Reference ::= EntityRef | CharRef
// [68] EntityRef ::= '&' Name ';'
// [66] CharRef ::= '&#' [0-9]+ ';' | '&#x' [0-9a-fA-F]+ ';'
//
// We are called with the '&' as the next input character.
//
// Error Handling
// --------------
// To enable error recovery, if we find an illegal
// reference then we attempt to recover by scanning to the
// the end of the reference - ie the ';' character.
//=============================================================================
String ParserImpl::parseReference(bool bResolveEntities,
                                  bool bAttribute)
{
	String strRet;

	Character nextChar;

	if(skipNextStringConstantEx(sCharRefStart))
	{
		// we are processing a CharRef

		int base=10;
		String strRef;

		//
		// If the reference starts with a "x" then we have a 
		// haxadecimal reference, otherwise it is a decimal number.
		//
		if(Scanner::SkipNextCharConstant(m_scannerPos, 'x'))
		{
			base=16;
			strRef = Scanner::GetNextContigHexString(m_scannerPos, nextChar);
		}
		else
		{
			strRef = Scanner::GetNextContigDecimalString(m_scannerPos, false, nextChar);
		}

		if(strRef.length() && nextChar == ';')
		{
			Scanner::Skip(m_scannerPos, 1); // skip over the ';'

			UCS4Char charRef = NumUtils::ToLong(strRef, base);
			
			//
			// Well-formedness constraint: Legal Character
			// xml 1.0 section 4.1
			// Character references must match the production for Char[2]
			//
			Character ch((charRef <= 0x10FFFF) ? charRef : 0);
			if(CharTypeFacet::IsValidChar(ch))
			{
				strRet = ch.toString();
			}
			else
			{
				String errRef;
				if(base==16) errRef += QC_T("x");
				errRef += strRef;

				//
				// come clean and own up that the character is valid
				// but we can't deal with it if our CharType isn't 
				// large enough to hold it
				//
				if(charRef > CharTypeFacet::s_MaxChar && charRef <= 0x10FFFF)
				{
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_INVALIDXMLCHARCONF,
						"character reference &#{0}; is not a valid XML character on this platform/configuration"),
						errRef);

					errorDetected(Fatal, errMsg, EXML_INVALIDXMLCHARCONF);
				}
				else
				{
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_INVALIDXMLCHAR,
						"character reference &#{0}; is not a valid XML character"),
						errRef);

					errorDetected(Fatal, errMsg, EXML_INVALIDXMLCHAR);
				}
				// no recovery reqd
			}
		}
		else
		{
			// Yikes! bad character reference.  Example of this is: "&#);"
			const String& errMsg = System::GetSysMessage(sXML, EXML_INVALIDCHARREF,
				"invalid character reference");

			errorDetected(Fatal, errMsg, EXML_INVALIDCHARREF);

			// now try to recover to the end of the reference
			recoverPosition(1, &szSemicolon, 0);
		}
	}
	else if(skipNextCharConstantEx('&'))
	{
		// we are processing a named ref
		const String& reference = parseRefName(true); //eats the name and ";"
		if(reference.size())
		{
			if(bResolveEntities)
			{
				translateNamedReference(reference, bAttribute, strRet);
			}
			else
			{
				strRet = sAmpersand + reference + CharType(';');
			}
		}
	}
	else
	{
		// this should not happen - it is a logic error not an input error
		QC_DBG_ASSERT(false);
	}

	return strRet;
}

//=============================================================================
// ParserImpl::parsePEReference
// 
// Parameter Entity references can occur almost anywhere within the DTD.  When 
// they are located, they are resolved immediately into their replacement text.
//
// [69] PEReference ::= '%' Name ';'
//
// Returns: true if the ScannerPosition is located at a PE reference; false
//          otherwise.
//
// Note: A standalone '%' that is not followed by a name is usually an error
//       except when parsing the name of an entity.
//
//=============================================================================
bool ParserImpl::parsePEReference(bool bMarkupDeclaration,
                                  bool bSoftEOF,
                                  bool bNameReqd)
{
	bool PERefFound = false;

	if(Scanner::SkipNextCharConstant(m_scannerPos, '%'))
	{
		const String& reference = parseRefName(bNameReqd); //eats the name and ";"

		if(reference.size())
		{
			// we are processing a named PE Reference
			PERefFound = true;
			m_bDTDContainsPEReferences = true;

			if(!bMarkupDeclaration && parsingInternalDTDSubset() )
			{
				// Well-formedness constraint: PEs in Internal Subset
				// In the internal DTD subset, PE references can occur
				// only where markup declarations can occur, not within
				// markup declarations

				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_PEININTERNALSUBSET,
					"parameter entity reference '%{0};' must not be used within markup in the internal DTD subset"),
					reference);
				
				errorDetected(Fatal, errMsg, EXML_PEININTERNALSUBSET);
			}

			EntityMap::const_iterator iter = m_peMap.find(reference);
			if(iter != m_peMap.end())
			{
				//
				// okay, we have found the reference as declared in the DTD.
				// Now, we play a trick of the light, by updating the
				// scanner position to that of the reference. with the next
				// position equal to the current position.
				
				// But we only read external parameter entities if we
				// are pemitted to do so!
				//
				Entity* pEntity = ((*iter).second.get());
				if(m_features.m_bResolveParameterEntities &&
				  (pEntity->isInternalEntity() || m_features.m_bProcessExternalParameterEntities))
				{
					ScannerPosition entityPos = pEntity->getStartPosition();

					entityPos.setNextPosition(m_scannerPos,
					                          bSoftEOF ? 
											  ScannerPosition::softEOF
											  : (m_bParsingEntityValue
					                             ? ScannerPosition::skip
					                             : ScannerPosition::space));

					m_scannerPos = entityPos;
				}
				else if(areCallbacksPermitted() && m_npEntityEventHandler)
				{
					m_npEntityEventHandler->onUnexpandedEntity(
						reference, EntityType::Parameter,
						pEntity->getPublicId(),
						pEntity->getSystemId(),
						pEntity->getBaseURI());
				}
			}
			else
			{
				undeclaredEntity(reference, EntityType::Parameter);
			}
		}
	}

	return PERefFound;
}


//=============================================================================
// ParserImpl::parseName
// 
// Parsing helper-function to extract a name or a NmToken from the input stream.
//
// [05] Name ::= (Letter | '_' | ':') (NameChar)*
// [07] NmToken ::= (NameChar)+
//
//=============================================================================
String ParserImpl::parseName(const String& type,
                             bool bReqd,
                             bool bExpectWhitespace,
                             bool bNmToken)
{
	size_t wsCount = 0;
	if(bExpectWhitespace)
	{
		wsCount = skipWhiteSpaceEx();
	}

	const String& name = getNextStringTokenEx(CharTypeFacet::NameChar);

	if((name.empty() && bReqd) || (!bNmToken && name.size() && !CharTypeFacet::IsNameStartChar(Character(name.data(), name.size()))))
	{
		String desc = type;
		if(!name.empty())
		{
			desc += QC_T(" '");
			desc += name;
			desc += sApos;
		}

		if(m_features.m_bNamespaceSupport)
		{
			const String& errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_NAMESTARTNS,
				szNSNameStart),
				desc);
			
			errorDetected(Fatal, errMsg, EXML_NAMESTARTNS);
		}
		else
		{
			const String& errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_NAMESTART,
				"{0} must start with a letter, '_' or ':'"),
				desc);
			
			errorDetected(Fatal, errMsg, EXML_NAMESTART);
		}
	}

	if(!name.empty())
	{
		if(m_features.m_bNamespaceSupport && !bNmToken && name.find(QC_T(":")) != String::npos)
		{
			const String& errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_NSCOLON,
				"{0} '{1}' must not contain ':' with namespaces enabled"),
				type, name);

			errorDetected(Fatal, errMsg, EXML_NSCOLON);
		}

		if(bExpectWhitespace && 0 == wsCount)
		{
			reportMissingWhiteSpaceBefore(name);
		}
	}

	return name;
}

//=============================================================================
// ParserImpl::parseQName
// 
// Parsing helper-function to extract a QName from the input stream.
//
// A QName is a production from the XML Namespaces recommendation
// (http://www.w3.org/TR/REC-xml-names/)
//
// QName stands for "Qualified Name" and is characterized by the following
// production
// 
// [ns06] QName ::= (Prefix ':')? LocalPart
//
// [ns04] NCName ::= (Letter | '_') (NCNameChar)*  // an xml name minuis the ':'
// [ns05] NCNameChar ::= ....                      // an xml namechar minuis the ':' 
// [ns07] Prefix ::= NCName
// [ns08] LocalPart ::= NCName
//
//=============================================================================
bool ParserImpl::parseQName(QName& qname,
                            const String& type,
                            bool bReqd,
                            bool bExpectWhitespace)
{
	bool bRet = false;

	//
	// If namespace support is switched off then we revert to the
	// old-fashioned name parsing
	//
	if(!m_features.m_bNamespaceSupport)
	{
		const String& name = parseName(type, bReqd, bExpectWhitespace, false);
		if(!name.empty())
		{
			qname.setRawName(name);
			bRet = true;
		}
	}
	else // Namespace support is on
	{
		size_t wsCount = 0;
		if(bExpectWhitespace)
		{
			wsCount = skipWhiteSpaceEx();
		}

		const String& name = getNextStringTokenEx(CharTypeFacet::NameChar);

		if((name.empty() && bReqd) || (name.size() && (name[0] == ':' || !CharTypeFacet::IsNameStartChar(Character(name.data(), name.size())))))
		{
			String desc = type;
			desc += QC_T(" '");
			desc += name;
			desc += sApos;

			const String& errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_NAMESTARTNS,
				szNSNameStart),
				desc);
		
			errorDetected(Fatal, errMsg, EXML_NAMESTARTNS);
		}

		if(!name.empty())
		{
			bRet = true;
			qname.setRawName(name);


			if(bExpectWhitespace && 0 == wsCount)
			{
				reportMissingWhiteSpaceBefore(name);
			}

			//
			// The QName will have automatically been divided into its local part and prefix part
			// but we still have to check that it is valid.
			//
			if(qname.getDelimPosition() == name.size()-1)
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_NSENDCOLON,
					"{0} '{1}' must not end with a ':' with namespaces enabled"),
					type, name);

				errorDetected(Fatal, errMsg, EXML_NSENDCOLON);
			}
			else
			{
				//
				// test for illegal subsequent colon
				//
				if(qname.getLocalName().find(sColon) != String::npos)
				{
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_NSMULTICOLON,
						"{0} '{1}' must not contain multiple colons with namespaces enabled"),
						type, name);

					errorDetected(Fatal, errMsg, EXML_NSMULTICOLON);
				}
			}
		}
	}

	return bRet;
}

//=============================================================================
// ParserImpl::parseRefName
// 
// Parsing helper-function to extract a reference from the input stream.
//=============================================================================
String ParserImpl::parseRefName(bool bReqd)
{
	const String& name = parseName(sEntityReference, bReqd, false, false);

	// skip over the terminating ';' 
	if(name.size() && !Scanner::SkipNextCharConstant(m_scannerPos, ';'))
	{
		const String& errMsg = System::GetSysMessage(sXML, EXML_REFNOTTERM,
			"entity reference not terminated correctly.  Expected ';'");

		errorDetected(Fatal, errMsg, EXML_REFNOTTERM);
		// no recovery needed/possible
	}
	return name;
}

//=============================================================================
// ParserImpl::testNextTokenType
// 
// Some people might argue that this function should be part of the Scanner,
// it has been placed inside Parser because it requires knowledge of the
// syntax of XML tags - something the Scanner doesn't otherwise require 
// knowledge of.
//
// Performance in this function is critical; at every juncture the Parser
// calls this function to see what XML tag is next in the input stream.
// 
// This function is preferable to each production testing its own start tag
// because that implies redundant calls to the scanner to get the next token
// and it also prevents the Parser from displaying meaningful error messages
// when the input stream contains an unrecognized token.
//
// Returns EndOfEntity (-1) if EOF is read from the input stream.
//
//=============================================================================
int ParserImpl::testNextTokenType(const TokenTableEntry* pTable,
                                  String& tokenFound,
                                  bool& bErrorReported)
{
	int retToken = 0;
	bErrorReported = false;
	tokenFound = String();
	
	// Note: the following characters have special meaning: 
	// w - white-space
	// n - name character
	// all other characters represent themselves

	ValueRestorer<ScannerPosition> positionRestorer(m_scannerPos, m_scannerPos);

	unsigned long flags = 0;

	size_t i=0;
	size_t maxTags=0;
	unsigned bitSet = 1;
	for(const TokenTableEntry* pMyTable=pTable; pMyTable->m_tokenType; pMyTable++, i++, bitSet <<= 1)
	{
		QC_DBG_ASSERT((sizeof(flags) * 8) > i);
		flags |= bitSet;
		maxTags++;
	}

	for(i=0; flags; i++)
	{
		unsigned long copyFlags = flags;
		const Character nextChar = Scanner::GetNextCharacter(m_scannerPos);

		if(nextChar.isEOF())
		{
			return EndOfEntity;
		}
		else
		{
			nextChar.appendToString(tokenFound);
		}

		size_t maxExprSize = 0; // size of the longest expression still in the running

		size_t j;
		unsigned bitTest;
		for(bitTest=1, j=0; j<maxTags; j++, bitTest <<= 1)
		{
			if(flags & bitTest)
			{
				bool bMatches = true; // assume a positive match in case
				                      // we have already passed the end of the test string

				if(i < pTable[j].m_length)
				{
					const CharType x = pTable[j].m_string[i];
					if(x == 'n')
					{
						bMatches = CharTypeFacet::IsNameChar(nextChar);
					}
					else if(x == 'w')
					{
						bMatches = CharTypeFacet::IsWhiteSpace(nextChar);
					}
					else
					{
						bMatches = (nextChar == x);
					}
				}

				if(bMatches)
				{
					flags |= bitTest;
				}
				else
				{
					flags &= ~bitTest;
				}
				
				//
				// if this tag is still in the running, take a look at its size
				// to set the maxExprSize
				//
				if(bMatches && pTable[j].m_length > maxExprSize)
				{
					maxExprSize = pTable[j].m_length;
				}
			}
		}

		//
		// okay, we have passed the next character through all the
		// expressions that were still in the running.  If only one
		// expression remains then we have arrived - but not so fast!
		// Only if the entire expression is satisfied can we truely
		// relax and return the tag to the caller - otherwise
		// we have to wait ...
		//
		if(((i+1) >= maxExprSize) && flags)
		{
			// hurrah!  One or more tags have been found.  This is what we wanted to know
			// Now we just have to find the best match - based on the size of the match
			//
			tokenFound = tokenFound.substr(0, maxExprSize);
			for(bitTest=1, j=0; j<maxTags; j++, bitTest <<= 1)
			{
				if((flags & bitTest) && pTable[j].m_length == maxExprSize)
				{
					return pTable[j].m_tokenType;
				}
			}
		}
		else if(!flags) // no matches at all!
		{
			// we now resort to the tags that were still in the running
			// before the last character was read
			String tagsIntended;
			size_t count=0;

			for(bitTest=1, j=0; j<maxTags; j++, bitTest<<=1)
			{
				if(copyFlags & bitTest)
				{
					if(count++)
					{
						//
						// Test to see if this is the last option available
						// by comparing the flags with the current bit set
						// off to a integer with the current bit set on.  If the
						// flags is greater, then we have more flags to go...
						//
						if((copyFlags & ~bitTest) > bitTest)
						{
							tagsIntended += QC_T(", "); 
						}
						else
						{
							tagsIntended += QC_T(" or "); 
						}
					}
					
					if(pTable[j].m_altString)
					{
						tagsIntended += StringUtils::FromLatin1(pTable[j].m_altString);
					}
					else
					{
						tagsIntended += StringUtils::FromLatin1(pTable[j].m_string);
					}
				}
			}

			// only output the "possibly intended..." message is the number of
			// possibilities is reasonable
			if(count <= 6) //arbitrary number
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_INVALIDTOKENEXP,
					"invalid token '{0}'. Probably intended {1}"),
					tokenFound, tagsIntended);

				errorDetected(Fatal, errMsg, EXML_INVALIDTOKENEXP);
				bErrorReported = true;
			}
		}
	}

	return retToken;

}

//=============================================================================
// ParserImpl::formatForPrint
// 
// Called with a character that needs to be displayed (normally in an error
// message).  As not all characters are printable, this formats the character
// so that it is printable.
//=============================================================================
String ParserImpl::formatForPrint(const Character& ch) const
{
	if(ch.isEOF())
	{
		return QC_T("end of file");
	}
	else
	{
		UCS4Char ucs = ch.toUnicode();
		if(ucs >= 0x20UL && ucs < 0x7FUL)
		{
			return StringUtils::FromLatin1(StringUtils::Format("'%c' (U+%04lX)", char(ch.first()), (unsigned long)ch.toUnicode()));
		}
		else
		{
			return StringUtils::FromLatin1(StringUtils::Format("U+%04lX", (unsigned long)ucs));
		}
	}
}

void ParserImpl::errorDetected(ErrorLevel level, const String& errMsg, long messageID)
{
	if(m_scannerPos.isValid())
	{
		errorDetected(level, errMsg, m_scannerPos, messageID);
	}
	else
	{
		throw XMLParsingException(m_docSystemId,
		                          StreamPosition(),
		                          level,
		                          messageID,
		                          errMsg);
	}
}

//=============================================================================
// ParserImpl::errorDetected
// 
// Called whenever an error condition is detected by the parser.
//
// The primary purpose for this routine is to
// a) record the fact that an error has occurred
// b) reduce the number of errors reported by filtering
//    out multiple fatal errors from a single location
// c) Inform the application by calling the ErrorEventHandler
//
// If the application has not provided us an ErrorEventHandler then
// we just swallow the error UNLESS it is Fatal.  Untrapped Fatal errors
// result in an XMLParsingException.  This gives a shorthand way for
// applications that aren't interested in reporting multiple errors and wish
// to disregard warnings and validity constraints to cease parsing on the
// first error found.
//=============================================================================
void ParserImpl::errorDetected(ErrorLevel level, const String& errMsg, const ScannerPosition& position, long messageID)
{
	m_bInErrorHandler = true;

	if(m_worstErrorFound < level)
	{
		m_worstErrorFound = level;
	}

	if(level > m_worstErrorFoundAtPosition 
		|| (level < Fatal && level == m_worstErrorFoundAtPosition)
		|| m_lastErrorOffset != position.getOffset()
		|| m_pLastErrorEntity != &position.getEntity())
	{
		m_worstErrorFoundAtPosition = level;
		m_lastErrorOffset = position.getOffset();
		m_pLastErrorEntity = &position.getEntity();

		if(m_npErrorEventHandler)
		{
			// create a sensible Context String
			// --------------------------------
			//
			// What we need is to backtrack to the previous newline
			// and get a string starting there.  However, this will not
			// allways be possible or desirable.  For very long lines, it doesn't make
			// sense to backtrack too far, the context of the error should still be
			// plain to see even with only a partial line.  Also, we can't backtrack
			// to the previous buffer - because we don't have back pointers, so the
			// most we can backtrack is to the start of the current buffer.
			//
			// The most we want to forward-track is to the next newline.  But again, if that
			// is very far away we only want a portion of the string.
			//
			// Also, we only want the text from a single entity.  It could be
			// confusing if we presented the user with a string of concatenated entities
			// - it shows too much of the workings of the parser.
			//
			size_t maxLen = 80;
			ScannerPosition firstPos = Scanner::BackTrackToChar(position, '\n', maxLen-10/* max chars to backtrack*/);

			size_t startCol = firstPos.getColumnNumber();

			//
			// Now scan forward from firstPos to the next newline
			//
			Character nextChar;
			String contextStr;

			try
			{
				contextStr = Scanner::GetNextContigString(
									firstPos,
									CharTypeFacet::Any,
									0,
									QC_T("\n"),
									nextChar,
									maxLen).asString();
			}
			catch(...)
			{
			}

			if(contextStr.size())
			{
				ContextString context(contextStr, startCol, position.getLineNumber());
				m_npErrorEventHandler->onError(level, messageID, errMsg, position, &context);
			}
			else
			{
				m_npErrorEventHandler->onError(level, messageID, errMsg, position, 0);
			}
		}
		else if(level == Fatal)
		{
			throw XMLParsingException(position.getSystemId(),
			                          position.getStreamPosition(),
			                          level,
			                          messageID,
			                          errMsg);
		}
	}

	//
	// Signal clean exit from error handler
	//
	m_bInErrorHandler = false;
}

//=============================================================================
// ParserImpl::resetParser
// 
// Prepare the parser to parse a new document
//=============================================================================
void ParserImpl::resetParser()
{
	m_worstErrorFound = NoError;
	m_worstErrorFoundAtPosition = NoError;
	m_lastErrorOffset = 0;
	m_pLastErrorEntity = 0;
	
	//
	// clear out the DTD entity maps
	//
	m_geMap.clear();
	m_peMap.clear();

	//
	// clear out the DTD element map
	//
	m_elementMap.clear();

	//
	// clear out the DTD Notation map
	//
	m_notationSet.clear();

	//
	// clear out the namespace prefix map
	//
	m_namespaceFrameVector.clear();
	m_namespaceFrameVector.push_back(NamespaceFrame());

	// Clear out the ID sets
	m_idSet.clear();
	m_idRefSet.clear();

	m_entityResolutionStack.clear();
	m_bXMLDeclSeen = false;
	m_bParseInProgress = false;
	m_bParsingDTD = false;
	m_bStandaloneDoc = false;
	m_bInErrorHandler = false;
	m_bParsingEntityValue = false;
	m_bParsingEntityDeclName = false;
	m_scannerPos = ScannerPosition();
	m_currentElementName = QC_T("/"); //root
	m_DTDName.erase();
	m_docSystemId.erase();
	m_docPublicId.erase();
	m_bHasDTD = false;
	m_bExternalDTDSubsetDeclared = false;
	m_bDTDContainsPEReferences = false;
	m_pCurrentElementContentSpec = NULL;
	m_pCurrentElementType = NULL;
	m_rpDTDEntity.release();
}

//=============================================================================
// ParserImpl::areCallbacksPermitted
// 
// Can be called at any time during the parsing of a document - tells whether
// or not any significant errors have been found.  If they have then 
// parsing events should no longer be passed on to the application.
//=============================================================================
bool ParserImpl::areCallbacksPermitted() const
{
	return (m_worstErrorFound < Fatal);
}

//=============================================================================
// ParserImpl::unexpectedChar
// 
// Helper function to report errors when illegal character (including EOF)
// found in the middle of the (logical) document.
//=============================================================================
void ParserImpl::unexpectedChar(const Character& x, const String& where)
{
	String errMsg;
	long errNum;

	if(x.isEOF())
	{
		if(m_scannerPos.getEntity().isExternalEntity())
		{
			errNum = EXML_UNEXPECTEDEOF;
			errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, errNum,
				"unexpected end of file within {0}"),
				where);
		}
		else
		{
			errNum = EXML_UNEXPECTEDENTITYEND;
			errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, errNum,
				"unexpected end of entity '{0}' within {1}"),
				m_scannerPos.getEntity().getName(), where);
		}
	}
	else
	{
		errNum = EXML_UNEXPECTEDCHAR;
		errMsg = MessageFormatter::Format(
			System::GetSysMessage(sXML, errNum,
			"unexpected character {0} within {1}"),
			formatForPrint(x), where);
	}

	errorDetected(Fatal, errMsg, errNum);
}

//=============================================================================
// ParserImpl::unexpectedToken
// 
// Helper function to report errors when unexpected tokens are found
//=============================================================================
void ParserImpl::unexpectedToken(int nextToken, const String& strToken,
                                 const String& next)
{
	long errNum;
	String errMsg;

	if(nextToken == EndOfEntity)
	{
		if(m_scannerPos.getEntity().isExternalEntity())
		{
			errNum = EXML_UNEXPECTEDTOKENEOF;
			errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, errNum,
				"unexpected end of file, expecting {0}"),
				next);
		}
		else
		{
			errNum = EXML_UNEXPECTEDTOKENEOE;
			errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, errNum,
				"unexpected end of entity '{0}' expecting {1}"),
				m_scannerPos.getEntity().getName(), next);
		}
	}
	else
	{
		String tok;
		if(nextToken == 0)
		{
			if(strToken.size() == 1)
			{
				tok = formatForPrint(Character(strToken.data(), strToken.size()));
			}
			else
			{
				tok = sApos + strToken + sApos;
			}
		}
		else
		{
			tok = StringUtils::FromLatin1(tokenDescriptionTable[nextToken]);
		}
		errNum = EXML_UNEXPECTEDTOKEN;
		errMsg = MessageFormatter::Format(
			System::GetSysMessage(sXML, errNum,
			"unexpected {0}, expecting {1}"),
			tok, next);
	}

	errorDetected(Fatal, errMsg, errNum);
}

//=============================================================================
// ParserImpl::skipRequiredWhitespaceAfter
// 
// Helper function to skip required white-space - and report an error
// when it is missing
//
//=============================================================================
bool ParserImpl::skipRequiredWhitespaceAfter(const String& after, const String& where)
{
	if(!skipWhiteSpaceEx())
	{
		reportMissingWhiteSpaceAfter(after, where);
		return false;
	}
	else
	{
		return true;
	}
}

//=============================================================================
// ParserImpl::skipRequiredWhitespaceBefore
// 
// Helper function to skip required white-space - and report an error
// when it is missing
//
//=============================================================================
bool ParserImpl::skipRequiredWhitespaceBefore(const String& before, const String& where)
{
	if(!skipWhiteSpaceEx())
	{
		reportMissingWhiteSpaceBefore(before, where);
		return false;
	}
	else
	{
		return true;
	}
}

//=============================================================================
// ParserImpl::skipNextStringConstantEx
// 
// Extended version of Scanner::SkipNextStringConstant that performs the
// same logic, but automatically expands Parameter Entities inline.
//=============================================================================
bool ParserImpl::skipNextStringConstantEx(const String& str)
{
	QC_DBG_ASSERT(!str.empty());

	ValueRestorer<ScannerPosition> positionRestorer(m_scannerPos, m_scannerPos);
	
	for(size_t i=0; i<str.size(); i++)
	{
		if(getNextCharacterEx() != str[i])
			return false;
	}

	// all the string's characters matched the input stream, so advance position
	// and return true.
	positionRestorer.release();
	return true;
}

//=============================================================================
// ParserImpl::getNextCharacterEx
// 
// Extended version of Scanner::GetNextCharacter that performs the
// same logic, but automatically expands Parameter Entities inline.
//=============================================================================
Character ParserImpl::getNextCharacterEx()
{
	if(m_bParsingDTD && Scanner::PeekNextCharacter(m_scannerPos) == '%')
	{
		if(parsePEReference(false, false, !m_bParsingEntityDeclName)) //eats the '%' even if not PERef
		{
			if(m_bParsingEntityValue)
			{
				return getNextCharacterEx();
			}
			else
			{
				return chSpace;
			}
		}
		else
		{
			return Character('%');
		}
	}
	else
	{
		return Scanner::GetNextCharacter(m_scannerPos);
	}
}

//=============================================================================
// ParserImpl::peekNextCharacterEx
// 
// Extended version of Scanner::PeekNextCharacter that performs the
// same logic, but automatically expands Parameter Entities inline.
//=============================================================================
Character ParserImpl::peekNextCharacterEx()
{
	if(m_bParsingDTD && Scanner::PeekNextCharacter(m_scannerPos) == '%')
	{
		ValueRestorer<ScannerPosition> positionRestorer(m_scannerPos, m_scannerPos);

		if(parsePEReference(false, false, !m_bParsingEntityDeclName)) //eats the '%' even if not PERef
		{
			if(m_bParsingEntityValue)
			{
				return peekNextCharacterEx();
			}
			else
			{
				return chSpace;
			}
		}
		else
		{
			return Character('%');
		}
	}
	else
	{
		return Scanner::PeekNextCharacter(m_scannerPos);
	}
}

//=============================================================================
// ParserImpl::skipNextCharConstantEx
// 
// Extended version of Scanner::PeekNextChar that performs the
// same logic, but automatically expands Parameter Entities inline.
//=============================================================================
bool ParserImpl::skipNextCharConstantEx(CharType x)
{
	if(peekNextCharacterEx() == x)
	{
		getNextCharacterEx();
		return true;
	}
	else
		return false;
}

//=============================================================================
// ParserImpl::skipWhiteSpaceEx
// 
// Pass over all whitespace - including white-space contained within expanded
// parameter entities
//
//=============================================================================
size_t ParserImpl::skipWhiteSpaceEx()
{
	size_t skipCount(0);

	while (CharTypeFacet::IsWhiteSpace(peekNextCharacterEx()))
	{
		getNextCharacterEx();
		skipCount++;
	}
	return skipCount;
}

//=============================================================================
// ParserImpl::getNextStringTokenEx
// 
// Return the next string of characters pointed at by position, where each
// character comforms to the classification determined by includeMask
// and does not conform to excludeMask.  If a string is returned then position
// is advanced accordingly.
//=============================================================================
String ParserImpl::getNextStringTokenEx(CharTypeFacet::Mask includeMask)
{
	if(m_bParsingDTD)
	{
		String strRet;
		Character nextChar;

		while( (nextChar = peekNextCharacterEx()).isEOF() != true)
		{
			if(CharTypeFacet::IsCharType(nextChar, includeMask))
			{
				getNextCharacterEx().appendToString(strRet);
			}
			else
			{
				break;
			}
		}
		return strRet;
	}
	else
	{
		Character nextChar;
		return Scanner::GetNextContigString(m_scannerPos, 
		                                    includeMask, 0, 
		                                    sNull, nextChar).asString();
	}
}

//=============================================================================
// ParserImpl::translateNamedReference
// 
// Given a named reference, return a translation.
//
// Note: This is one of the IMPORTANT functions because is has untold power!
//
// If the passed entity reference is one of the build-in references (eg &amp;)
// then the replacement text is simply the replacement character.  However,
// the reference may be an entity declared in the DTD.  In this case
// the entity can be arbitrarily complex with nested entities inside.
//
// We need to know where the entity is being invoked from.  If it is being
// invoked from within an attribute value then it is parsed differently to 
// when it is invoked from element content.
//=============================================================================
bool ParserImpl::translateNamedReference(const String& name, bool bAttribute,
                                         String& strRet)
{
	bool bEntityFound = false;

	EntityMap::const_iterator iter = m_geMap.find(name);
	if(iter != m_geMap.end())
	{
		Entity& entity = *((*iter).second.get());
		bEntityFound = true;

		//
		// We have found the referred entity - it has been declared in the
		// DTD.
		//

		//
		// We can only deal with parsed entities here...
		//
		if(!entity.isParsed())
		{
			const String& errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_UNPARSEDENT,
				"illegal reference to unparsed entity '{0}'"),
				getDisplayEntityName(name, EntityType::General));

			errorDetected(Fatal, errMsg, EXML_UNPARSEDENT);
			return false;
		}

		//
		// XML 1.0 4.4.4: A reference to an external entity value
		// is forbidden within an attribute value
		//
		if(bAttribute && entity.isExternalEntity())
		{
			const String& errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_EXTERNALENTINATTR,
				"reference to external entity '{0}' is forbidden within an attribute value"),
				getDisplayEntityName(name, EntityType::General));

			errorDetected(Fatal, errMsg, EXML_EXTERNALENTINATTR);
			return false;
		}

		//
		// The entity declaration has been found.  You'd think that was fine
		// but actually it may be an error if the declaration was external and
		// we're reading a standalone document
		//
		if(m_bStandaloneDoc && entity.isExternallyDeclared())
		{
			//
			// Check that it is not simply a re-defined standard entity (&amp; etc)
			//
			StdEntityMap::const_iterator stdIter = m_stdEntityMap.find(name);
			if(stdIter == m_stdEntityMap.end())
			{
				//
				// WFC: Entity Declared
				//
				// Even though the entity has been declared, it was declared
				// outside of the internal subset, which is illegal if we happen 
				// to be reading the document entity
				const bool bExternalReference = (m_scannerPos.getEntity().getType() != EntityType::Document);
				if(!bExternalReference)
				{
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_EXTERNALENTSA,
						"reference to externally declared entity '{0}' is not permitted from a standalone document"),
						getDisplayEntityName(name, EntityType::General));

					errorDetected(Fatal, errMsg, EXML_EXTERNALENTSA);
				}

				//
				// VC: Standalone Document
				//
				// Externally declared entities may not be referenced within 
				// a standalone document!
				//
				// Note: this is subtely different to the above WFC, because
				// it counts for any entity references, appearing anywhere
				// in the document - including the DTD subset.
				// 
				if(m_features.m_bDoValidityChecks && m_scannerPos.getEntity().getTopParent()->getType() == EntityType::Document)
				{
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_EXTERNALENTSA,
						"reference to externally declared entity '{0}' is not permitted from a standalone document"),
						getDisplayEntityName(name, EntityType::General));

					errorDetected(Error, errMsg, EXML_EXTERNALENTSA);
				}
			}
		}


		// Now things get a little interesting.  When the entity declaration
		// was parsed, we stored a stream containing the replacement text.  We
		// now want to parse that replacement text as if it is part of the
		// document.  So we obtain a ScannerPosition for the start of the
		// stream and then parse the data.
		//
		// Note: If the entity reference is contained within an attribute
		// value then it requires normalisation and any mark-up contained
		// therein is not part of the document structure (and is 
		// actually illegal due to a well-formedness constraint).
		// 
		// Note also: we would get into a terrible pickle if one entity
		// referenced itself (directly or indirectly) recursively.
		// The XML 1.0, 4.1 spec makes this illegal anyway, so we should
		// test for it here. 
		//
		// We have a list (treated like a stack)
		// of entities currently being resolved.  Each time through this
		// code (remember this will be called recursively) we push the name
		// of the entity being resolved onto the stack.  If it is already on 
		// there then we have a recursion error.  We push and pop from the
		// back to make traversal more logical.
		//

		//
		// Performance optimisation... simple internal entities that do not
		// contain markup and are part of element content do not need to be
		// re-parsed, nor do we need to check for recursion etc.  We could
		// pass their value back as the returned string, but we may get
		// better performance by keeping the data as a BufferRange.
		//
		if(m_features.m_bResolveGeneralEntities)
		{
			bool bSimpleEntity = false;
			if(!bAttribute && entity.isInternalEntity())
			{
				InternalEntity& intEntity = static_cast<InternalEntity&>(entity);
				bool bCharDataAllowed = (!m_pCurrentElementType || m_pCurrentElementType->allowsCharData());
				if(intEntity.isSimpleData() && bCharDataAllowed)
				{
					bSimpleEntity = true;
					// Hurrah, we have a performance improving situation

					if(areCallbacksPermitted() && m_npEntityEventHandler)
					{
						m_npEntityEventHandler->onStartEntity(name,
															  EntityType::General);
					}

					const BufferRange& range = intEntity.getBufferRange();
					if(range.getSize())
					{
						reportCharData(range);
					}

					if(areCallbacksPermitted() && m_npEntityEventHandler)
					{
						m_npEntityEventHandler->onEndEntity(name,
															EntityType::General);
					}
				}
			}

			if(!bSimpleEntity)
			{
				//
				// Check for self-recursion
				//
				if(std::find(m_entityResolutionStack.begin(), m_entityResolutionStack.end(), name)
					!= m_entityResolutionStack.end())
				{
					String stack;

					EntityStack::const_iterator stackIter;

					for(stackIter=m_entityResolutionStack.begin();
						stackIter!=m_entityResolutionStack.end(); ++stackIter)
					{
						stack += (*stackIter);
						stack += QC_T("->");
					}

					stack += name;

					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_ENTOTYRECURSION,
						"recursive reference to entity '{0}'. Reference path: {1}"),
						getDisplayEntityName(name, EntityType::General),
						stack);

					errorDetected(Fatal, errMsg, EXML_ENTOTYRECURSION);
				}
				//
				// So, this is not a recursive call to ourselves so we can
				// expand the entity - but only if the application wants
				// that.  Otherwise we leave it unexpanded and inform
				// the app that this is what we've done.
				//
				else if(!entity.isExternalEntity() || m_features.m_bProcessExternalGeneralEntities)
				{
					// push the entity onto the back of the list
					m_entityResolutionStack.push_back(name);
					
					// Save the current value of m_scannerPos and restore
					// when leaving this scope

					ValueRestorer<ScannerPosition> 
							positionRestorer(m_scannerPos, entity.getStartPosition());

					//
					// If we are parsing an entity reference within an attribute, then we
					// need to recursively process the entity (looking for sub-entities etc)
					// and the end result will be a string.
					// Alternatively, if we are parsing element content, then we don't need
					// to create a result string and we allow the normal event mechanisms
					// to deal with the content.
					//
					if(bAttribute)
					{
						//strRet = parseAndNormalizeAttValue(bCData, false, 0, bNormalized);
						parseAndRecurseAttributeValue(0/*no delimiter*/, strRet);
					}
					else
					{
						if(areCallbacksPermitted() && m_npEntityEventHandler)
						{
							m_npEntityEventHandler->onStartEntity(name,
																  EntityType::General);
						}

						parseContent(/* entity content */ true);

						if(areCallbacksPermitted() && m_npEntityEventHandler)
						{
							m_npEntityEventHandler->onEndEntity(name,
																EntityType::General);
						}
					}

					// and pop the entity back off the list
					size_t stackSize = m_entityResolutionStack.size();
					QC_DBG_ASSERT(stackSize!=0);
					if(stackSize)
					{
						QC_DBG_ASSERT(m_entityResolutionStack.back() == name);
						m_entityResolutionStack.pop_back();
					}
				}
				//
				// Valid entity not expanded, inform the application
				//
				else if(areCallbacksPermitted() && m_npEntityEventHandler)
				{
					m_npEntityEventHandler->onUnexpandedEntity(
						name, EntityType::General,
						entity.getPublicId(),
						entity.getSystemId(),
						entity.getBaseURI());
				}
			}
		}
		else // not expanding general entities
		{
			if(areCallbacksPermitted() && m_npEntityEventHandler)
			{
				m_npEntityEventHandler->onUnexpandedEntity(
					name, EntityType::General,
					entity.getPublicId(),
					entity.getSystemId(),
					entity.getBaseURI());
			}
		}
	}
	else // not in DTD entity map
	{
		//
		// Is it a standard entity?
		//
		StdEntityMap::const_iterator stdIter = m_stdEntityMap.find(name);
		if(stdIter != m_stdEntityMap.end())
		{
			strRet = (*stdIter).second;
			bEntityFound = true;
		}
		else
		{
			undeclaredEntity(name, EntityType::General);
		}
	}

	return bEntityFound;
}

//==============================================================================
// ParserImpl::undeclaredEntity
//
// Called when an undeclared entity reference is read.
// See WFC: Entity Declared
// and VC: Entity Declared
//==============================================================================
void ParserImpl::undeclaredEntity(const String& name, const EntityType& type)
{
	const String& errMsg = MessageFormatter::Format(
		System::GetSysMessage(sXML, EXML_UNDECLENTITY,
		"reference to undeclared entity '{0}'"),
		getDisplayEntityName(name, type));

	//
	// We have come across an undeclared entity.
	//
	// In some cases an undeclared entity is a WFC, in others it is a VC.
	//
	// According to XML 1.0, 4.1 WFC: Entity Declared, we have a WFC if
	// -  the reference occurs outside of the external dtd subset and (any) PEs
	// -  its a document without a DTD, or
	// -  a document with standalone=yes, or
	// -  a document with only an internal DTD subset that contains no
	//    PE references
	//
	// The problem with the above is that references to undeclared references
	// may be ignored when running in non-validating mode - yet the application
	// may not want to perform full validation.
	//
	// In this case the application will either have to recognize the sitation by
	// overriding onUnexpandedEntity(), or enable warning tests.
	//
	const bool bExternalReference = (m_scannerPos.getEntity().getType() == EntityType::Parameter)
	                             || (m_scannerPos.getEntity().getTopParent()->getType() != EntityType::Document);

	if(!bExternalReference && (!m_bHasDTD || m_bStandaloneDoc ||
	  (!m_bExternalDTDSubsetDeclared && !m_bDTDContainsPEReferences)))
	{
		// Well-formedness constraint: Entity Declared
		errorDetected(Fatal, errMsg, EXML_UNDECLENTITY);
	}
	else
	{
		//
		// Otherwise it's a validity constraint
		// VC: Entity Declared
		//
		if(m_features.m_bDoValidityChecks)
		{
			errorDetected(Error, errMsg, EXML_UNDECLENTITY);
		}
		else if(m_features.m_bDoWarningChecks)
		{
			errorDetected(Warning, errMsg, EXML_UNDECLENTITY);
		}

		if(areCallbacksPermitted() && m_npEntityEventHandler)
		{
			m_npEntityEventHandler->onUnexpandedEntity(
				name, type,
				sNull, sNull, sNull);
		}
	}
}

//=============================================================================
// ParserImpl::recoverPosition
// 
// This routine is used after an error situation has occurred
// and it attempts to position the scanner at an appropriate place in the
// input stream so that parsing can continue.
//
// The recovery task can be performed with varying degress of success - but
// the task is problematic and its implementation un-scientific.
//
// The approach we have adopted is that we will be passed a list of termination tokens
// representing the possible termination tokens for the grammar production
// currently being parsed.  If any of these are found then they will be eaten and
// control is returned.
//
// While searching for these tokens, we also keep an eye open for the "<" token
// and eof because we never want to scan past them.
// 
// Note: An "expanded" form of this function has not been created.  This
// is deliberate because it should not be necessary to expand PEs during
// recovery.  If the document is well-formed then the closing delimiter should
// appear in the same entity as the opening delimiter - which by definition has
// already been seen and is therefore part of the ScannerPosition stack.
//=============================================================================
void ParserImpl::recoverPosition(size_t numTokens,
                                 const char* endTokens[],
                                 size_t ixEatableToken)
{
	const size_t MaxTokens = 10;
	const char* myTokens[MaxTokens];
	
	// numTokens must be less than the size of the array that we have allocated
	QC_DBG_ASSERT(numTokens < MaxTokens);
	
	size_t i;
	for(i=0; i<numTokens; i++)
	{
		myTokens[i] = endTokens[i];
	}

	myTokens[i] = "<";

	int skipRC = Scanner::SkipToDelimiters(m_scannerPos, numTokens+1, myTokens);

	if(skipRC == -1 || skipRC == (int)numTokens) // i.e. the cOpenAngle token that we added
	{
		// no additional skipping reqd
	}
	else
	{
		QC_DBG_ASSERT(skipRC < (int)numTokens);
		// if the token found is "eatable", then swallow it
		if(skipRC >= (int)ixEatableToken)
		{
			Scanner::SkipNextStringConstant(m_scannerPos, myTokens[skipRC]);
		}
	}
}

//==============================================================================
// ParserImpl::getDisplayEntityName
//
// Helper method to format an entity name for display in error messages.
//==============================================================================
String ParserImpl::getDisplayEntityName(const String& name, const EntityType& type) const
{
	String ret = (type == EntityType::Parameter) ? QC_T("%")
	                                             : QC_T("&"); 
	ret += name;
	ret += QC_T(";");
	return ret;
}

//=============================================================================
// ParserImpl::getElement
// 
// Locate and return an ElementType from our internal map.
//
// Returns a NULL ptr if no matching entry exists.
//
//=============================================================================
AutoPtr<ElementType> ParserImpl::getElement(const QName& name) const
{
	ElementMap::const_iterator iter = m_elementMap.find(name.getRawName());
	if(iter != m_elementMap.end())
	{
		return (*iter).second;
	}
	else
	{
		return NULL;
	}
}

//=============================================================================
// ParserImpl::addElement
// 
// Add a new element to our map
//
//=============================================================================
AutoPtr<ElementType> ParserImpl::addElement(const QName& name)
{
	QC_DBG_ASSERT(m_elementMap.find(name.getRawName()) == m_elementMap.end());
	ElementType* pElementType = new ElementType(name);
	m_elementMap[name.getRawName()] = pElementType;
	return pElementType;
}

//=============================================================================
// ParserImpl::validateDTD
// 
// Validate the DTD once it has been parsed in its entirety
//
// Note: The ScannerPosition will be located immediately after the DTD
//=============================================================================
void ParserImpl::validateDTD()
{
	//
	// Validate all Element Definitions
	//
	ElementMap::const_iterator elemIter;
	for(elemIter=m_elementMap.begin(); elemIter != m_elementMap.end(); ++elemIter)
	{
		(*elemIter).second->validate(*this);
	}

	//
	// Validate all General Entity Declarations
	//
	EntityMap::const_iterator entIter;
	for(entIter=m_geMap.begin(); entIter != m_geMap.end(); ++entIter)
	{
		(*entIter).second->validate(*this);
	}
}

//==============================================================================
// ParserImpl::getEntity
//
// Return a (ref-counted) ptr to the named Entity
//==============================================================================
AutoPtr<Entity> ParserImpl::getEntity(const String& name) const
{
	EntityMap::const_iterator iEnt = m_geMap.find(name);
	if(iEnt == m_geMap.end())
		return 0;
	else
		return (*iEnt).second;
}

//==============================================================================
// ParserImpl::addElementId
//
// When we locate an element with an ID parameter, we add it to our set so that
// IDREFs can be checked later.
//==============================================================================
bool ParserImpl::addElementId(const String& id)
{
	if(m_idSet.find(id) != m_idSet.end())
		return false;
	else
	{
		m_idSet.insert(id);
		return true;
	}
}

//==============================================================================
// ParserImpl::addElementIdRef
//
// Make a note of an IDREF attribute - so that it can be checked against the
// list of valid IDs when we have processedthe entire document.
//==============================================================================
void ParserImpl::addElementIdRef(const String& id)
{
	m_idRefSet.insert(id);
}

//==============================================================================
// ParserImpl::postRootValidityChecks
//
// Perform the checks that can only be done once the document root element
// has been parsed
//==============================================================================
void ParserImpl::postRootValidityChecks()
{
	QC_DBG_ASSERT(m_features.m_bDoValidityChecks);

	// Validity constraint: IDREF

	IdSet::const_iterator i;
	for(i=m_idRefSet.begin(); i!=m_idRefSet.end(); ++i)
	{
		if(m_idSet.find(*i) == m_idSet.end())
		{
			const String& errMsg = MessageFormatter::Format(
				System::GetSysMessage(sXML, EXML_MISSINGID,
				"an IDREF attribute refers to the ID '{0}' which is not present in the document"),
				*i);

			errorDetected(Error, errMsg, EXML_MISSINGID);
		}
	}
}

//==============================================================================
// ParserImpl::processNamespacePrefixes
//
//
// This function enables namespace support by:-
// 1) locating the xmlns attributes (if any)
// 2) optionally - removing the xmlns attributes from the list so that
//    the application doesn't need to worry about them
// 3) updating the namespace prefix map with any newly declared prefixes
// 4) qualifying any prefixed attributes
// 5) testing for duplicate attrbiutes (by virtue of same namsepace uri)
// 6) qualifying the element name
//
// The caller is responsible for "scoping" the namespace prefix map
//
// Note: A list of prefix deltas is created to report new and changed namespace
// declarations to the application. A future enhancement might be to calculate
// this list (at the cost of increased execution time) using the difference
// between the current namespace frame and the previous namespace frame. The
// previous non-default namespace frame is guaranteed to a subset of the current
// non-default namespace frame (because namespaces, other than the default
// namespace, can't be "undeclared"). See "Namespaces in XML" section 2 for
// proof.
//
// Return a bool indicating if any namespace declaration were encountered.
//==============================================================================
bool ParserImpl::processNamespacePrefixes(QName& elementName, AttributeSet& attrs)
{
	QC_DBG_ASSERT(m_features.m_bNamespaceSupport);

	bool bNewNamespaceFrameCreated = false;
	size_t i;

	for(i=0; i<attrs.size(); i++)
	{
		const Attribute& attr = *attrs.getAttribute(i).get();
		const String& attributeValue = attr.getValue();
		const String& prefix = attr.getName().getPrefix();
		const String& rawName = attr.getName().getRawName();
		const String& localName = attr.getName().getLocalName();
		bool bDefaultNamespace = (rawName == sNSPrefix);

		if(bDefaultNamespace || prefix == sNSPrefix)   // is it a namespace declaration?
		{
			if(!bNewNamespaceFrameCreated)
			{
				// Create new namespace frame, initialized with a copy of the current frame.
				// Note: The NamespaceFrame constructor employed does not get a copy of the delta PrefixList.
				QC_DBG_ASSERT(!m_namespaceFrameVector.empty());
				const NamespaceFrame& currentFrame = m_namespaceFrameVector.back();
				m_namespaceFrameVector.push_back(NamespaceFrame(currentFrame.m_prefixMap, currentFrame.m_defaultURI));

				bNewNamespaceFrameCreated = true;
			}

			QC_DBG_ASSERT(bNewNamespaceFrameCreated);

			NamespaceFrame& currentFrame = m_namespaceFrameVector.back();

			//
			// This test is a little unusual because it is not a validity constraint
			// but it is an error (as opposed to Fatal)
			//
			// See http://www.w3.org/2000/09/xppa for details about why relative URIs
			// should not be permitted.
			//
			if(m_features.m_bRelativeNamespaceURITestEnabled && attributeValue.size())
			{
				//
				// How to decide if the URI is relative?
				// 
				// Refer to RFC1808: Relative Uniform Resource Locators
				// which indicates a relative URL does not have a "scheme"
				// which is another word for protocol.
				//
				// So we look for a protocol, and if we can't find it assume
				// it must be a relative uri.
				//
				bool bRelativeURI = (attributeValue.find(QC_T(":"), 0) == String::npos);

				if(bRelativeURI)
				{
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_RELATIVENSURI,
						"the namespace URI '{0}' represents a relative URI"),
						attributeValue);

					errorDetected(Error, errMsg, EXML_RELATIVENSURI);
				}
			}

			if(bDefaultNamespace)
			{
				if(currentFrame.m_defaultURI != attributeValue)
				{
					// Set the default namespace URI...
					currentFrame.m_defaultURI = attributeValue;

					// ... and make a note of it in our delta list
					currentFrame.m_deltaPrefixList.push_back(std::make_pair(false, String()));
				}
			}
			else // !bDefaultNamespace
			{
				QC_DBG_ASSERT(!localName.empty());

				if(attributeValue.empty())
				{
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_NSURIBLANK,
						"the namespace URI for the prefix '{0}' must have a value"),
						localName);

					errorDetected(Fatal, errMsg, EXML_NSURIBLANK);
				}

				bool bChangedPrefix = false;
				bool bNewPrefix = false;
				
				const NamespaceFrame::PrefixMap::iterator& prefixIter = currentFrame.m_prefixMap.find(localName);
				if(prefixIter != currentFrame.m_prefixMap.end())
				{
					bChangedPrefix = (attributeValue != (*prefixIter).second);
					if(bChangedPrefix)
					{
						(*prefixIter).second = attributeValue;
					}
				}
				else
				{
					currentFrame.m_prefixMap.insert(std::pair<String, String>(localName, attributeValue));
					bNewPrefix = true;
				}

				if(bNewPrefix || bChangedPrefix)
				{
					// Detected a material namespace prefix change.
					// Either it previously did not exist or it's value was not equal to "attributeValue".
					currentFrame.m_deltaPrefixList.push_back(std::make_pair(bNewPrefix, localName));
				}
			}

			// If namespace declarations are NOT supposed to be reported to the
			// application, then remove the xmlns: attributes from the attribute map.
			if(!m_features.m_bReportNamespaceDeclarations)
			{
				attrs.removeAttribute(i--);
				continue;
			}
		}
	}

	//
	// resolve the element name
	//
	resolveNamespace(elementName, false);

	//
	// and resolve all the attribute names
	//
	for(i=0; i<attrs.size(); i++)
	{
		Attribute& attr = *attrs.getAttribute(i).get();

		//
		// NOTE
		// ----
		// This uses direct access to the m_name member of Attribute
		// and updates that value in place.
		//
		resolveNamespace(attr.m_name, true);
	}

	//
	// If there are multiple namespace prefixes in scope
	// then we are obliged to check for duplicate attributes
	//
	NamespaceFrame& currentFrame = m_namespaceFrameVector.back();
	size_t numNamespacePrefixes = currentFrame.m_prefixMap.size();
	if(numNamespacePrefixes > 1)
	{
		std::set<QName, std::less<QName> > attrSet;
		for(i=0; i<attrs.size(); i++)
		{
			Attribute& attr = *attrs.getAttribute(i).get();
			if(!attrSet.insert(attr.getName()).second)
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_DUPLNSATTR,
					"duplicate attribute '{0}' within namespace URI '{1}'"),
					attr.getName().getLocalName(),
					attr.getName().getNamespaceURI());

				errorDetected(Fatal, errMsg, EXML_DUPLNSATTR);
			}
		}
	}

	return bNewNamespaceFrameCreated;
}


//==============================================================================
// ParserImpl::reportStartNamespacePrefixes
//
// For each namespace prefix in the delta list, call the event handler 
// functions to inform the application about namespace mapping changes.
//
// Note: reportEndNamespacePrefixes guarantees to call 
// onNamespaceEnd/onNamespaceChange for a prefix mapping in the reverse order
// to which this function calls onNamespaceStart/onNamespaceChange.
//==============================================================================
void ParserImpl::reportStartNamespacePrefixes()
{
	QC_DBG_ASSERT(m_features.m_bNamespaceSupport);

	if(!(areCallbacksPermitted() && m_npContentEventHandler))
		return;

	// References to the current and previous frame namespace prefix mappings
	const NamespaceFrame& currentFrame = m_namespaceFrameVector.back();
	const NamespaceFrame::PrefixMap& currentPrefixMap = currentFrame.m_prefixMap;
	const NamespaceFrame::PrefixList& deltaPrefixList = currentFrame.m_deltaPrefixList;

	QC_DBG_ASSERT(!deltaPrefixList.empty());

	//
	// For each new/changed namespace...
	//
	for(NamespaceFrame::PrefixList::const_iterator iter = deltaPrefixList.begin(); iter!=deltaPrefixList.end(); ++iter)
	{
		const bool bNew = (*iter).first;
		const String& prefix = (*iter).second;
		
		String newURI;
		const bool bDefaultNamespace = prefix.empty();

		if(bDefaultNamespace)
		{
			// New URI for the default namespace
			newURI = currentFrame.m_defaultURI;
		}
		else
		{
			// New URI for the "prefix" namespace mapping
			const NamespaceFrame::PrefixMap::const_iterator& currIter = currentPrefixMap.find(prefix);
			QC_DBG_ASSERT(currIter != currentPrefixMap.end());
			newURI = (*currIter).second;
		}

		if(bNew)
		{
			m_npContentEventHandler->onNamespaceBegin(prefix, newURI);
		}
		else
		{
			//
			// The prefix mapping for "prefix" replaces an existing namespace mapping
			// By definition, it has a distinctly different value in the current frame.
			//
			QC_DBG_ASSERT(m_namespaceFrameVector.size() >= 2);
			const NamespaceFrame& previousFrame = m_namespaceFrameVector[m_namespaceFrameVector.size()-2];
			const NamespaceFrame::PrefixMap& previousPrefixMap = previousFrame.m_prefixMap;

			String currentURI;

			if(bDefaultNamespace)
			{
				// "prefix" mapping for the default namespace in the previous frame
				currentURI = previousFrame.m_defaultURI;
			}
			else
			{
				//
				// Note: This second "find" (costly in terms of execution time) is only
				// executed when a prefix mapping is overiden.
				//
				const NamespaceFrame::PrefixMap::const_iterator& prevIter = previousPrefixMap.find(prefix);
				QC_DBG_ASSERT(prevIter != previousPrefixMap.end());
				currentURI = (*prevIter).second;
			}

			m_npContentEventHandler->onNamespaceChange(prefix,  currentURI,
			                                           newURI, false /* not restoring */);
		}
	}
}


//==============================================================================
// ParserImpl::reportEndNamespacePrefixes
//
// For each namespace prefix in the delta list, call the virtual callback 
// functions to inform the application about namespace mapping changes.
//
// Note: We guarantee to call onNamespaceEnd/onNamespaceChange for a prefix
// mapping in the reverse order to which reportStartNamespacePrefixes called 
// onNamespaceStart/onNamespaceChange.
//==============================================================================
void ParserImpl::reportEndNamespacePrefixes()
{
	QC_DBG_ASSERT(m_features.m_bNamespaceSupport);

	if(!(areCallbacksPermitted() && m_npContentEventHandler))
		return;

	// References to the current and previous frame namespace prefix mappings
	const NamespaceFrame& currentFrame = m_namespaceFrameVector.back();
	const NamespaceFrame::PrefixMap& currentPrefixMap = currentFrame.m_prefixMap;
	const NamespaceFrame::PrefixList& deltaPrefixList = currentFrame.m_deltaPrefixList;

	QC_DBG_ASSERT(!deltaPrefixList.empty());

	//
	// For each new/changed namespace...
	//
	for(NamespaceFrame::PrefixList::const_reverse_iterator  iter = deltaPrefixList.rbegin(); iter!=deltaPrefixList.rend(); ++iter)
	{
		const bool bNew = (*iter).first;
		const String& prefix = (*iter).second;
		
		String currentURI;
		const bool bDefaultNamespace = prefix.empty();

		if(bDefaultNamespace)
		{
			// URI for the default namespace is ending
			currentURI = currentFrame.m_defaultURI;
		}
		else
		{
			// URI for the "prefix" namespace mapping is ending
			const NamespaceFrame::PrefixMap::const_iterator& currIter = currentPrefixMap.find(prefix);
			QC_DBG_ASSERT(currIter != currentPrefixMap.end());
			currentURI = (*currIter).second;
		}

		if(bNew)
		{
			// A prefix namespace mapping for "prefix" didn't exist in the previous frame
			m_npContentEventHandler->onNamespaceEnd(prefix, currentURI);
		}
		else
		{
			//
			// The prefix mapping for "prefix" is being restored from the previous frame.
			// By definition, it has a distinctly different value in the current frame.
			//
			QC_DBG_ASSERT(m_namespaceFrameVector.size() >= 2);
			const NamespaceFrame& previousFrame = m_namespaceFrameVector[m_namespaceFrameVector.size()-2];
			const NamespaceFrame::PrefixMap& previousPrefixMap = previousFrame.m_prefixMap;

			String restoredURI;

			if(bDefaultNamespace)
			{
				// "prefix" mapping for the default namespace in the previous frame
				restoredURI = previousFrame.m_defaultURI;
			}
			else
			{
				//
				// Note: This second "find" (costly in terms of execution time) is only
				// executed when a prefix mapping is overiden.
				//
				const NamespaceFrame::PrefixMap::const_iterator& prevIter = previousPrefixMap.find(prefix);
				QC_DBG_ASSERT(prevIter != previousPrefixMap.end());
				restoredURI = (*prevIter).second;
			}
			
			m_npContentEventHandler->onNamespaceChange(prefix, currentURI,
			                                           restoredURI, true /* restoring */);
		}
	}
}

//==============================================================================
// ParserImpl::resolveNamespace
//
// Given a QName, resolve the namespace URI using the active namespace prefix
// map.
//
// If the prefix is empty then we use the URI of the default namespace, but this
// is only true for element names, attributes don't have a default namespace.
//
//==============================================================================
void ParserImpl::resolveNamespace(QName& qname, bool bAttribute)
{
	const String& prefix = qname.getPrefix();

	//
	// From XML Namespaces 4 "Using Qualified Names"...
	// "The prefix "xmlns" is used only for namespace bindings and is not itself
	// bound to any namespace name
	//
	// Note: The "xml" prefix (used in xml:space) is defined by default
	//
	if(prefix != sNSPrefix)
	{
		// If the prefix is empty then use the default namespace
		// but remember this is not the case for attributes
		if(prefix.empty())
		{
			if(!bAttribute)
			{
				qname.setNamespaceURI(m_namespaceFrameVector.back().m_defaultURI);
			}
		}
		else
		{
			const NamespaceFrame& currentFrame = m_namespaceFrameVector.back();
			NamespaceFrame::PrefixMap::const_iterator iPrefix = currentFrame.m_prefixMap.find(prefix);
			if(iPrefix != currentFrame.m_prefixMap.end())
			{
				qname.setNamespaceURI((*iPrefix).second);
			}
			else
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sXML, EXML_UNDECLNS,
					"the namespace prefix '{0}' has not been declared"),
					prefix);

				errorDetected(Fatal, errMsg, EXML_UNDECLNS);
			}
		}
	}
}

//==============================================================================
// ParserImpl::isStandaloneDocument
//
// Returns the declared standalone= value from the XML declaration of the
// current document.
//==============================================================================
bool ParserImpl::isStandaloneDocument() const
{
	return m_bStandaloneDoc;
}

//==============================================================================
// ParserImpl::getDocumentError
//
// Return the most significant error that has occurred whilst parsing the
// current document.
//==============================================================================
Parser::ErrorLevel ParserImpl::getDocumentError() const
{
	return m_worstErrorFound;
}

//==============================================================================
// ParserImpl::parseTextDecl
//
// [77] TextDecl ::= '<?xml' VersionInfo? EncodingDecl S? '?.'
//
// Called when an extenral entity is being prepared for processing, to determine
// the encoding declaration.  This routine does, of course, check the
// well-formedness of the TextDecl also.
//
// Returns the number of characters that make up the TextDecl.
//==============================================================================
size_t ParserImpl::parseTextDecl(ScannerPosition& position, String& version,
                                 String& encoding)
{
	size_t declSize = 0;

	bool bDocumentEntity = (position.getEntity().getType() == EntityType::Document);

	ValueRestorer<ScannerPosition> elementRestorer(m_scannerPos, position);

	if(Scanner::SkipNextStringConstant(m_scannerPos, sXMLDecl) &&
	   CharTypeFacet::IsWhiteSpace(Scanner::PeekNextCharacter(m_scannerPos)))
	{
		//
		// The next thing we want is the list of attributes associated with
		// the xml declaration.
		//
		// Note: parseAttributeList() eats white space
		//
		AttributeSetImpl attrs;
		if(parseAttributeList(NULL, attrs, '?'))
		{
			int nVersionIndex, nEncodingIndex;
			nVersionIndex = nEncodingIndex = -1;

			for(size_t i=0; i<attrs.size(); ++i)
			{
				const Attribute& attr = *attrs.getAttribute(i).get();
				const String& name = attr.getName().getRawName();

				if(name == sVersion)
				{
					nVersionIndex = attr.getIndex();
					version = attr.getValue();
				}
				else if(name == sEncoding)
				{
					nEncodingIndex = attr.getIndex();
					encoding = attr.getValue();
				}
				else if(m_scannerPos.getEntity().getType() != EntityType::Document)
				{
					const String& errMsg = MessageFormatter::Format(
						System::GetSysMessage(sXML, EXML_TEXTDECLATTR,
						"unknown attribute '{0}' within text declaration"),
						attr.getName().getRawName());

					errorDetected(Fatal, errMsg, EXML_TEXTDECLATTR);
				}
			}

			//
			// Check that the required attributes are declared
			// and in the required order
			// (no need to do this for the document as these will be checked later)
			if(!bDocumentEntity)
			{
				if(nEncodingIndex == -1)
				{
					const String& errMsg = System::GetSysMessage(sXML, EXML_TEXTDECLENC,
						"missing 'encoding' from text declaration");

					errorDetected(Fatal, errMsg, EXML_TEXTDECLENC);
				}
				else if(nEncodingIndex < nVersionIndex)
				{
					const String& errMsg = System::GetSysMessage(sXML, EXML_TEXTDECLATTRORDER,
						"attribute order invalid within text declaration");

					errorDetected(Fatal, errMsg, EXML_TEXTDECLATTRORDER);
				}
			}
		}

		//
		// We have parsed the entire text declaration up to the point of the
		// terminator "?>".
		//
		if(!Scanner::SkipNextStringConstant(m_scannerPos, sPIEnd))
		{
			// (no need to report error for the document as this will be checked later)
			if(!bDocumentEntity)
			{
				// report the error
				reportDeclTermError(QC_T("text"), sPIEnd);

				// sadly we cannot even try to recover in the case of external entities
			}
		}

		declSize = m_scannerPos.getOffset();
	}

	return declSize;
}

//==============================================================================
// ParserImpl::validatePENesting
//
// Simple function to test that the entity being parsed hasn't changed.
// Note: this is a Validity constraint, so is only invoked when performing
// validation.
//==============================================================================
void ParserImpl::validatePENesting(const Entity& startEntity, const String& where)
{
	QC_DBG_ASSERT(m_features.m_bDoValidityChecks);

	if(m_scannerPos.getEntity() != startEntity)
	{
		const String& errMsg = MessageFormatter::Format(
			System::GetSysMessage(sXML, EXML_ENTITYNESTING,
			"{0} must start and end within the same entity"),
			where);

		errorDetected(Error, errMsg, EXML_ENTITYNESTING);
	}
}

//==============================================================================
// ParserImpl::getCurrentPosition
//
//==============================================================================
const Position& ParserImpl::getCurrentPosition() const
{
	//
	// If the scanner position is valid, then we return that, otherwise
	// we return a reference to our private Position implementation which
	// gives the system and public ids but no position information
	//
	if(m_scannerPos.isValid())
	{
		return m_scannerPos;
	}
	else
	{
		return *this;
	}
}

//==============================================================================
// ParserImpl::isNotationDeclared
//
//==============================================================================
bool ParserImpl::isNotationDeclared(const String& notation) const
{
	NotationSet::const_iterator i = m_notationSet.find(notation);
	return (i != m_notationSet.end());
}

//==============================================================================
// ParserImpl::getCurrentBaseURI
//
//==============================================================================
String ParserImpl::getCurrentBaseURI() const
{
	if(m_scannerPos.isValid())
	{
		return m_scannerPos.getEntity().getResolvedSystemId();
	}
	else
	{
		return m_docSystemId;
	}
}

//==============================================================================
// ParserImpl::isFeatureEnabled
//
//==============================================================================
bool ParserImpl::isFeatureEnabled(int featureID) const
{
	return m_features.isFeatureEnabled(featureID);
}

//==============================================================================
// ParserImpl::isFeatureSupported
//
//==============================================================================
bool ParserImpl::isFeatureSupported(int featureID) const
{
	return m_features.isFeatureSupported(featureID);
}

//==============================================================================
// ParserImpl::enableFeature
//
//==============================================================================
void ParserImpl::enableFeature(int featureID, bool bEnable)
{
	m_features.enableFeature(featureID, bEnable, m_bParseInProgress);
}

//==============================================================================
// ParserImpl::setContentEventHandler
//
// Set the ContentEventHandler.
//
// Note: ownership of the pointer remains with the caller,
//       the EventHandler class is not reference-counted
//
//==============================================================================
void ParserImpl::setContentEventHandler(ContentEventHandler* npHandler)
{
	m_npContentEventHandler = npHandler;
}

//==============================================================================
// ParserImpl::setDTDEventHandler
//
// Set the DTDEventHandler.
//
// Note: ownership of the pointer remains with the caller,
//       the EventHandler class is not reference-counted
//
//==============================================================================
void ParserImpl::setDTDEventHandler(DTDEventHandler* npHandler)
{
	m_npDTDEventHandler = npHandler;
}

//==============================================================================
// ParserImpl::setErrorEventHandler
//
// Set the ErrorEventHandler.
//
// Note: ownership of the pointer remains with the caller,
//       the EventHandler class is not reference-counted
//
//==============================================================================
void ParserImpl::setErrorEventHandler(ErrorEventHandler* npHandler)
{
	m_npErrorEventHandler = npHandler;
}

//==============================================================================
// ParserImpl::setEntityEventHandler
//
// Set the EntityEventHandler.
//
// Note: ownership of the pointer remains with the caller,
//       the EventHandler class is not reference-counted
//
//==============================================================================
void ParserImpl::setEntityEventHandler(EntityEventHandler* npHandler)
{
	m_npEntityEventHandler = npHandler;
}

//==============================================================================
// ParserImpl::setEntityResolutionHandler
//
// Set the EntityResolutionHandler.
//
// Note: ownership of the pointer remains with the caller,
//       the EventHandler class is not reference-counted
//
//==============================================================================
void ParserImpl::setEntityResolutionHandler(EntityResolutionHandler* npHandler)
{
	m_npEntityResolutionHandler = npHandler;
}

//==============================================================================
// ParserImpl::getContentEventHandler
//
//==============================================================================
ContentEventHandler* ParserImpl::getContentEventHandler() const
{
	return m_npContentEventHandler;
}

//==============================================================================
// ParserImpl::getDTDEventHandler
//
//==============================================================================
DTDEventHandler* ParserImpl::getDTDEventHandler() const
{
	return m_npDTDEventHandler;
}

//==============================================================================
// ParserImpl::getEntityEventHandler
//
//==============================================================================
EntityEventHandler* ParserImpl::getEntityEventHandler() const
{
	return m_npEntityEventHandler;
}

//==============================================================================
// ParserImpl::getEntityResolutionHandler
//
//==============================================================================
EntityResolutionHandler* ParserImpl::getEntityResolutionHandler() const
{
	return m_npEntityResolutionHandler;
}

//==============================================================================
// ParserImpl::getErrorEventHandler
//
//==============================================================================
ErrorEventHandler* ParserImpl::getErrorEventHandler() const
{
	return m_npErrorEventHandler;
}

//==============================================================================
// ParserImpl::setDTDOverride
//
// Provides the facility to inject an external DTD subset into a document
// or replace the one already referenced in a DOCTYPE declaration.
//
// This override remains in place until this function is called again with
// a null parameter.
//==============================================================================
void ParserImpl::setDTDOverride(XMLInputSource* pDTDInputSource)
{
	m_rpDTDOverride = pDTDInputSource;
}

//==============================================================================
// ParserImpl::getDTDOverride
//
//==============================================================================
AutoPtr<XMLInputSource> ParserImpl::getDTDOverride() const
{
	return m_rpDTDOverride;
}

//==============================================================================
// ParserImpl::reportMissingWhiteSpaceAfter
//
//==============================================================================
void ParserImpl::reportMissingWhiteSpaceAfter(const String& after, const String& where)
{
	const String& errMsg = MessageFormatter::Format(
		System::GetSysMessage(sXML, EXML_MISSINGWHITESPACE5,
		"white-space expected after {0} in {1}"),
		after, where);

	errorDetected(Fatal, errMsg, EXML_MISSINGWHITESPACE5);
}

//==============================================================================
// ParserImpl::reportMissingWhiteSpaceBefore
//
//==============================================================================
void ParserImpl::reportMissingWhiteSpaceBefore(const String& before)
{
	const String& errMsg = MessageFormatter::Format(
		System::GetSysMessage(sXML, EXML_MISSINGWHITESPACE1,
		"white-space expected before {0}"),
		before);

	errorDetected(Fatal, errMsg, EXML_MISSINGWHITESPACE1);
}

//==============================================================================
// ParserImpl::reportMissingWhiteSpaceBefore
//
//==============================================================================
void ParserImpl::reportMissingWhiteSpaceBefore(const String& before, const String& where)
{
	const String& errMsg = MessageFormatter::Format(
		System::GetSysMessage(sXML, EXML_MISSINGWHITESPACE2,
		"white-space expected before {0} in {1}"),
		before, where);

	errorDetected(Fatal, errMsg, EXML_MISSINGWHITESPACE2);
}

//==============================================================================
// ParserImpl::reportDeclTermError
//
// Helper function to report an error when declaration productions do not end
// with the expected token.
//==============================================================================
void ParserImpl::reportDeclTermError(const String& what, const String& expected)
{
	const String& errMsg = MessageFormatter::Format(
		System::GetSysMessage(sXML, EXML_DECLNOTTERM,
		"{0} declaration not terminated correctly. Expected '{1}'"),
		what, expected);

	errorDetected(Fatal, errMsg, EXML_DECLNOTTERM);
}

//==============================================================================
// ParserImpl::parsingInternalDTDSubset
//
// When are we parsing the Internal DTD Subset?  We leave the subset the moment
// we enter into an external entity, and we re-enter the internal subset
// afterwards.
// The Entity-parent relationship refers to declaration ownership - which
// is subtly different to the parsing relationship - which is about the order
// of dereferencing as opposed to the order of declaration.
//
// For this reason we can only determine if we are within the internal subset
// by looking at the ScannerPosition stack.
// (this may be changed to use ScannerPosition flags)
//==============================================================================
bool ParserImpl::parsingInternalDTDSubset() const
{
	if(m_bParsingDTD)
	{
		const ExternalEntity* pEntity = Scanner::GetCurrentExternalEntity(m_scannerPos);
		QC_DBG_ASSERT(pEntity!=0);
		return (pEntity->getType() == EntityType::Document);
	}
	return false;
}

size_t ParserImpl::getColumnNumber() const
{
	return 0;
}

size_t ParserImpl::getLineNumber() const
{
	return 0;
}

size_t ParserImpl::getOffset() const
{
	return 0;
}

String ParserImpl::getPublicId() const
{
	return m_docPublicId;
}

String ParserImpl::getSystemId() const
{
	return m_docSystemId;
}

String ParserImpl::getResolvedSystemId() const
{
	return m_docSystemId;
}

bool ParserImpl::isParseInProgress() const
{
	return m_bParseInProgress;
}

QC_XML_NAMESPACE_END

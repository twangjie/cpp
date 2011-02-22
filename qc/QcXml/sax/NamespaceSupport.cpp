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
// Class NamespaceSupport
/**
    @class qc::sax::NamespaceSupport

    @brief Tracks Namespace declarations currently in effect.
	
	<p>This class encapsulates the logic of Namespace processing:
	it tracks the declarations currently in force for each context
	and automatically processes qualified XML 1.0 names into their
	Namespace parts; it can also be used in reverse for generating
	XML 1.0 from Namespaces.</p>
	
	<p>Namespace support objects are reusable, but the reset method
	must be invoked between each session.</p>
	
	<p>Here is a simple session:</p>
	
    @code
    String uri, localName;
    AutoPtr<NamespaceSupport> rpSupport = new NamespaceSupport;
    
    rpSupport->pushContext();
    rpSupport->declarePrefix("", "http://www.w3.org/1999/xhtml");
    rpSupport->declarePrefix("dc", "http://www.purl.org/dc#");
    
    rpSupport->processName("p", false, uri, localName);
    Console::cout() << QC_T("Namespace URI: ") << uri << endl;
    Console::cout() << QC_T("Local name: ") << localName << endl;

    rpSupport->processName("dc:title", false, uri, localName);
    Console::cout() << QC_T("Namespace URI: ") << uri << endl;
    Console::cout() << QC_T("Local name: ") << localName << endl;

    rpSupport->popContext();
    @endcode
*/
//==============================================================================

#include "NamespaceSupport.h"

QC_SAX_NAMESPACE_BEGIN

//==============================================================================
/**
	The XML Namespace as a constant.
	
	<p>This is the Namespace URI that is automatically mapped
	to the @c xml prefix.</p>
*/
//==============================================================================
const String NamespaceSupport::XMLNS =  QC_T("http://www.w3.org/XML/1998/namespace");

const String sXML = QC_T("xml");
const String sNSPrefix = QC_T("xmlns");

//==============================================================================
// NamespaceSupport::NamespaceSupport
//
/**
   Default constructor.
*/
//==============================================================================
NamespaceSupport::NamespaceSupport()
{
	init();
}

//==============================================================================
// NamespaceSupport::declarePrefix
//
/**
	Declare a Namespace prefix.
	
	<p>This method declares a prefix in the current Namespace
	context; the prefix will remain in force until this context
	is popped, unless it is shadowed in a descendant context.</p>
	
	<p>To declare a default Namespace, use the empty string.  The
	prefix must not be @c xml or @c xmlns. </p>
	
	<p>Note that there is an asymmetry in this class: while {@link
	#getPrefix getPrefix} will not return the empty (default) prefix,
	even if you have declared one; to check for a default prefix,
	you have to look it up explicitly using {@link #getURI getURI}.
	This asymmetry exists to make it easier to look up prefixes
	for attribute names, where the default prefix is not allowed.</p>
	
	@param prefix The prefix to declare, or the empty string.
	@param uri The Namespace URI to associate with the prefix.
	@return true if the prefix was legal, false otherwise
	@sa processName()
	@sa getURI()
	@sa getPrefix()
*/
//==============================================================================
bool NamespaceSupport::declarePrefix(const String& prefix, const String& uri)
{
	if(prefix==sXML || prefix==sNSPrefix)
	{
		return false;
	}

	NamespaceContext& currentContext = m_namespaceContextStack.top();
	if(prefix.empty())
	{
		// Default namespace
		currentContext.m_defaultURI = uri;
	}
	else
	{
		currentContext.m_prefixMap[prefix] = uri;
	}
	currentContext.m_deltaPrefixList.push_back(prefix);

	return true;
}

//==============================================================================
// NamespaceSupport::getDeclaredPrefixes
//
/**
	Return a list of all prefixes declared in this context.
	
	<p>The empty (default) prefix will be included in this 
	list; note that this behaviour differs from that of
	getPrefix() and getPrefixes().</p>
	
	@return A list of all prefixes declared in this context.
	@sa getPrefixes()
	@sa getURI()
*/
//==============================================================================
NamespaceSupport::PrefixList NamespaceSupport::getDeclaredPrefixes() const
{
	return m_namespaceContextStack.top().m_deltaPrefixList;
}

//==============================================================================
// NamespaceSupport::getPrefix
//
/**
	Return one of the prefixes mapped to a Namespace URI.
	
	<p>If more than one prefix is currently mapped to the same
	URI, this method will make an arbitrary selection; if you
	want all of the prefixes, use the getPrefixes() method instead.</p>
	
	<p><strong>Note:</strong> this will never return the empty (default) prefix;
	to check for a default prefix, use the getURI() method with an argument
	of the empty string.</p>
	
	@param uri The Namespace URI.
	@return One of the prefixes currently mapped to the URI supplied,
	        or the empty string if none is mapped or if the URI is assigned to
	        the default Namespace.
	@sa getPrefixes(const String& uri)
	@sa getURI()
*/
//==============================================================================
String NamespaceSupport::getPrefix(const String& uri) const
{
	const NamespaceContext::PrefixMap& currentPrefixMap = m_namespaceContextStack.top().m_prefixMap;
	for(NamespaceContext::PrefixMap::const_iterator iter = currentPrefixMap.begin(); iter!=currentPrefixMap.end(); ++iter)
	{
		// Our "arbitrary" selection is the first one we find
		if((*iter).second==uri)
		{
			return (*iter).first;
		}
	}

	// Not found
	return String();
}

//==============================================================================
// NamespaceSupport::getPrefixes()
//
/**
	Return a list of all prefixes currently declared.
	
	<p><strong>Note:</strong> if there is a default prefix, it will not be
	returned in this list; check for the default prefix
	using the getURI() method with an argument of the empty string.</p>
	
	@return A list of all prefixes currently declared except for the empty
	        (default) prefix.
	@sa getDeclaredPrefixes()
	@sa getURI()
*/
//==============================================================================
NamespaceSupport::PrefixList NamespaceSupport::getPrefixes() const
{
	PrefixList prefixes;

	const NamespaceContext::PrefixMap& currentPrefixMap = m_namespaceContextStack.top().m_prefixMap;
	for(NamespaceContext::PrefixMap::const_iterator iter = currentPrefixMap.begin(); iter!=currentPrefixMap.end(); ++iter)
	{
		prefixes.push_back((*iter).first);
	}

	return prefixes;
}

//==============================================================================
// NamespaceSupport::getPrefixes
//
/**
	Return a list of all prefixes currently declared for a URI.
	
	<p>This method returns prefixes mapped to a specific Namespace
	URI.  The @c xml prefix will be included.  If you want only one
	prefix that's mapped to the Namespace URI, and you don't care 
	which one you get, use the getPrefix() method instead.</p>
	
	<p><strong>Note:</strong> the empty (default) prefix is <em>never</em> included
	in this list; to check for the presence of a default
	Namespace, use the getURI() method with an argument of the empty string.</p>
	
	@param uri The Namespace URI.
    @return A list of all prefixes currently declared for the passed @c uri
	@sa getPrefix()
	@sa getDeclaredPrefixes()
	@sa getURI()
*/
//==============================================================================
NamespaceSupport::PrefixList NamespaceSupport::getPrefixes(const String& uri) const
{
	NamespaceSupport::PrefixList prefixes;

	const NamespaceContext::PrefixMap& currentPrefixMap = m_namespaceContextStack.top().m_prefixMap;
	for(NamespaceContext::PrefixMap::const_iterator iter = currentPrefixMap.begin(); iter!=currentPrefixMap.end(); ++iter)
	{
		if((*iter).second==uri)
		{
			prefixes.push_back((*iter).first);
		}
	}

	return prefixes;
}

//==============================================================================
// NamespaceSupport::getURI
//
/**
	Look up the URI associated with a prefix in this context.
	
	@param prefix The prefix to look up.
	@return The associated Namespace URI, or the empty string if none is
	        declared.	
*/
//==============================================================================
String NamespaceSupport::getURI(const String& prefix) const
{
	const NamespaceContext& currentContext = m_namespaceContextStack.top();

	if(prefix.empty())
	{
		// Default URI
		return currentContext.m_defaultURI;
	}

	const NamespaceContext::PrefixMap& currentPrefixMap = currentContext.m_prefixMap;
	const NamespaceContext::PrefixMap::const_iterator& iter = currentPrefixMap.find(prefix);

	if(iter!=currentPrefixMap.end())
	{
		// Found mapping
		return (*iter).second;
	}
	else
	{
		// No current prefix mapping
		return String();
	}
}

//==============================================================================
// NamespaceSupport::popContext
//
/**
	Revert to the previous Namespace context.
	
	<p>Normally, you should pop the context at the end of each
	XML element.  After popping the context, all Namespace prefix
	mappings that were previously in force are restored.</p>
	
	<p>You must not attempt to declare additional Namespace
	prefixes after popping a context, unless you push another
	context first.</p>
	
	@sa pushContext()
*/
//==============================================================================
void NamespaceSupport::popContext()
{
	m_namespaceContextStack.pop();
}

//==============================================================================
// NamespaceSupport::pushContext
//
/**
	Start a new Namespace context.
	
	<p>Normally, you should push a new context at the beginning
	of each XML element: the new context will automatically inherit
	the declarations of its parent context, but it will also keep
	track of which declarations were made within this context.</p>
	
	<p>The Namespace support object always starts with a base context
	already in force: in this context, only the @c xml prefix is
	declared.</p>
	
	@sa popContext()
*/
//==============================================================================
void NamespaceSupport::pushContext()
{
	m_namespaceContextStack.push(m_namespaceContextStack.top());
	// Clear the list of prefix deltas for the new context.
	m_namespaceContextStack.top().m_deltaPrefixList.clear();
}

//==============================================================================
// NamespaceSupport::reset
//
/**
	Reset this Namespace support object for reuse.
	
	<p>It is necessary to invoke this method before reusing the
	Namespace support object for a new session.</p>
*/
//==============================================================================
void NamespaceSupport::reset()
{
	// Fastest way to empty the stack
	m_namespaceContextStack = NamespaceContextStack();

	init();
}

//==============================================================================
// NamespaceSupport::init
//
// Private initialization.
//==============================================================================
void NamespaceSupport::init()
{
	NamespaceContext initialContext;

	initialContext.m_prefixMap[sXML] = XMLNS;

	// Note: We do not add the "xml" prefix mapping to the list of prefix deltas.
	// This means it not regarded as having been declared in the initial context.

	m_namespaceContextStack.push(initialContext);
}

//==============================================================================
// NamespaceSupport::processName
//
/**
	Process a raw XML 1.0 name.
	
	<p>This method processes a raw XML 1.0 name in the current
	context by removing the prefix and looking it up among the
	prefixes currently declared.</p>
	
	<p>The two return parameters: @c retUri and @c retLocal
	are filled in with the associated Namespace URI and local part
	respectively.  A return value of @c false is returned if the
	@c qName contains a Namespace prefix which has not been declared.</p>
	
	<p>Note that attribute names are processed differently to
	element names: an unprefixed element name will receive the
	default Namespace (if any), while an unprefixed element name
	will not.</p>
	
	@param qName The raw XML 1.0 name to be processed.
	@param bAttribute A flag indicating whether this is an
	       attribute name (true) or an element name (false).
    @param retUri A return parameter that will contain the Namespace URI associated
           with @c qName.
    @param retLocal A return parameter that will contain the local part of the
	       Namespace-qualified @c qName.
    @return true if the supplied raw XML name contained either a valid
	       prefix or no prefix at all; false otherwise.
	@sa declarePrefix()
*/
//==============================================================================
bool NamespaceSupport::processName(const String& qName, bool bAttribute,
                                   String& retUri, String& retLocal) const
{
	const size_t delimPos = qName.find(':');
	if(delimPos == String::npos)
	{
		//
		// No prefix present in the QName...
		//
		if(bAttribute)
		{
			retUri.erase();
		}
		else
		{
			const NamespaceContext& currentContext = m_namespaceContextStack.top();
			retUri = currentContext.m_defaultURI;
		}

		retLocal = qName;
		return true;
	}
	else
	{
		retLocal = qName.substr(delimPos+1);
		const String& prefix = qName.substr(delimPos);
		retUri = getURI(prefix);
		return !retUri.empty();
	}
}

QC_SAX_NAMESPACE_END

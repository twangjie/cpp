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
// Class: ElementContentSpec
// 
// Overview
// --------
// Class used to record the tree structure representing the parse tree of an
// element's content model as dictated by a DTD.
// 
// Each node is either a 'Terminal' or a list node.  Terminal nodes have
// no children but they do contain an element name representing the name
// of the sub-element that can appear at that position.  Each node has a 
// multiplicity (Optional(?), OneOrMore(+) or ZeroOrMore(*)).  When a list
// node has a multiplicity it relates to the entire list.
//
// List nodes are either "sequence" lists or "choice" lists.  For a choice
// list only one child is required in order to satisfy the list.  For 
// a sequence list each child must appear in turn (depending of course on
// the multiplicity/optionality of the child node).
//
// Example:
// The specification:
// <!ELEMENT rob (el1,(el2|el3)*,el4,el5?)+
// results in the following tree:-
//
//              *-------*
//              1 Seq  +|
//              *-------*
//                  |
//     --------------------------------------
//     |            |           |           |
//  *-------*   *-------*   *-------*   *-------*
//  2 el1  +|   3choice*|   6 el4   |   7 el5  ?|
//  *-------*   *-------*   *-------*   *-------*
//                  |
//            -------------
//            |           |     
//        *-------*   *-------* 
//        4el2    |   5 el3   | 
//        *-------*   *-------* 
//
// Once the tree has been created (during DTD parsing), it can be analysed to see
// if the content model is legal: the XML 1.0 rec states that the model must be
// determinsitic.  There are a few ways to determine this, but the easiest is to
// check each node in the tree and calculate the set of possible next nodes that can
// be reached from that node.  If a single element can be reached more than once then
// we have a non-determinitic model.  It transpires that this is set of reachable 
// elements is useful anyway because when validating the document we can use this
// set to check the document contents.
//
// In the above example, the set of reachable elements from each node is:-
// 1: el1
// 2: el1, el2, el3, el4
// 3: el2, el3, el4
// 4: el2, el3, el4
// 5: el2, el3, el4
// 6: el5, el1, </>
// 7: el1, </>
//
// Example 2:
// The specification:
// <!ELEMENT Ellie (Rupert)+
// results in the following tree:-
//
//              *-------*
//              1 Seq  +|
//              *-------*
//                  |
//              *-------*
//              2Rupert |
//              *-------*
//
// In this example, the set of reachable elements from each node is:-
// 1: Rupert
// 2: Rupert, </>
//
//=============================================================================

#include "ElementContentSpec.h"
#include "ParserImpl.h"
#include "XMLMessages.h"

#include "QcCore/base/debug.h"
#include "QcCore/base/System.h"
#include "QcCore/util/MessageFormatter.h"

#include <list> 


QC_XML_NAMESPACE_BEGIN

using namespace util;

const String sXML = QC_T("xml");

// creates a top-level parent
ElementContentSpec::ElementContentSpec(const String& elementName) :
	m_bEndElementOK(false),
	m_bNextSetComplete(false),
	m_elementName(elementName),
	m_eListType(Choice),
	m_eMultiplicity(One),
	m_pParent(0),
	m_seqNo(0),
	m_bValidated(false)
{
}


// creates a terminal
ElementContentSpec::ElementContentSpec(ElementContentSpec* pParent, int seqNo, const String& elementName) :
	m_bEndElementOK(false),
	m_bNextSetComplete(false),
	m_elementName(elementName),
	m_eListType(Terminal),
	m_eMultiplicity(One),
	m_pParent(pParent),
	m_seqNo(seqNo),
	m_bValidated(false)
{
}

// creates a non-terminal
ElementContentSpec::ElementContentSpec(ElementContentSpec* pParent, int seqNo) :
	m_bEndElementOK(false),
	m_bNextSetComplete(false),
	m_eListType(Choice),
	m_eMultiplicity(One),
	m_pParent(pParent),
	m_seqNo(seqNo),
	m_bValidated(false)
{
}

ElementContentSpec::~ElementContentSpec()
{
	Children::iterator iter;
	for(iter = m_children.begin(); iter!= m_children.end(); ++iter)
	{
		delete (*iter);
	}
}

const String& ElementContentSpec::getName() const
{
	return m_elementName;
}
	
ElementContentSpec& ElementContentSpec::addTerminal(const String& elementName)
{
	QC_DBG_ASSERT(!(m_eListType == Choice && containsTerminal(elementName)));

	ElementContentSpec* pRet = new ElementContentSpec(this, m_children.size(), elementName);
	m_children.push_back(pRet);
	return *pRet;
}

//==============================================================================
// ElementContentSpec::containsTerminal
//
// Determine if the passed elementName is contains as a Terminal
// child element within out collection.
//==============================================================================
bool ElementContentSpec::containsTerminal(const String& elementName) const
{
	Children::const_iterator iter;
	for(iter = m_children.begin(); iter!= m_children.end(); ++iter)
	{
		ElementContentSpec* pChild = (*iter);
		if(pChild->getElementName() == elementName)
		{
			return true;
		}
	}
	return false;
}

ElementContentSpec& ElementContentSpec::addChild()
{
	ElementContentSpec* pRet = new ElementContentSpec(this, m_children.size());
	m_children.push_back(pRet);
	return *pRet;
}

ElementContentSpec::Multiplicity ElementContentSpec::getMultiplicity() const
{
	return m_eMultiplicity;
}

ElementContentSpec::ListType ElementContentSpec::getListType() const
{
	return m_eListType;
}

const String& ElementContentSpec::getElementName() const
{
	return m_elementName;
}

bool ElementContentSpec::isTerminal() const
{
	if(m_eListType == Terminal)
	{
		QC_DBG_ASSERT(m_elementName.length() > 0);
		return true;
	}
	else
		return false;
}

bool ElementContentSpec::isEndElementPermitted() const
{
	return m_bEndElementOK;
}

bool ElementContentSpec::isEmpty() const
{
	return (!isTerminal() && m_children.empty());
}

bool ElementContentSpec::doesAllowMultiple() const
{
	return (m_eMultiplicity == OneOrMore || m_eMultiplicity == ZeroOrMore);
}

//==============================================================================
// ElementContentSpec::doesAllowNone
//
// Discover whether or not this node is completely optional
//==============================================================================
bool ElementContentSpec::doesAllowNone() const
{
	bool bRet = false;
	
	if(m_eMultiplicity == ZeroOrMore || m_eMultiplicity == Optional)
	{
		bRet = true;
	}
	else
	{
		if(!isTerminal())
		{
			bRet = true;
			for(size_t i=0; i<m_children.size(); i++)
			{
				ElementContentSpec* pChild = m_children[i];
				bRet = bRet && pChild->doesAllowNone();
				if(bRet && m_eListType == Choice)
					break;
			}
		}
	}
	return bRet;
}

//=============================================================================
// ElementContentSpec::isRequired
// 
// Determines whether a node in the content spec tree is "required" or
// "optional".  It is required if the multiplicity of itself and its
// ancestors demand at least one instance.
//
//=============================================================================
bool ElementContentSpec::isRequired() const
{
	bool bRet = !doesAllowNone();
	if(bRet && m_pParent)
		bRet = m_pParent->isRequired();
	return bRet;
}

void ElementContentSpec::setMultiplicity(Multiplicity eMultiplicity)
{
	m_eMultiplicity = eMultiplicity;
}

void ElementContentSpec::setListType(ListType eListType)
{
	m_eListType = eListType;
}

//=============================================================================
// ElementContentSpec::validate
// 
// This function tests for non-deterministic content models.
//
// Note: does not throw exceptions so that all possible errors can be
// detected.
//=============================================================================
void ElementContentSpec::validate(ParserImpl& parser) const
{
	const_cast<ElementContentSpec*>(this)->generateNextSet(parser);

	// and don't forget the kids
	for(size_t i=0; i<m_children.size(); i++)
	{
		ElementContentSpec* pChild = m_children[i];
		pChild->validate(parser);
	}
}

//=============================================================================
// ElementContentSpec::validateNextElement
// 
// This function tests to see if the next element in the xml input
// stream is valid according to the content model.
//
//=============================================================================
const ElementContentSpec* ElementContentSpec::validateNextElement(const String& name) const
{
	QC_DBG_ASSERT(m_bNextSetComplete);
	const ElementContentSpec* pRet = NULL;

	Map::const_iterator iter = m_nextSet.find(name);

	if(iter!=m_nextSet.end())
		pRet = (*iter).second;

	return pRet;
}

//=============================================================================
// ElementContentSpec::generateNextSet
// 
// This [important] function traverses all the possible arcs from this node
// and creates a set of permissible next Elements.
//
// This can either be called actively or lazilly, depending on whether
// the user wants to validate the DTD for ambiguous content models before
// or during the parsing of the document.
//
// Note: When dealing with a node, we are inherently at the start of the node,
// there is no positional information.  However, when control is passed up to
// the parent to get next elements from there, the position of the child is
// taken into consideration.
//=============================================================================
void ElementContentSpec::generateNextSet(ParserImpl& parser)
{
	if(m_bNextSetComplete)
		return;

	if(isTerminal() && doesAllowMultiple())
	{
		AddElementToSet(this, m_nextSet, parser);
	}

	if(!m_children.empty())
	{
		getNextElementsFrom(-1, m_nextSet, m_bEndElementOK, parser);
	}
	else if(m_pParent)
	{
		m_pParent->getNextElementsFrom(m_seqNo, m_nextSet, m_bEndElementOK, parser);
	}
	else
	{
		// empty root!  Surely this should not happen!
		m_bEndElementOK = true;
	}

	m_bNextSetComplete = true;
}

//=============================================================================
// ElementContentSpec::getFirstElements
// 
// Recursive function that navigates down the content model tree, adding
// elements to the passed set as it goes.
//
// The outcome is a set of elements that represents the set of first
// elements that can occur within this branch of the content model.
//=============================================================================
size_t ElementContentSpec::getFirstElements(Map& theSet, ParserImpl& parser, size_t startChild) const
{
	size_t lastChild = 0;
	if(isTerminal())
	{
		AddElementToSet(this, theSet, parser);
	}
	else
	{
		for(size_t i=startChild; i<m_children.size(); i++)
		{
			lastChild = i;
			const ElementContentSpec* pChild = m_children[i];
			pChild->getFirstElements(theSet, parser, 0);
			if(m_eListType == Seq && !pChild->doesAllowNone())
			{
				break;
			}
		}
	}
	return lastChild;
}

//=============================================================================
// ElementContentSpec::getNextElementsFrom
// 
// Helper function to help navigate the specification tree.  This function
// passes control up to the parent node and gets next elements from there.
//
//=============================================================================
void ElementContentSpec::getNextElementsFrom(int seqNo,
											 Map& theSet,
											 bool& bEndElementOK,
											 ParserImpl& parser) const
{
	QC_DBG_ASSERT(seqNo < (int)m_children.size());
	QC_DBG_ASSERT(!isTerminal());   // We cannot be a Terminal because we have children!

	bool bDelegateToParent = false;
	bool bCalledFromChild = (seqNo != -1);

	if(m_eListType == Choice || seqNo == (int)(m_children.size()-1))
	{
		if(doesAllowMultiple() || !bCalledFromChild)
		{
			getFirstElements(theSet, parser, 0);
		}
		if(doesAllowNone() || bCalledFromChild)
		{
			bDelegateToParent = true;
		}
	}
	else // must be sequence then
	{
		int nextChild = seqNo+1;
		QC_DBG_ASSERT(nextChild < (int)m_children.size());
		int lastChild = getFirstElements(theSet, parser, nextChild);
		if(m_children[lastChild]->doesAllowNone())
		{
			if(doesAllowMultiple())
			{
				getFirstElements(theSet, parser, 0);
			}
			bDelegateToParent = true;
		}
	}
	if(bDelegateToParent)
	{
		if(m_pParent)
		{
			m_pParent->getNextElementsFrom(m_seqNo, theSet, bEndElementOK, parser);
		}
		else
		{
			bEndElementOK = true;
		}
	}
}

//=============================================================================
// ElementContentSpec::AddElementToSet
// 
// Static helper function to add element to the set.  If a duplicate is
// found then it means that we have a non-deterministic content model.
//=============================================================================
void ElementContentSpec::AddElementToSet(const ElementContentSpec* pSpec,
										 Map& theMap,
										 ParserImpl& parser)
{
	QC_DBG_ASSERT(pSpec->isTerminal());
	const String& name = pSpec->getName();
	Map::const_iterator iter = theMap.find(name);
	if(iter == theMap.end())
	{
		//theMap[pSpec->getName()] = pSpec;
		theMap.insert(std::make_pair(name, pSpec));
	}
	else if((*iter).second != pSpec)
	{
		const String& errMsg = MessageFormatter::Format(
			System::GetSysMessage(sXML, EXML_NOTDFA,
			"non-deterministic content model for element '{0}': more than one path leads to element '{1}'"),
			pSpec->getTopParent()->getElementName(),
			pSpec->getName());

		parser.errorDetected(Parser::Error, errMsg, EXML_NOTDFA);
	}
}

//=============================================================================
// ElementContentSpec::getDisplayableNextList
// 
// Helper function to format a list of valid next elements.  Used for error
// messages or debugging info.
//=============================================================================
String ElementContentSpec::getDisplayableNextList(const String& currentElementName) const
{
	QC_DBG_ASSERT(m_bNextSetComplete);

	String strRet;
	Map::const_iterator iter;
	size_t count=1;
	for(iter=m_nextSet.begin(); iter!=m_nextSet.end(); ++iter, count++)
	{
		if(count != 1)
		{
			if(m_bEndElementOK || count < m_nextSet.size())
				strRet += QC_T(", '");
			else
				strRet += QC_T(" or '");
		}
		else
		{
			strRet += QC_T("'");
		}

		strRet += (*iter).second->getName();
		strRet += QC_T("'");
	}
	
	if(m_bEndElementOK)
	{
		if(count > 1)
			strRet += QC_T(" or ");
		strRet += QC_T("'</");
		strRet += currentElementName;
		strRet += QC_T(">'");
	}
	return strRet;
}

//=============================================================================
// ElementContentSpec::getDisplayableRequiredList
// 
// Helper function to format a list of required next elements.  Used for error
// messages.
//=============================================================================
String ElementContentSpec::getDisplayableRequiredList() const
{
	QC_DBG_ASSERT(m_bNextSetComplete);

	std::list<String> requiredList;
	String strRet;

	Map::const_iterator iter;
	for(iter=m_nextSet.begin(); iter!=m_nextSet.end(); ++iter)
	{
		if((*iter).second->isRequired())
		{
			requiredList.push_back((*iter).second->getName());
		}
	}

	std::list<String>::const_iterator listIter;
	size_t count = 1;
	for(listIter=requiredList.begin(); listIter!=requiredList.end(); ++listIter, count++)
	{
		if(count != 1)
		{
			if(count < requiredList.size())
				strRet += QC_T(", '");
			else
				strRet += QC_T(" or '");
		}
		else
		{
			strRet += QC_T("'");
		}

		strRet += (*listIter);
		strRet += QC_T("'");
	}
	
	return strRet;
}

//==============================================================================
// ElementContentSpec::getTopParent
//
// Return the ElementContentSpec that contains the name of the element
// to which this content spec relates.
//==============================================================================
const ElementContentSpec* ElementContentSpec::getTopParent() const
{
	if(m_pParent)
		return m_pParent->getTopParent();
	else
		return this;
}

//==============================================================================
// ElementContentSpec::validated
//
// Return an indicator to show whether or not this ElementContentSpec has
// been validated yet.  For efficiency these are not validated unless they
// are used (validation involves testing the model is a DFA and is therefore
// a little expensive).
//==============================================================================
bool ElementContentSpec::validated() const
{
	return m_bValidated;
}

//==============================================================================
// ElementContentSpec::asString
//
//==============================================================================
String ElementContentSpec::asString(bool bMixedContent) const
{
	String ret;

	if(m_eListType == Choice || m_eListType == Seq)
	{
		if(m_children.size() || bMixedContent)
		{
			ret += QC_T("(");

			if(bMixedContent)
			{
				ret += QC_T("#PCDATA");
			}
			Children::const_iterator iter;
			for(iter = m_children.begin(); iter!= m_children.end(); ++iter)
			{
				if((iter != m_children.begin()) || bMixedContent)
				{
					ret += (m_eListType == Choice) ? QC_T("|") : QC_T(",");
				}
				ElementContentSpec* pChild = (*iter);
				ret += pChild->asString(false);
			}
			ret += QC_T(")");
		}
	}
	else // must be a Terminal!
	{
		QC_DBG_ASSERT(isTerminal());
		ret = m_elementName;
	}

	switch(m_eMultiplicity)
	{
	case Optional:
		ret += QC_T("?");
		break;
	case ZeroOrMore:
		ret += QC_T("*");
		break;
	case OneOrMore:
		ret += QC_T("+");
		break;
	default:
		break;
	}

	return ret;
}

QC_XML_NAMESPACE_END

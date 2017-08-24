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

#ifndef QC_XML_ElementContentSpec_h
#define QC_XML_ElementContentSpec_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

#include <vector>
#include <map>

QC_XML_NAMESPACE_BEGIN

class ParserImpl;

class QC_XML_PKG ElementContentSpec
{
public:

	typedef std::vector<ElementContentSpec*> Children;
	typedef std::map<String, const ElementContentSpec*, std::less<String> > Map;
	
	enum ListType {Seq, Choice, Terminal};
	enum Multiplicity {One, Optional, ZeroOrMore, OneOrMore};

	// creates a top-level parent
	ElementContentSpec(const String& name); 
	
	// creates a terminal
	ElementContentSpec(ElementContentSpec* pParent, int seqNo, const String& name); 

	// creates a non-terminal
	ElementContentSpec(ElementContentSpec* pParent, int seqNo);

	~ElementContentSpec();

	const String& getName() const;
	
	ElementContentSpec& addTerminal(const String& elementName);
	ElementContentSpec& addPCData();
	bool containsTerminal(const String& elementName) const;
	ElementContentSpec& addChild();

	Multiplicity getMultiplicity() const;
	ListType getListType() const;
	const String& getElementName() const;
	bool isTerminal() const;
	bool isEmpty() const;
	bool doesAllowMultiple() const;
	bool doesAllowNone() const;
	bool isRequired() const;

	void setMultiplicity(Multiplicity eMultiplicity);
	void setListType(ListType eListType);

	void validate(ParserImpl& parser) const;
	bool validated() const;
	
	const ElementContentSpec* validateNextElement(const String& name) const;
	bool isEndElementPermitted() const;
	void generateNextSet(ParserImpl& parser);

	String getDisplayableNextList(const String& currentElementName) const;
	String getDisplayableRequiredList() const;
	String asString(bool bMixedElement) const;

	const ElementContentSpec* getTopParent() const;

protected:
	
	static void AddElementToSet(const ElementContentSpec* pSpec, Map& theMap, ParserImpl& parser);
	size_t getFirstElements(Map& theMap, ParserImpl& parser, size_t startChild) const;
	void getNextElementsFrom(int seqNo, Map& theMap, bool& bEndElementOK, ParserImpl& parser) const;

private:
	bool m_bEndElementOK;
	bool m_bNextSetComplete;
	Children m_children;
	String m_elementName;
	ListType m_eListType;
	Multiplicity m_eMultiplicity;
	Map m_nextSet;
	ElementContentSpec* m_pParent;
	int m_seqNo;
	bool m_bValidated;
};

QC_XML_NAMESPACE_END

#endif //QC_XML_ElementContentSpec_h

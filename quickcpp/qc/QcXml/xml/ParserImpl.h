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

#ifndef QC_XML_ParserImpl_h
#define QC_XML_ParserImpl_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

#include "Parser.h"
#include "ParserFeatureState.h"
#include "ScannerPosition.h"
#include "Entity.h"
#include "CharTypeFacet.h"
#include "ContextString.h"
#include "ElementType.h"
#include "ExternalEntity.h"

#include "QcCore/net/URL.h"

#include <map>
#include <set>
#include <list>
#include <vector>
#include <utility>


QC_XML_NAMESPACE_BEGIN

class ElementContentSpec;
class BufferRange;
struct TokenTableEntry;

class ParserImpl : public Parser, private Position
{
	friend class ElementType;
	friend class ElementContentSpec;
	friend class ExternalEntity;
	friend class AttributeType;

public:

	ParserImpl();
	virtual ~ParserImpl();

	virtual void parseURL(const String& systemId);
	virtual void parseInputSource(XMLInputSource* pInputSource);

	virtual void parseDTD(const String& systemId);

	virtual bool isStandaloneDocument() const;
	virtual ErrorLevel getDocumentError() const;

	virtual const Position& getCurrentPosition() const;
	virtual String getCurrentBaseURI() const;

	virtual bool isFeatureEnabled(int featureID) const; 
	virtual bool isFeatureSupported(int featureID) const; 
	virtual void enableFeature(int featureID, bool bEnable);
	virtual bool isParseInProgress() const;

	virtual void setDTDOverride(XMLInputSource* pDTDInputSource);
	virtual AutoPtr<XMLInputSource> getDTDOverride() const;
	
	//
	// EventHandler setting/getting methods.
	//
	// Note: ownership of the pointer remains with the caller,
	//       the EventHandler class is not reference-counted
	//
	virtual void setContentEventHandler(ContentEventHandler* npHandler);
	virtual void setDTDEventHandler(DTDEventHandler* npHandler);
	virtual void setEntityEventHandler(EntityEventHandler* npHandler);
	virtual void setErrorEventHandler(ErrorEventHandler* npHandler);
	virtual void setEntityResolutionHandler(EntityResolutionHandler* npHandler);

	virtual ContentEventHandler* getContentEventHandler() const;
	virtual DTDEventHandler* getDTDEventHandler() const;
	virtual EntityEventHandler* getEntityEventHandler() const;
	virtual EntityResolutionHandler* getEntityResolutionHandler() const;
	virtual ErrorEventHandler* getErrorEventHandler() const;

protected:

	bool areCallbacksPermitted() const;
	void errorDetected(ErrorLevel level, const String& errMsg, long messageID);
	void errorDetected(ErrorLevel level, const String& errMsg, const ScannerPosition& position, long messageID);
	void reportMissingWhiteSpaceAfter(const String& after, const String& where);
	void reportMissingWhiteSpaceBefore(const String& before);
	void reportMissingWhiteSpaceBefore(const String& before, const String& where);
	void reportDeclTermError(const String& what, const String& expected);

	AutoPtr<Entity> getEntity(const String& name) const;

	bool addElementId(const String& id);
	void addElementIdRef(const String& id);
	bool isNotationDeclared(const String& notation) const;

	size_t parseTextDecl(ScannerPosition& position, String& version, String& encoding);
	
protected: // Parsing methods
	
	String parseAndNormalizeAttValue(bool bCData, bool bDelimited, CharType delimiter, bool& bNormalized);
	void parseAndRecurseAttributeValue(CharType delimiter, String& strRet);
	bool parseAttDef(const QName& attName, ElementType& element);
	bool parseAttDefs(ElementType& element);
	bool parseAttDefaultDecl(bool bRequireWhiteSpace, AttributeType& attr);
	bool parseAttEnumeration(AttributeType& attr);
	bool parseAttEnumList(bool bNmToken, const String& what, AttributeType& attribute);
	bool parseAttlistDecl();
	bool parseAttNotation(AttributeType& attr);
	bool parseAttributeList(const ElementType* pElementType, AttributeSet& attrSet, CharType delimiter);
	bool parseAttValue(bool bCData, bool& bNormalized, String& strRet);
	bool parseCDSect();
	bool parseCharData();
	bool parseComment();
	bool parseConditionalDecl();
	bool parseContent(bool bEntityContent=false);
	bool parseContentSpec(ElementType& element);
	bool parseContentMultiplicity(ElementType& element, ElementContentSpec* pSpec);
	bool parseDocTypeDecl();
	void parseDocument();
	bool parseDTDMarkup(bool bInternalSubset, bool bConditionalSection, bool bExpectEOF);
	bool parseElement(bool bRootElement);
	bool parseElementDecl();
	bool parseElementContentSpecList(ElementType& element, ElementContentSpec* pContentSpec);
	bool parseEntityDecl();
	bool parseEntityValue(const String& name, bool bParameterEntity);
	bool parseETag(const QName& elementName);
	void parseExtDTDSubset(const String& publicId, const String& systemId);
	void parseExtDTDSubsetOverride();
	void parseExtDTDEntity();
	bool parseExternalID(String& systemLiteral, String& pubidLiteral, bool bReqd, bool bOptionalSystemLiteral, const String& where);
	bool parseMisc();
	String parseName(const String& type, bool bReqd, bool bExpectWhitespace, bool bNmToken);
	bool parseQName(QName& name, const String& type, bool bReqd, bool bExpectWhitespace);
	bool parseNotationDecl();
	String parseRefName(bool bReqd);
	bool parsePEReference(bool bMarkupDeclaration, bool bHardEOF, bool bNameReqd);
	void parsePEDeclarations();
	bool parsePI();
	bool parseProlog();
	String parsePubidLiteral();
	String parseReference(bool bResolveEntities, bool bAttribute);
	bool parseReferenceInContent();
	bool parseSTag(QName& elementName, bool& bEmptyElement, const ElementType*& pElementType, AttributeSet& attrSet);
	String parseSystemLiteral();
	bool parseWhitespace();
	bool parseIgnorableWhitespace();
	bool parseXMLDecl();

protected: // Helper methods

	String formatForPrint(const Character& ch) const;
	
	void resetParser();
	void unexpectedChar(const Character& ch, const String& where);
	void unexpectedToken(int nextToken, const String& strToken, const String& expected);
	bool translateNamedReference(const String& name, bool bAttribute, String& strRet);
	void recoverPosition(size_t numTokens, const char* endTokens[], size_t ixEatableToken);
	int testNextTokenType(const TokenTableEntry* pTable, String& tokenFound, bool& bErrorReported);
	bool skipRequiredWhitespaceAfter(const String& after, const String& where);
	bool skipRequiredWhitespaceBefore(const String& before, const String& where);
	String getDisplayEntityName(const String& name, const EntityType& type) const;
	void elementStartValidityChecks(const QName& elementName, bool bRootElement, bool bEmptyElement, const ElementType* pElementType);

	AutoPtr<ElementType> getElement(const QName& name) const;
	AutoPtr<ElementType> addElement(const QName& name);

	void normalizeAttributeValue(String& strRet, bool& bNormalized) const;
	bool processNamespacePrefixes(QName& elementName, AttributeSet& attrs);
	void resolveNamespace(QName& qname, bool bAttribute);
	void reportCharData(const BufferRange& range) const;
	void reportStartNamespacePrefixes();
	void reportEndNamespacePrefixes();
	void validatePENesting(const Entity& startEntity, const String& where);
	void undeclaredEntity(const String& name, const EntityType& type);
	bool parsingInternalDTDSubset() const;

protected: // Validation functions
	void validateDTD();
	void validateDTDElementType(const ElementType& element);
	void postRootValidityChecks();

protected: // PE-expanding Scanning methods
	String getNextStringTokenEx(CharTypeFacet::Mask includeMask);

	Character getNextCharacterEx();
	Character peekNextCharacterEx();
	size_t skipWhiteSpaceEx();
	bool skipNextStringConstantEx(const String& str);
	bool skipNextCharConstantEx(CharType x);

private: // Position interface
	virtual size_t getColumnNumber() const;
	virtual size_t getLineNumber() const;
	virtual size_t getOffset() const;
	virtual String getPublicId() const;
	virtual String getSystemId() const;
	virtual String getResolvedSystemId() const;

private:
	ErrorLevel m_worstErrorFound;
	ErrorLevel m_worstErrorFoundAtPosition;
	const Entity* m_pLastErrorEntity;
	size_t  m_lastErrorOffset;

	typedef AutoPtr<Entity> RPEntity;
	typedef AutoPtr<ElementType> RPElementType;

	typedef std::map<String, String, std::less<String> > StdEntityMap;
	typedef std::map<String, RPEntity, std::less<String> > EntityMap;
	typedef std::set<String, std::less<String> > NotationSet;
	typedef std::map<String, RPElementType, std::less<String> > ElementMap;
	typedef std::list<String> EntityStack;
	typedef std::set<String, std::less<String> > IdSet;

	struct NamespaceFrame
	{
		typedef std::map<String, String, std::less<String> > PrefixMap;
		typedef std::list< std::pair < bool , String > > PrefixList;

		NamespaceFrame();
		NamespaceFrame(const PrefixMap& prefixMap, const String& defaultURI);

		String m_defaultURI;
		PrefixMap m_prefixMap;
		PrefixList m_deltaPrefixList;
	};

	typedef std::vector<NamespaceFrame> NamespaceFrameVector;
	NamespaceFrameVector m_namespaceFrameVector;

	IdSet m_idSet;
	IdSet m_idRefSet;
	EntityMap m_geMap;
	EntityMap m_peMap;
	NotationSet m_notationSet;
	StdEntityMap m_stdEntityMap;
	ElementMap m_elementMap;
	EntityStack m_entityResolutionStack;

	bool m_bXMLDeclSeen;
	bool m_bStandaloneDoc;
	bool m_bParseInProgress;
	bool m_bInErrorHandler;
	bool m_bParsingDTD;
	bool m_bHasDTD;
	bool m_bExternalDTDSubsetDeclared;
	bool m_bDTDContainsPEReferences;
	bool m_bParsingEntityValue;
	bool m_bParsingEntityDeclName;
	bool m_bReportPEBoundaries;

	ScannerPosition m_scannerPos;
	String m_currentElementName;
	String m_DTDName;
	String m_docPublicId;
	String m_docSystemId;

	const ElementType* m_pCurrentElementType;
	const ElementContentSpec* m_pCurrentElementContentSpec;

	AutoPtr<ExternalEntity> m_rpDTDEntity;

	ParserFeatureState m_features;
	AutoPtr<XMLInputSource> m_rpDTDOverride;

	ContentEventHandler*     m_npContentEventHandler;
	DTDEventHandler*         m_npDTDEventHandler;
	EntityEventHandler*      m_npEntityEventHandler;
	EntityResolutionHandler* m_npEntityResolutionHandler;
	ErrorEventHandler*       m_npErrorEventHandler;
};

QC_XML_NAMESPACE_END

#endif //QC_XML_ParserImpl_h

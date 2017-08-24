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

#ifndef QC_XML_ExternalEntity_h
#define QC_XML_ExternalEntity_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

#include "Entity.h"
#include "XMLInputSource.h"
#include "QcCore/net/URL.h"

QC_XML_NAMESPACE_BEGIN

using net::URL;

class ParserImpl;

class QC_XML_PKG ExternalEntity : public Entity
{
public:

	ExternalEntity(ParserImpl& parser, EntityType type, XMLInputSource* pInputSource);
	
	ExternalEntity(ParserImpl& parser, EntityType type, const Entity* pParent,
	               bool bExternallyDeclared, const String& name,
	               const String& publicId, const String& systemId,
	               const String& notation);

	virtual ScannerPosition getStartPosition();
	virtual bool isParsed() const;
	virtual bool isExternalEntity() const;
	virtual bool isInternalEntity() const;
	virtual void load();
	bool isLoaded() const;
	
	virtual AutoPtr<Reader> getReader() const;
	virtual String getResolvedSystemId() const;
	virtual String getBaseURI() const;
	virtual String getSystemId() const;
	virtual String getPublicId() const;
	const String& getNotation() const;

	virtual void validate(ParserImpl& parser) const;

protected:
	void createReader(ParserImpl& parser, const String& extEncoding, InputStream* pInputStream); 
	void init(XMLInputSource* pInputSource);
	void resolveEntity();

private:
	ParserImpl& m_parser;
	bool m_bLoaded;
	String m_notation;
	AutoPtr<XMLInputSource> m_rpInputSource;
	AutoPtr<Reader> m_rpReader;
	ScannerPosition m_startPosition;
	String m_resolvedSystemId;
	String m_systemId;
	String m_publicId;
};

QC_XML_NAMESPACE_END

#endif //QC_XML_ExternalEntity_h

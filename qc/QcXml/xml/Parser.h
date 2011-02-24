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
// Class: Parser
// 
// Overview
// --------
// Abstract interface class.  The Parser interface represents the public
// interface exposed by the QC Technology XML Toolkit for parsing
// XML documents.
//
//==============================================================================

#ifndef QC_XML_Parser_h
#define QC_XML_Parser_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h

QC_XML_NAMESPACE_BEGIN

class ContentEventHandler;
class DTDEventHandler;
class EntityResolutionHandler;
class EntityEventHandler;
class ErrorEventHandler;
class XMLInputSource;
class Position;
class ContextString;

class QC_XML_PKG Parser : public virtual QCObject
{

public:

	enum ErrorLevel {NoError=0, Warning, Error, Fatal};

	virtual void parseURL(const String& systemId) = 0;
	virtual void parseInputSource(XMLInputSource* pInputSource) = 0;
	
	virtual void parseDTD(const String& systemId) = 0;

	virtual bool isStandaloneDocument() const = 0;
	virtual ErrorLevel getDocumentError() const = 0;

	virtual const Position& getCurrentPosition() const = 0;
	virtual String getCurrentBaseURI() const = 0;

	virtual bool isFeatureEnabled(int featureID) const = 0; 
	virtual bool isFeatureSupported(int featureID) const = 0; 
	virtual void enableFeature(int featureID, bool bEnable) = 0;
	virtual bool isParseInProgress() const = 0;
	
	virtual void setDTDOverride(XMLInputSource* pDTDInputSource) = 0;
	virtual AutoPtr<XMLInputSource> getDTDOverride() const = 0;
	
	//
	// EventHandler setting/getting methods.
	//
	// Note: ownership of the pointer remains with the caller,
	//       the EventHandler class is not reference-counted
	//
	virtual void setContentEventHandler(ContentEventHandler* npHandler) = 0;
	virtual void setDTDEventHandler(DTDEventHandler* npHandler) = 0;
	virtual void setEntityEventHandler(EntityEventHandler* npHandler) = 0;
	virtual void setErrorEventHandler(ErrorEventHandler* npHandler) = 0;
	virtual void setEntityResolutionHandler(EntityResolutionHandler* npHandler) = 0;

	virtual ContentEventHandler* getContentEventHandler() const = 0;
	virtual DTDEventHandler* getDTDEventHandler() const = 0;
	virtual EntityEventHandler* getEntityEventHandler() const = 0;
	virtual EntityResolutionHandler* getEntityResolutionHandler() const = 0;
	virtual ErrorEventHandler* getErrorEventHandler() const = 0;
};

QC_XML_NAMESPACE_END

#endif //QC_XML_Parser_h

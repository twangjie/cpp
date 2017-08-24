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

#ifndef QC_SAX_XMLReader_h
#define QC_SAX_XMLReader_h

#include "defs.h"

#include "ContentHandler.h"
#include "EntityResolver.h"
#include "DTDHandler.h"
#include "ErrorHandler.h"
#include "LexicalHandler.h"
#include "DeclHandler.h"

QC_SAX_NAMESPACE_BEGIN 

class QC_SAX_PKG XMLReader : public virtual QCObject
{
public:
	virtual AutoPtr<ContentHandler> getContentHandler() const = 0;
	virtual AutoPtr<DeclHandler> getDeclHandler() const = 0;
	virtual AutoPtr<DTDHandler> getDTDHandler() const = 0;
	virtual AutoPtr<EntityResolver> getEntityResolver() const = 0;
	virtual AutoPtr<ErrorHandler> getErrorHandler() const = 0;
	virtual AutoPtr<LexicalHandler> getLexicalHandler() const = 0;

	virtual bool getFeature(const String& name) const = 0;
          
	virtual void parse(InputSource* pInputSource) = 0;
	virtual void parse(const String& systemId) = 0;
          
	virtual void setContentHandler(ContentHandler* pHandler) = 0;
	virtual void setDeclHandler(DeclHandler* pHandler) = 0;
	virtual void setDTDHandler(DTDHandler* pHandler) = 0;
	virtual void setEntityResolver(EntityResolver* pResolver) = 0;
	virtual void setErrorHandler(ErrorHandler* pHandler) = 0;
	virtual void setLexicalHandler(LexicalHandler* pHandler) = 0;
	virtual void setFeature(const String& name, bool bSet) = 0;
};

QC_SAX_NAMESPACE_END

#endif //QC_SAX_XMLReader_h

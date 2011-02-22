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

#ifndef QC_XML_ParserFeatureState_h
#define QC_XML_ParserFeatureState_h

#ifndef QC_XML_defs_h
#include "defs.h"
#endif //QC_XML_defs_h


QC_XML_NAMESPACE_BEGIN

class QC_XML_PKG ParserFeatureState
{

public:

	ParserFeatureState();

	void enableFeature(int featureID, bool bValue, bool bParsing=false);
	bool isFeatureSupported(int featureID) const;
	bool isFeatureEnabled(int featureID) const;

public:
	// Direct member access to Features
	bool m_bDoInteroperabilityChecks;
	bool m_bDoValidityChecks;
	bool m_bDoWarningChecks;
	bool m_bNamespaceSupport;
	bool m_bProcessExternalParameterEntities;
	bool m_bProcessExternalGeneralEntities;
	bool m_bReportDocumentComments;
	bool m_bReportDTDComments;
	bool m_bReportNamespaceDeclarations;
	bool m_bValidateNamespaceDeclarations;
	bool m_bRelativeNamespaceURITestEnabled;
	bool m_bReportPEBoundaries;
	bool m_bResolveParameterEntities;
	bool m_bResolveGeneralEntities;

protected:
	void makeConsistent(int featureID, bool bValue, bool bParsing);

};

QC_XML_NAMESPACE_END

#endif //QC_XML_ParserFeatureState_h

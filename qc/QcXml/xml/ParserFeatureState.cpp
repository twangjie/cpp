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

#include "ParserFeatureState.h"
#include "XMLFeatures.h"
#include "UnsupportedFeatureException.h"

QC_XML_NAMESPACE_BEGIN

struct FeatureTableEntry
{
	int id;
	bool ParserFeatureState::* pFeature;
};

FeatureTableEntry featureTable[] =
{
	{XMLFeatures::InteroperabilityTests, &ParserFeatureState::m_bDoInteroperabilityChecks},
	{XMLFeatures::Validate, &ParserFeatureState::m_bDoValidityChecks},
	{XMLFeatures::WarningTests, &ParserFeatureState::m_bDoWarningChecks},
	{XMLFeatures::NamespaceSupport, &ParserFeatureState::m_bNamespaceSupport},
	{XMLFeatures::ReadExternalParameterEntities, &ParserFeatureState::m_bProcessExternalParameterEntities},
	{XMLFeatures::ReadExternalGeneralEntities, &ParserFeatureState::m_bProcessExternalGeneralEntities},
	{XMLFeatures::ReportDocumentComments, &ParserFeatureState::m_bReportDocumentComments},
	{XMLFeatures::ReportDTDComments, &ParserFeatureState::m_bReportDTDComments},
	{XMLFeatures::ReportNamespaceDeclarations, &ParserFeatureState::m_bReportNamespaceDeclarations},
	{XMLFeatures::ValidateNamespaceDeclarations, &ParserFeatureState::m_bValidateNamespaceDeclarations},
	{XMLFeatures::RelativeNamespaceURITest, &ParserFeatureState::m_bRelativeNamespaceURITestEnabled},
	{XMLFeatures::ReportPEBoundaries, &ParserFeatureState::m_bReportPEBoundaries},
	{XMLFeatures::ResolveParameterEntities, &ParserFeatureState::m_bResolveParameterEntities},
	{XMLFeatures::ResolveGeneralEntities, &ParserFeatureState::m_bResolveGeneralEntities}
};

const size_t numFeatures = sizeof(featureTable)/sizeof(FeatureTableEntry);

ParserFeatureState::ParserFeatureState() :
	m_bDoInteroperabilityChecks(false),
	m_bDoValidityChecks(false),
	m_bDoWarningChecks(false),
	m_bNamespaceSupport(false),
	m_bProcessExternalParameterEntities(true),
	m_bProcessExternalGeneralEntities(true),
	m_bReportDocumentComments(false),
	m_bReportDTDComments(false),
	m_bReportNamespaceDeclarations(false),
	m_bValidateNamespaceDeclarations(true),
	m_bRelativeNamespaceURITestEnabled(false),
	m_bReportPEBoundaries(false),
	m_bResolveParameterEntities(true),
	m_bResolveGeneralEntities(true)
{
}

//==============================================================================
// ParserFeatureState::enableFeature
//
//==============================================================================
void ParserFeatureState::enableFeature(int featureID, bool bValue, bool bParsing)
{
	//
	// Iterate over the feature table looking for the feature.
	//
	for(size_t index=0; index<numFeatures; ++index)
	{
		if(featureID == featureTable[index].id)
		{
			if(this->*featureTable[index].pFeature != bValue)
			{
				this->*featureTable[index].pFeature = bValue;
				makeConsistent(featureID, bValue, bParsing);
			}
			return;
		}
	}

	throw UnsupportedFeatureException(featureID);
}

//==============================================================================
// ParserFeatureState::testConsistency
//
// Called to check that a consistent set of options are set.
//==============================================================================
void ParserFeatureState::makeConsistent(int featureID, bool bValue, bool /*bParsing*/)
{
	if(featureID == XMLFeatures::Validate && bValue == true)
	{
		m_bProcessExternalParameterEntities = true;
		m_bProcessExternalGeneralEntities = true;
	}
}

//==============================================================================
// ParserFeatureState::isFeatureEnabled
//
//==============================================================================
bool ParserFeatureState::isFeatureEnabled(int featureID) const
{
	//
	// Iterate over the feature table looking for the feature.
	//
	for(size_t index=0; index<numFeatures; ++index)
	{
		if(featureID == featureTable[index].id)
		{
			return this->*featureTable[index].pFeature;
		}
	}

	throw UnsupportedFeatureException(featureID);
}

//==============================================================================
// ParserFeatureState::isFeatureSupported
//
//==============================================================================
bool ParserFeatureState::isFeatureSupported(int featureID) const
{
	//
	// Iterate over the feature table looking for the feature.
	//
	for(size_t index=0; index<numFeatures; ++index)
	{
		if(featureID == featureTable[index].id)
		{
			return true;
		}
	}

	return false;
}

QC_XML_NAMESPACE_END


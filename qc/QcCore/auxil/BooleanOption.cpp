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

#include "BooleanOption.h"
#include "messages.h"

#include "QcCore/base/IllegalArgumentException.h"
#include "QcCore/base/System.h"
#include "QcCore/util/MessageFormatter.h"

QC_AUXIL_NAMESPACE_BEGIN

const String sAuxil = QC_T("auxil");

using namespace util;

//==============================================================================
// BooleanOption::BooleanOption
//
/**
   Constructs a BooleanOption.

   @param longName the long name of this option
   @param shortName the short name of this option.  Specify 0 to indicate there
          is no short option name
   @param bDefault default value to report if the option is not specified on the
          command line
   @throws IllegalArgumentException if @c longName starts with '--'
*/
//==============================================================================
BooleanOption::BooleanOption(const String& longName, CharType shortName, bool bDefault) :
	BasicOption(longName, shortName, none),
	m_bValue(bDefault)
{
	// compare() params wrong way round in gnu libstdc++
	// if(longName.compare(0, 2, QC_T("--"))==0)
	if(longName.substr(0, 2) == QC_T("--"))
	{
		const String& errMsg = MessageFormatter::Format(
			System::GetSysMessage(sAuxil, EAUXIL_OPTIONNAMEBOOL,
			"illegal boolean option name: {0}"),
			longName);

		throw IllegalArgumentException(errMsg);
	}
}

const String BooleanOption::getArgument() const
{
	if(m_bValue)
		return QC_T("true");
	else
		return QC_T("false");
}

	
void BooleanOption::setPresent(const String& howSpecified, OptionType optionTypeUsed)
{
	BasicOption::setPresent(howSpecified, optionTypeUsed);

	if(optionTypeUsed == ShortOption)
	{
		// when short option is used for boolean options we reverse the value
		m_bValue = !m_bValue;
	}
	else //long option specified
	{
		if(howSpecified.substr(0, 2) == QC_T("no"))
			m_bValue = false;
		else
			m_bValue = true;
	}
}

bool BooleanOption::testLongOption(const String& option) const
{
	if(option.substr(0, 2) == QC_T("no"))
	{
		String comparitor = option.substr(2);
		return (getLongName().substr(0, comparitor.size()) == comparitor);
	}
	else
	{
		return (getLongName().substr(0, option.size()) == option);
	}
}

bool BooleanOption::getValue() const
{
	return m_bValue;
}

QC_AUXIL_NAMESPACE_END


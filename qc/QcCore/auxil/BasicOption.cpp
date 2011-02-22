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

#include "BasicOption.h"
#include "messages.h"

#include "QcCore/base/System.h"
#include "QcCore/util/MessageFormatter.h"

QC_AUXIL_NAMESPACE_BEGIN 

using namespace util;

const String sAuxil = QC_T("auxil");

BasicOption::BasicOption(const String& longName, CharType shortName, ArgumentType eArgumentType) :
	m_bPresent(false),
	m_eArgumentType(eArgumentType),
	m_longName(longName),
	m_shortName(shortName)
{
}

BasicOption::~BasicOption()
{
}

bool BasicOption::testShortOption(CharType option) const
{
	return (option == m_shortName);
}

bool BasicOption::testLongOption(const String& option) const
{
	// missing from gnu libstdc++
	// return m_longName.compare(0, option.size(), option)==0;
	return m_longName.substr(0, option.size()) == option;
}

BasicOption::ArgumentType BasicOption::getArgumentType() const
{
	return m_eArgumentType;
}

void BasicOption::setArgument(const String& arg)
{
	m_argument = arg;
}

const String BasicOption::getArgument() const
{
	return m_argument;
}

const String& BasicOption::getLongName() const
{
	return m_longName;
}

CharType BasicOption::getShortName() const
{
	return m_shortName;
}

//==============================================================================
// BasicOption::setPresent
//
/**
   Called by the CommandLineParser to indicate to the option that is has been
   specified on the command line.
   
   @param option how the option was specified on the command line
   @param optionTypeUsed indicates whether the long or short option was used

   @throws CommandLineException if the option is a duplicate
*/
//==============================================================================
void BasicOption::setPresent(const String& option, OptionType /*optionTypeUsed*/)
{
	if(m_bPresent)
	{
		const String& errMsg = MessageFormatter::Format(
			System::GetSysMessage(sAuxil, EAUXIL_OPTIONDUPLICATE,
			"duplicate option: {0}"),
			option);

		throw CommandLineException(option, errMsg);
	}

	m_bPresent = true;
}

bool BasicOption::isPresent() const
{
	return m_bPresent;
}

QC_AUXIL_NAMESPACE_END 

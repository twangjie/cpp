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

#ifndef QC_AUXIL_BasicOption_h
#define QC_AUXIL_BasicOption_h

#include "defs.h"
#include "CommandLineOption.h"

QC_AUXIL_NAMESPACE_BEGIN

class QC_AUXIL_PKG BasicOption : public CommandLineOption
{
public:
	BasicOption(const String& longName, CharType shortName, ArgumentType argType);
	virtual ~BasicOption();
	
	virtual const String getArgument() const;

	const String& getLongName() const;
	CharType getShortName() const;
	
	bool isPresent() const;

	// CommandLineOption interface ---v
	//
	virtual void setArgument(const String& arg);
	virtual bool testShortOption(CharType option) const;
	virtual bool testLongOption(const String& option) const;
	virtual ArgumentType getArgumentType() const;
	virtual void setPresent(const String& howSpecified, OptionType optionTypeUsed);
	//
	// ^--- end of CommandLineOption interface

private:
	String m_argument;
	bool m_bPresent;
	ArgumentType m_eArgumentType;
	String m_longName;
	CharType m_shortName;
};

QC_AUXIL_NAMESPACE_END

#endif //QC_AUXIL_BasicOption_h

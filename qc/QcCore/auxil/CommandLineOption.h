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

#ifndef QC_AUXIL_CommandLineOption_h
#define QC_AUXIL_CommandLineOption_h

#include "defs.h"
#include "CommandLineException.h"

QC_AUXIL_NAMESPACE_BEGIN

class QC_AUXIL_PKG CommandLineOption
{
public:
	enum ArgumentType {none, optional, mandatory};
	enum OptionType {ShortOption, LongOption};

	virtual void setArgument(const String& arg)=0;
	virtual bool testShortOption(CharType option) const=0;
	virtual bool testLongOption(const String& option) const=0;
	virtual ArgumentType getArgumentType() const=0;
	virtual void setPresent(const String& howSpecified, OptionType optionTypeUsed)=0;
};

QC_AUXIL_NAMESPACE_END

#endif  //QC_AUXIL_CommandLineOption_h

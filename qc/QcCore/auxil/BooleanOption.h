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

#ifndef QC_AUXIL_BooleanOption_h
#define QC_AUXIL_BooleanOption_h

#include "defs.h"
#include "BasicOption.h"

//
// A BooleanOption is like a BasicOption but with an aditional alias
// of [no]longName.
//

QC_AUXIL_NAMESPACE_BEGIN

class QC_AUXIL_PKG BooleanOption : public BasicOption
{
public:
	BooleanOption(const String& longName, CharType shortName, bool bDefault);
	
	virtual const String getArgument() const;
	virtual void setPresent(const String& howSpecified, OptionType optionTypeUsed);
	virtual bool testLongOption(const String& option) const;

	bool getValue() const;

private:
	bool m_bValue;
};

QC_AUXIL_NAMESPACE_END

#endif //QC_AUXIL_BooleanOption_h

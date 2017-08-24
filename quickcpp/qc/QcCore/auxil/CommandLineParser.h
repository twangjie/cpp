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

#ifndef QC_AUXIL_CommandLineParser_h
#define QC_AUXIL_CommandLineParser_h

#include "defs.h" 
#include "QcCore/base/String.h" 

#include <list>

QC_AUXIL_NAMESPACE_BEGIN 

class CommandLineOption;

class QC_AUXIL_PKG CommandLineParser
{
public:
	typedef std::list<String> StringList;

	CommandLineParser();

	unsigned parse(int argc, char *argv[]);
	void addOption(CommandLineOption* pOption);
	
	unsigned getFirstPositionalArg() const;
	const String& getProgramName() const;
	
	StringList getFilenames(int argc, char *argv[], int firstArg=0, int lastArg=0);

protected:
	CommandLineOption& getShortOption(char option) const;
	CommandLineOption& getLongOption(const String& option) const;

private:
	typedef std::list<CommandLineOption*> OptionList;
	OptionList m_optionList;	// un-owned
	unsigned m_firstPositionalArg;
	String m_programName;
	CharType m_shortOptionPrefix;
};

QC_AUXIL_NAMESPACE_END 

#endif  //QC_AUXIL_CommandLineParser_h


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

#include "CommandLineParser.h"
#include "CommandLineException.h"
#include "CommandLineOption.h"
#include "messages.h"

#include "QcCore/base/debug.h"
#include "QcCore/base/System.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/util/MessageFormatter.h"

#if defined(WIN32)
#include "QcCore/base/Win32Exception.h"
#include "QcCore/util/Win32Utils.h"
#endif //WIN32

#include <string.h>

QC_AUXIL_NAMESPACE_BEGIN 

const String sAuxil = QC_T("auxil");

using namespace util;

// Decide whether or not to allow DOS style short options (i.e. /h)
bool bDosStyleOptions = 
#if defined(WIN32)
	true;
#else
	false;
#endif //WIN32

//==============================================================================
// CommandLineParser::CommandLineParser
//
/**
   Constructs a CommandLineParser.
*/
//==============================================================================
CommandLineParser::CommandLineParser() :
	m_firstPositionalArg(0)
{
}

//==============================================================================
// CommandLineParser::parse
//
/**
   Parses the passed command line parameters, extracting any option names and 
   associated arguments.

   Parsing is conducted according to POSIX rules.  Short option names (defined
   as a single character) are recognized when prefixed by the short option prefix
   '-' ('/' is also recognized on Windows platforms).
   
   Parsing stops as soon as a non-option argument is found.  

   @returns the index of the first non-option argument
   @throws CommandLineException if invalid options have been specified on the
           command line
*/
//==============================================================================
unsigned CommandLineParser::parse(int argc, char *argv[])
{
	// default to prevent unwanted errors
	m_firstPositionalArg = argc;

	//
	// Extract the program name from argv[0]
	//
	if(argc > 0)
	{
		m_programName = StringUtils::FromNativeMBCS(argv[0]);
		
		//
		// Strip off the directory name if present.
		//
		size_t pos = m_programName.find_last_of(QC_T("\\/"));
		if(pos != String::npos)
		{
			m_programName = m_programName.substr(pos+1);
		}
	}

	int i = 1;	 // starting at 1 (argv[0] is the program name)
	for (; i<argc; i++)
	{
		char* pArg = argv[i];

		size_t argLen = strlen(pArg);

		// if this is a single-charater argument then it cannot be an
		// option - therefore we stop parsing [strict POSIX compliance]

		if(1 == argLen)
		{
			break;
		}
		else if(strncmp(pArg, "--", 2) == 0)	// is it a long option?
		{
			if(2 == argLen)
			{
				i++;    // special case where "--" appears on its own
				break;  // to signify the end of options
			}
			String sOption = StringUtils::FromNativeMBCS(pArg+2);
			//
			// create a string representation for reporting 
			//
			String displayOption = QC_T("-");
			displayOption += sOption;

			//
			// Long options have a few quirks:
			// i)   If it accepts arguments then there may be a "=" followed by
			//      the argument (this is the only way to specify optional arguments).
			// ii)  If the argument is mandatory, and there is no "=", then the next
			//      command line argument is assumed to be the option argument
			// iii) boolean options can be switched off by prefixing the option
			//      with "no" (e.g. --noverbose vs --verbose) This last quirk
			//      is the responsibility of each individual CommandLineOption to
			//      decide
			//

			String sArg;
			bool bHasArg = false;

			size_t eqPos = sOption.find_first_of('=');

			if(eqPos != String::npos)
			{
				sArg = sOption.substr(eqPos+1);
				sOption = sOption.substr(0, eqPos);
				bHasArg = true;
			}

			CommandLineOption& option = getLongOption(sOption);

			//
			// Inform the CommandLineOption that it has been selected
			//
			option.setPresent(sOption, CommandLineOption::LongOption);

			const CommandLineOption::ArgumentType argType = option.getArgumentType();

			if(argType != CommandLineOption::none)
			{
				if(bHasArg)
				{
					option.setArgument(sArg);
				}
				else if(argType == CommandLineOption::mandatory)
				{
					//
					// If there is a spare command line argument, then
					// we'll take that thank you very much, otherwise
					// we have a missing argument problem
					//
					if(i+1 < argc)
					{
						String arg = StringUtils::FromNativeMBCS(argv[++i]);	// steal the next argument
						option.setArgument(arg);
					}
					else
					{
						const String& errMsg = MessageFormatter::Format(
							System::GetSysMessage(sAuxil, EAUXIL_OPTIONARGMISSING,
							"the {0} option requires an argument"),
							displayOption);

						throw CommandLineException(displayOption, errMsg);
					}
				}
			}
		}
		else if('-' == *pArg || ('/' == *pArg && bDosStyleOptions))    // old-style short option?
		{
			m_shortOptionPrefix = *pArg;
			while (*(++pArg))
			{
				char cOption = *pArg;

				//
				// create a string representation for reporting 
				//
				String displayOption(1, m_shortOptionPrefix);
				displayOption += cOption;

				//
				// locate the Option coresponding to the parsed input
				// (throws exception if not found)
				//
				CommandLineOption& option = getShortOption(cOption);

				//
				// Inform the CommandLineOption that it has been selected
				//
				option.setPresent(String(1, cOption), CommandLineOption::ShortOption);

				const CommandLineOption::ArgumentType argType = option.getArgumentType();
				
				if(argType != CommandLineOption::none)
				{
					if(*(pArg+1))
					{
						String arg = StringUtils::FromNativeMBCS(pArg+1);
						option.setArgument(arg);
						break;	// end of short option
					}
					else
					{
						if(argType == CommandLineOption::mandatory)
						{
							//
							// If there is a spare command line argument, then
							// we'll take that thank you very much, otherwise
							// we have a missing argument problem
							//
							if(i+1 < argc)
							{
								String arg = StringUtils::FromNativeMBCS(argv[++i]);	// steal the next argument
								option.setArgument(arg);
							}
							else
							{
								const String& errMsg = MessageFormatter::Format(
									System::GetSysMessage(sAuxil, EAUXIL_OPTIONARGMISSING,
									"the {0} option requires an argument"),
									displayOption);

								throw CommandLineException(displayOption, errMsg);
							}
						}
					}
				}
			}
		}
		else
		{
			break;
		}
	}

	return m_firstPositionalArg=i;
}


//==============================================================================
// CommandLineParser::getFirstPositionalArg()
// 
// Following a successful parse, return the argument that represents the first
// non-option (ie positional) argument.
//
//==============================================================================
unsigned CommandLineParser::getFirstPositionalArg() const
{
	return m_firstPositionalArg;
}

//==============================================================================
// CommandLineParser::addOption
// 
// Add a CommandLineOption to our list of options.
// Note: we use a pointer parameter to indicate to the caller that a copy of the
// pointer is taken.  We do not take ownership of the pointer, just keep a
// copy of it until we are destroyed.
//
//==============================================================================
void CommandLineParser::addOption(CommandLineOption* pOption)
{
	m_optionList.push_back(pOption);
}

//==============================================================================
// CommandLineParser::getShortOption
//
/**
   Locates a CommandLineOption that answers to the short option passed.

   @returns The requested CommandLineOption object
   @throws CommandLineException if the option is unrecognized or ambiguous
*/
//==============================================================================
CommandLineOption& CommandLineParser::getShortOption(char option) const
{
	CommandLineOption* pRet = NULL;

	//
	// create a string representation for reporting 
	//
	String displayOption(1, m_shortOptionPrefix);
	displayOption += option;

	for (OptionList::const_iterator iter=m_optionList.begin(); iter!=m_optionList.end(); iter++)
	{
		if((*iter)->testShortOption(option))
		{
			if(NULL == pRet)
			{
				pRet = *iter;
			}
			else
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sAuxil, EAUXIL_OPTIONNAMEAMBIGUOUS,
					"the {0} option is ambiguous"),
					displayOption);

				throw CommandLineException(displayOption, errMsg);
			}
		}
	}

	if(pRet)
	{
		return (*pRet);
	}
	else
	{
		const String& errMsg = MessageFormatter::Format(
			System::GetSysMessage(sAuxil, EAUXIL_OPTIONNAMEUNKNOWN,
			"unrecognized option: {0}"),
			displayOption);

		throw CommandLineException(displayOption, errMsg);
	}
}

//==============================================================================
// CommandLineParser::getLongOption()
// 
// Locate a CommandLineOption that answers to the long option passed.
//
// It is up to the CommandLineOptions themselves to decide whether they are
// applicable.
//==============================================================================
CommandLineOption& CommandLineParser::getLongOption(const String& option) const
{
	CommandLineOption* pRet = NULL;

	//
	// create a string representation for reporting 
	//
	String displayOption = QC_T("--");
	displayOption += option;

	for (OptionList::const_iterator iter=m_optionList.begin(); iter!=m_optionList.end(); iter++)
	{
		if((*iter)->testLongOption(option))
		{
			if(NULL == pRet)
			{
				pRet = *iter;
			}
			else
			{
				const String& errMsg = MessageFormatter::Format(
					System::GetSysMessage(sAuxil, EAUXIL_OPTIONNAMEAMBIGUOUS,
					"the {0} option is ambiguous"),
					displayOption);

				throw CommandLineException(displayOption, errMsg);
			}
		}
	}

	if(pRet)
	{
		return (*pRet);
	}
	else
	{
		const String& errMsg = MessageFormatter::Format(
			System::GetSysMessage(sAuxil, EAUXIL_OPTIONNAMEUNKNOWN,
			"unrecognized option: {0}"),
			displayOption);

		throw CommandLineException(displayOption, errMsg);
	}
}

//==============================================================================
// CommandLineParser::getProgramName()
// 
// Return the program name.  This is set during the command line parsing.
//==============================================================================
const String& CommandLineParser::getProgramName() const
{
	return m_programName;
}


//==============================================================================
// CommandLineParser::getFilenames
//
// This is a useful function for making windows command line programs behave
// in apparently the same way that unix shell programs behave.
//
// The unix shell is much more powerful that the windows shell.  One of the 
// features missing from the windows shell is hte lack of filename expension.
// This function allows us to write command line programs withou worrying
// about this issue - because we expand the filename wildcards here.
// 
// Under unix this is a very simple wrapper.  Under Win32 it is a little
// more involved.
//==============================================================================
CommandLineParser::StringList CommandLineParser::getFilenames(int argc, char *argv[], int firstArg, int lastArg)
{
	StringList fileList;

	if(firstArg == 0)
		firstArg = m_firstPositionalArg;

	if(lastArg == 0)
		lastArg = argc-1;

	if(firstArg < argc)
	{
		for(int argp=firstArg; argp <= lastArg; argp++)
		{
			String arg = StringUtils::FromNativeMBCS(argv[argp]);

#ifdef WIN32
			// if the filename contains any wildcards then we need to expand it
			if(arg.find_first_of(QC_T("*?")) != String::npos)
			{

				try
				{
					//fileList.splice(fileList.end(), Win32Utils::ExpandFiles(arg));
					StringList files = Win32Utils::ExpandFiles(arg);
					fileList.splice(fileList.end(), files);
				}
				catch(Win32Exception& /*e*/)
				{
					// treat failures like unix - the name simply couldn't be expanded!
					fileList.push_back(arg);
				}
			}
			else
			{
				fileList.push_back(arg);
			}
#else // !WIN32
			fileList.push_back(arg);
#endif // WIN32
		}
	}
	return fileList;
}

QC_AUXIL_NAMESPACE_END 


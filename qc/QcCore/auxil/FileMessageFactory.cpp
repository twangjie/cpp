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

#include "FileMessageFactory.h"

#include "QcCore/base/System.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/base/SystemUtils.h"
#include "QcCore/io/BufferedReader.h"
#include "QcCore/io/File.h"
#include "QcCore/io/InputStreamReader.h"
#include "QcCore/io/FileInputStream.h"

QC_AUXIL_NAMESPACE_BEGIN

using namespace io;
using namespace util;

//==============================================================================
// FileMessageFactory::Install
//
// Static function to install a FileMessageFactory if the appropriate
// environment is present.
//==============================================================================
void FileMessageFactory::Install()
{
	//
	// If the environment variable: QC_MSG_DIR is set then hey! let's
	// use internationalized messages
	//
	const String& sMsgDir = System::GetEnvironmentString(QC_T("QC_MSG_DIR"));
	if(!sMsgDir.empty())
	{
		const String& sLang = System::GetEnvironmentString(QC_T("LANG"));
		System::SetMessageFactory(new FileMessageFactory(sMsgDir, sLang));
	}
}

//==============================================================================
// FileMessageFactory::FileMessageFactory
//
//==============================================================================
FileMessageFactory::FileMessageFactory(const String& msgDirectory, const String lang) :
	m_lang(lang),
	m_bDirectoryOK(false)
{
	//
	// A common user error will be to put a trailing separator char on the end of
	// the directory, so we'll remove it now.
	//
	if(!msgDirectory.empty())
	{
		size_t pos = msgDirectory.find_last_not_of(QC_T("/\\"));
		if(pos != String::npos)
		{
			m_msgDir = msgDirectory.substr(0, pos+1);
		}
	}

	m_bDirectoryOK = File(m_msgDir).isDirectory();
}

//==============================================================================
// FileMessageFactory::getMessageSet
//
//==============================================================================
AutoPtr<MessageSet> FileMessageFactory::getMessageSet(const String& org, const String& app)
{
	AutoPtr<MessageSet> rpSet = new MessageSet(org, app);

	if(m_bDirectoryOK)
	{
		//
		// construct the name of the message file that we will read to create
		// the requested message set
		//
		// filename ::= msg-dir '/' [org-name '/'] [lang'/'] app-nane '.msg'
		//
		const String sDelim = QC_T(" \t");

		String filename = m_msgDir;
		filename += File::GetSeparatorChar();

		if(!org.empty())
		{
			filename += org;
			filename += File::GetSeparatorChar();
		}

		if(!m_lang.empty())
		{
			filename += m_lang;
			filename += File::GetSeparatorChar();
		}

		filename += app;
		filename += QC_T(".msg");


		//
		// Now attempt to open the file.  All failures are silently ignored, with the
		// result that messages are presented to the user using the English prototype.
		//
		try
		{
			//
			// Make use of the QC I/O facilities.  The input file must be coded
			// in UTF-8, so we use a UTF-8 Reader
			//
			const String sUTF8 = QC_T("UTF-8");

			AutoPtr<BufferedReader> rpReader = new BufferedReader(
			                                  new InputStreamReader(
											  new FileInputStream(filename), sUTF8));

			//
			// Read the message file, one line at a time.
			//
			// Note that multi-line messages are permitted.  To cater for this
			// we do not add the message to the MessageSet until we have read all 
			// messages for the given number.
			//

			String line;
			String message;
			size_t messageID=0;

			while(rpReader->readLine(line) != Reader::EndOfFile)
			{
				if(line[0] == '#')
					continue;

				//
				// It was the intention to allow the input file to be relatively free-format
				// with a number followed by a string with any number of separator characters
				// in between.  The problem with this approach is that some messages need
				// leading spaces (such as the verbose usage display when the --help option
				// is given to a command-line program).
				//
				// For this reason we only permit one separator character between the
				// number and the start of the message.  The message can start at any
				// position, but it must be the first non-blank token on a line.
				//
				size_t numPos = line.find_first_not_of(sDelim);
				if(numPos != String::npos)
				{
					size_t msgPos = line.find_first_of(sDelim, numPos+1);

					size_t newMessageID;
					String newMessage;

					if(msgPos == String::npos)
					{
						newMessageID = NumUtils::ToLong(line.substr(numPos));
					}
					else
					{
						newMessageID = NumUtils::ToLong(line.substr(numPos, msgPos-numPos));
						newMessage = line.substr(msgPos+1);
					}

					//
					// okay, we have the message number (which cannot be zero)
					//
					if(newMessageID)
					{
						//
						// If it the same message ID as last time round the loop, then
						// append the message to the saved message...
						//
						if(newMessageID == messageID)
						{
							message += QC_T("\n");
							message += newMessage;
						}
						else 
						{
							//
							// ...otherwise add the saved message to the MessageSet
							//
							if(!message.empty())
							{
								rpSet->addMessage(messageID, message);
							}
							//
							// and save the current message for the next time round the loop
							//
							message = newMessage;
							messageID = newMessageID;
						}
					}
				}
			}
			// add the last one if present
			if(!message.empty())
			{
				rpSet->addMessage(messageID, message);
			}
		}
		catch(Exception& /*e*/)
		{
		}
	}
	
	return rpSet;
}

QC_AUXIL_NAMESPACE_END

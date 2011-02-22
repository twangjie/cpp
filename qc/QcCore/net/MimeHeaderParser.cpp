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

#include "MimeHeaderParser.h"

#include "QcCore/base/StringUtils.h"
#include "QcCore/base/Tracer.h"

QC_NET_NAMESPACE_BEGIN

//==============================================================================
// MimeHeaderSequence::parseInputStream
//
// Clear the headers before parsing an InputStream for MIME headers.
// 
// Mime-type headers comprise a keyword followed by ":" followed
// by whitespace and the value.  If a line begins with witespace it is
// a continuation of the preceeding line.
//
// MIME headers are delimited by an empty line.
//=============================================================================
AutoPtr<MimeHeaderSequence> MimeHeaderParser::ParseHeaders(InputStream* pInputStream)
{
	if(!pInputStream) throw NullPointerException();
	
	AutoPtr<MimeHeaderSequence> rpRet(new MimeHeaderSequence);

	Tracer::Trace(Tracer::Net, Tracer::Low, QC_T("reading headers:"));

	//
	// Until we reach the data part of the response we know we are
	// dealing with iso-8859-1 characters
	//
	String line;

	while (ReadLineLatin1(pInputStream, line) != InputStream::EndOfFile)
	{
		if(line.empty())
		{
			// we have reached the MIME headers delimiter
			break;
		}
		else
		{
			Tracer::Trace(Tracer::Net, Tracer::Low, line);

			//
			// Check for continuation of preceding header
			//
			if(UnicodeCharacterType::IsSpace(line[0]))
			{
				if(!rpRet->m_headerFields.empty())
				{
					//
					// The following strange syntax came about trying to
					// get ospace stl to work with workshop 4.2
					// - it didn't like m_headerFields[m_headerFields->size()-1]
					//
					MimeHeaderSequence::HeaderFieldEntry& entry = 
						*(rpRet->m_headerFields.end()-1);

					entry.second += StringUtils::StripWhiteSpace(line, StringUtils::both);
				}
			}
			else
			{
				size_t delimPos = line.find(QC_T(':'));
				String key;
				String value;
				if(delimPos != String::npos)
				{
					key = line.substr(0, delimPos);
					// strip any leading and trailing white-space
					value = StringUtils::StripWhiteSpace(line.substr(delimPos+1), StringUtils::both);
				}
				else
				{
					value = StringUtils::StripWhiteSpace(line, StringUtils::trailing);
				}
				rpRet->insertHeader(key, value);
			}
		}
	}
	return rpRet;
}

//==============================================================================
// MimeHeaderParser::ReadLineLatin1
//
//==============================================================================
long MimeHeaderParser::ReadLineLatin1(InputStream* pInputStream, String& retLine)
{
	retLine.erase();

	long charCount=0;
	int x;

	while( (x=pInputStream->read()) != InputStream::EndOfFile)
	{
		if(x == '\n')
		{
			return charCount;
		}
		else if(x != '\r')
		{
			Character ch((UCS4Char)x);
			ch.appendToString(retLine);
			++charCount;
		}
	}
	return InputStream::EndOfFile;
}

QC_NET_NAMESPACE_END

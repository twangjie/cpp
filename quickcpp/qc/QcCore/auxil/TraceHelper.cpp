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

#include "TraceHelper.h"

#include "QcCore/base/System.h"
#include "QcCore/base/Thread.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/util/AttributeListParser.h"
#include "QcCore/io/FileOutputStream.h"
#include "QcCore/io/OutputStreamWriter.h"
#include "QcCore/net/Socket.h"

QC_AUXIL_NAMESPACE_BEGIN

using namespace util;
using namespace io;
using namespace net;

const CharType* szFile = QC_T("file");
const CharType* szHost = QC_T("host");
const CharType* szPort = QC_T("port");
const CharType* szFlush = QC_T("flush");
const CharType* szAll  = QC_T("all");
const CharType FieldSep = QC_T('|');

//==============================================================================
// TraceHelper::TraceHelper
//
//==============================================================================
TraceHelper::TraceHelper(const String& name, const String& command) :
	m_nAllLevel(0),
	m_name(name),
	m_bAutoFlush(false)
{
	//
	// Initialize our section table
	//
	for(short i=0; i<MaxSections; ++i)
	{
		m_sectionTable[i] = 0;
	}
	
	//
	// Parse the command string, which is of the form:
	// "File=filename flush=flush <section>=<number> <section>=<number>..."
	//
	if(!command.empty())
	{
		AttributeListParser parser;
		parser.parseString(command);
		const String& filename=parser.getAttributeValueICase(szFile);
		const String& host=parser.getAttributeValueICase(szHost);
		if(filename.size())
		{
			m_rpWriter = new OutputStreamWriter(new FileOutputStream(filename));
		}
		else if(host.size())
		{
			int port = 1122;
			const String& sPort=parser.getAttributeValueICase(szPort);
			if(sPort.size())
				port = NumUtils::ToInt(sPort);
			AutoPtr<Socket> rpSocket = new Socket(host, port);
			m_rpWriter = new OutputStreamWriter(rpSocket->getOutputStream().get());
		}

		parser.removeAttribute(szFile);
		parser.removeAttribute(szHost);
		parser.removeAttribute(szPort);

		if(m_rpWriter)
		{
			m_rpWriter->write(name);
			m_rpWriter->write(QC_T(" trace started at "));
			m_rpWriter->write(DateTime::GetSystemTime().toString());
			m_rpWriter->write(System::GetLineEnding());
			m_rpWriter->write(QC_T("Trace options: "));
			m_rpWriter->write(command);
			m_rpWriter->write(System::GetLineEnding());
			m_rpWriter->flush();

			for(size_t i=0; i<parser.getAttributeCount(); ++i)
			{
				const String& section = parser.getAttributeName(i);

				if(StringUtils::CompareNoCase(section, szFlush)==0)
				{
					m_bAutoFlush = true;
				}
				else
				{
					int level = NumUtils::ToInt(parser.getAttributeValue(i));
					short nSection = GetSectionNumber(section);
					if(nSection != -1)
					{
						doActivate(nSection, level);
					}
				}
			}
		}
	}
}

//==============================================================================
// TraceHelper::~TraceHelper
//
//==============================================================================
TraceHelper::~TraceHelper()
{
	Enable(false);

	if(m_rpWriter)
	{
		try
		{
			m_rpWriter->write(QC_T("stopped at "));
			m_rpWriter->write(DateTime::GetSystemTime().toString());
			m_rpWriter->write(System::GetLineEnding());
			m_rpWriter->close();
		}
		catch(Exception& /*e*/)
		{
		}
	}
}

//==============================================================================
// TraceHelper::doTrace
//
//==============================================================================
void TraceHelper::doTrace(short nSection, short nLevel, const CharType* message, size_t len)
{
	if(nSection > 0 && nSection < MaxSections && m_rpWriter && (nLevel <= m_nAllLevel || nLevel <= m_sectionTable[nSection]))
	{
		try
		{
			QC_SYNCHRONIZED_PTR(m_rpWriter->getLock().get())
			// Disable tracing to avoid infinite recursion where
			// tracing mechanism itself causes tracing output
			Enable(false);
			formatOutput(nSection, nLevel);
			m_rpWriter->write(message, len);
			m_rpWriter->write(System::GetLineEnding());
			if(m_bAutoFlush)
				m_rpWriter->flush();
			Enable(true);
		}
		catch(Exception&)
		{
			Enable(true);
		}
	}
}

//==============================================================================
// TraceHelper::doTrace
//
//==============================================================================
void TraceHelper::doTraceBytes(short nSection, short nLevel,
                               const String& message, const Byte* bytes, size_t len)
{
	if(nSection > 0 && nSection < MaxSections && m_rpWriter && (nLevel <= m_nAllLevel || nLevel <= m_sectionTable[nSection]))
	{
		try
		{
			QC_SYNCHRONIZED_PTR(m_rpWriter->getLock().get())
			// Disable tracing to avoid infinite recursion where
			// tracing mechanism itself causes tracing output
			Enable(false);
			formatOutput(nSection, nLevel);
			m_rpWriter->write(message);
			m_rpWriter->write(StringUtils::FromLatin1((char*)bytes, len));
			m_rpWriter->write(System::GetLineEnding());
			if(m_bAutoFlush)
				m_rpWriter->flush();
			Enable(true);
		}
		catch(Exception&)
		{
			Enable(true);
		}
	}
}

//==============================================================================
// TraceHelper::formatOutput
//
// MT note: the writer's lock should be held prior to calling this
//==============================================================================
void TraceHelper::formatOutput(short nSection, short nLevel)
{
	m_rpWriter->write(FieldSep);
	m_rpWriter->write(NumUtils::ToString(DateTime::GetSystemTime().toAnsiTime()));
	m_rpWriter->write(FieldSep);

#ifdef QC_MT
	m_rpWriter->write(Thread::CurrentThreadId().toString());
	m_rpWriter->write(FieldSep);
#endif //QC_MT

	const CharType* pSectionName = GetSectionName(nSection);
	if(pSectionName)
	{
		m_rpWriter->write(pSectionName);
	}
	else
	{
		m_rpWriter->write(QC_T("unknown"));
	}
	m_rpWriter->write(StringUtils::FromLatin1(StringUtils::Format("|%03d|", (int)nLevel)));
}

//==============================================================================
// TraceHelper::doActivate
//
// todo protect this, which means we should set the writer lock equal to our id
//==============================================================================
void TraceHelper::doActivate(short nSection, short nLevel)
{
	if(nSection == Tracer::All)
	{
		m_nAllLevel = nLevel;
	}
	else if(nSection > 0 && nSection < MaxSections)
	{
		m_sectionTable[nSection] = nLevel;
	}

	if(nLevel)
	{
		Tracer::Enable(true);
	}
}

QC_AUXIL_NAMESPACE_END

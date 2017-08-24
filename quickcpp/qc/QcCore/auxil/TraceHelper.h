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
//
// Class: TraceHelper
// 
// Overview
// --------
// A class that performs simple yet effecting tracing.
//
//==============================================================================

#ifndef TraceHelper_h
#define TraceHelper_h

#include "defs.h"

#include "QcCore/base/Tracer.h"
#include "QcCore/io/Writer.h"

QC_AUXIL_NAMESPACE_BEGIN

using io::Writer;

class QC_AUXIL_PKG TraceHelper : public Tracer
{
public:
	TraceHelper(const String& name, const String& command);
	~TraceHelper();

protected:
	virtual void doTrace(short nSection, short nLevel, const CharType* message, size_t len);
	virtual void doTraceBytes(short nSection, short nLevel, const String& message, const Byte* bytes, size_t len);
	virtual void doActivate(short nSection, short nLevel);

	void formatOutput(short nSection, short nLevel);

private:
	enum {MaxSections = 32};
	short m_sectionTable[MaxSections];
	AutoPtr<Writer> m_rpWriter;
	short m_nAllLevel;
	String m_name;
	bool m_bAutoFlush;
};

QC_AUXIL_NAMESPACE_END

#endif //TraceHelper_h

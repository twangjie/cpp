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

#ifndef QC_BASE_Tracer_h
#define QC_BASE_Tracer_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include "ManagedObject.h"
#include "String.h"

QC_BASE_NAMESPACE_BEGIN

class QC_BASE_PKG Tracer : public virtual ManagedObject
{
public:
	enum Levels {Highest=0,     /*!< important events that are always traced */
	             Exceptions=10, /*!< exception events */
	             High=20,       /*!< high priority events */
	             Medium=40,     /*!< medium priority events */
	             Low=60,        /*!< low-priority events */
	             Min=99         /*!< the lowest priority */
	};

	enum Sections {All=0,       /*!< used to activate all sections */
	               Base=1,      /*!< classes within the top-level @QuickCPP namespace */
	               Util=2,      /*!< classes within the util namespace */
	               IO=3,        /*!< classes within the io namespace */
	               Net=4,       /*!< classes within the net namespace */
	               Auxil=9,     /*!< classes within the auxil namespace */
	               XML=10,      /*!< classes within the xml namespace */
	               User=16      /*!< starting value for user-assigned sections */
	};

	static void SetTracer(Tracer* pTracer);
 
	static void Trace(short nSection, short nLevel, const String& message);
	static void Trace(short nSection, short nLevel, const CharType* message, size_t len);
	static void TraceBytes(short nSection, short nLevel, const String& message, const Byte* bytes, size_t len);

	static void Activate(short nSection, short nLevel);
	static bool IsEnabled(){ return s_bEnabled; }
	static void Enable(bool bEnable);
	static const CharType* GetSectionName(short nSection);
	static short GetSectionNumber(const String& section);

protected:
	virtual void doTrace(short nSection, short nLevel, const CharType* message, size_t len)=0;
	virtual void doTraceBytes(short nSection, short nLevel, const String& message, const Byte* bytes, size_t len)=0;
	virtual void doActivate(short nSection, short nLevel)=0;

	virtual const CharType* getUserSectionName(short nSection);
	virtual short getUserSectionNumber(const String& section);

private:
	static Tracer* QC_MT_VOLATILE s_pTracer;
	static QC_MT_VOLATILE bool s_bEnabled;
};

QC_BASE_NAMESPACE_END
#endif //QC_BASE_Tracer_h

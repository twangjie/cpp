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
// Class: System
// 
// Overview
// --------
// The System class is a well-known and accessible class module
// that provides access to important system information. . It cannot be
// instantiated - all methods are static.
//
//==============================================================================

#ifndef QC_BASE_System_h
#define QC_BASE_System_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include "String.h"
#include "AutoPtr.h"

QC_BASE_NAMESPACE_BEGIN

class ObjectManager;
class MessageFactory;

class QC_BASE_PKG System
{
public:

	static ObjectManager& GetObjectManager();

	static String GetAppMessage(const String& org, const String& app,
	                            size_t messageID, const char* szDefault);

	static String GetSysMessage(const String& lib,
	                            size_t messageID, const char* szDefault);

	static AutoPtr<MessageFactory> GetMessageFactory();
	static void SetMessageFactory(MessageFactory* pFactory);

	static void Terminate();

	static const String& GetLineEnding();
	static String GetEnvironmentString(const String& name);
	static String GetVersionAsString();
	static unsigned long GetVersion();

	static String GetProperty(const String& name, const String& defaultValue);
	static String GetProperty(const String& name);
	static long GetPropertyLong(const String& name, long defaultValue);
	static bool GetPropertyBool(const String& name, bool bDefault);

	static void SetProperty(const String& name, const String& value);
	static void SetPropertyLong(const String& name, long value);
	static void SetPropertyBool(const String& name, bool bSet);

private:
	System(); // not implemented

	static ObjectManager * QC_MT_VOLATILE s_pObjectManager;
	static MessageFactory * QC_MT_VOLATILE s_pMessageFactory;
};

QC_BASE_NAMESPACE_END

#endif //QC_BASE_System_h


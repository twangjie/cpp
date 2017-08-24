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
/**
	@class qc::System
	
	@brief Class module providing useful system-level functions as
	well as providing a well-known and accessible root from which
	to navigate to other essential services.
*/
//==============================================================================

#include "System.h"
#include "String.h"
#include "FastMutex.h"
#include "ObjectManager.h"
#include "QCObject.h"
#include "MessageFactory.h"
#include "NumUtils.h"
#include "StringUtils.h"
#include "Thread.h"

#include <memory>
#include <map>

#include <string.h>

QC_BASE_NAMESPACE_BEGIN

//==================================================================
// Multi-threaded locking strategy
//
// To ensure that singleton resources are not created by multiple
// concurrent threads, the pointers are protected using a
// single static mutex.
//
// To minimize the time that the SystemMutex is held, we do
// not lock the mutex for the creation of a singleton object,
// only when a complete object has been created and we need to swap
// the address into our static pointer variable.
//==================================================================
#ifdef QC_MT
	FastMutex SystemMutex;
#endif //QC_MT

#ifndef QC_DOCUMENTATION_ONLY
	typedef std::map<String, String, std::less<String> > PropertyMap;
#endif

PropertyMap SystemProperties;                  // mutex protected
ObjectManager* QC_MT_VOLATILE System::s_pObjectManager = 0;   // mutex protected
MessageFactory* QC_MT_VOLATILE System::s_pMessageFactory = 0; // mutex protected

const String sEOL = 
#if defined(WIN32)
	QC_T("\r\n");
#else
	QC_T("\n");
#endif

//==============================================================================
// System::GetObjectManager
//
/**
   Returns a reference to the global ObjectManager, which can be used to
   hold instances of reference-counted objects for the lifetime of the
   application.
   @mtsafe
*/
//==============================================================================
ObjectManager& System::GetObjectManager()
{
	if(!s_pObjectManager)
	{
		//==================================================================
		// Multi-threaded locking strategy
		//
		// This uses the "double-checked locking pattern" (Schmidt 1996)
		// with a volatile storage member to minimise race conditions due
		// to the so-called "relaxed memory model"..
		//==================================================================

		QC_AUTO_LOCK(FastMutex, SystemMutex);

		// test again now that mutex has been locked
		if(!s_pObjectManager)
		{
			// note: Ideally this should be an ATOMIC operation
			// to protect code accessing the variable that doesn't lock the
			// mutex.
			s_pObjectManager = new ObjectManager;
		}
	}
	return *s_pObjectManager;
}

//==============================================================================
// System::GetSysMessage
//
/**
   Retrieves the text for a system message.

   Uses the application-supplied MessageFactory to locate the requested 
   message.  If the MessageFactory cannot provide the message then @c
   szDefault is converted into a String and returned.

   This function is used throughout the @QuickCPP library to access
   @a internationalized error and informational messages.

   @param lib name of the library component which is requesting the message
   @param messageID identifier for the message (unique within the library
          specified by @c lib)
   @param szDefault null-terminated ASCII string containing the default
          message text
   @returns the requested message text or the default value if no message is 
          available.
   @mtsafe
*/
//==============================================================================
String System::GetSysMessage(const String& lib,
                             size_t messageID, const char* szDefault)
{
	static const String elcel = QC_T("elcel");
	return GetAppMessage(elcel, lib, messageID, szDefault);
}

//==============================================================================
// System::GetAppMessage
//
/**
   Retrieves the text for a user-defined message.

   This function is can be used by applications or libraries built
   on top of @QuickCPP to access @a internationalized error and informational messages.

   The requested message is identified by the 3 parameters: @c org, @c app and @c messageID.
   These are passed to the application-supplied MessageFactory to locate the requested 
   message.  If the MessageFactory cannot provide the message then @c
   szDefault is converted into a String and returned.

   @param org name of the organization which created the application or library.  This
          can be any name except @c elcel which is reserved.
   @param app name of the application or library which is requesting the message
   @param messageID identifier for the message (unique within the library
          specified by @c app)
   @param szDefault null-terminated ASCII string containing the default
          message text
   @returns the requested message text or the default value if no message is 
          available.
   @mtsafe
*/
//==============================================================================
String System::GetAppMessage(const String& org, const String& app,
                             size_t messageID, const char* szDefault)
{
	String strRet;
	
	bool bFound = s_pMessageFactory && 
	              s_pMessageFactory->getMessage(org, app, messageID, strRet);

	//
	// If the message could not be created by the factory, simply create
	// a string from the passed prototype
	//
	if(!bFound)
	{
		size_t sLen = strlen(szDefault);
		strRet.reserve(sLen);

		const char* iter = szDefault;
		const char* end = szDefault + sLen;

		for(; iter < end; ++iter)
		{
			strRet += CharType(*iter);
		}
	}

	return strRet;
}

//==============================================================================
// System::Terminate
//
/**
   Called by the application when it is about to terminate.

   Releases any resources acquired during execution of the application including
   references to lazily instantiated global objects.  Once Terminate() has been
   called, further use of @QuickCPP classes is not supported.
   
   It is always a good idea for an application to call Terminate() to
   allow the @QuickCPP library to perform clean-up operations.  This is especially
   true when using @QuickCPP threads because System::Terminate() is responsible
   for ensuring that all user threads have terminated before it returns
   to the caller.

   It is also a good idea to ensure Terminate() is called correctly when using a
   memory-leak checker such as 
   <a target="_top" http://www.compuware.com/products/devpartner/bounds/">BoundsChecker</a>
   or the debug version of the Microsoft C Run-time Library.

   For safety, if the application has created @a daemon threads which are still
   active, Terminate() will not free the memory for global objects.  The @a daemon
   threads will terminate when the process terminates.

   To ensure that Terminate() is called even after an application throws an exception,
   it is recommended that the application make use of the SystemMonitor class
   rather than attempting to call Terminate() directly.
   
   @sa SystemMonitor
*/
//==============================================================================
void System::Terminate()
{

#ifdef QC_MT

	Thread::WaitAllUserThreads();
	//Thread::TerminateAllDaemonThreads();

	//
	// For safety, when daemon threads are active, we cannot free the
	// global ObjectManager.
	//
	bool bFreeGlobals = (Thread::GetActiveCount() == 0);

#else
	
	bool bFreeGlobals = true;

#endif //QC_MT

	//==================================================================
	// Multi-threaded locking strategy
	//
	// We protect the ObjectManager variable with our mutex.
	//
	// We do not hold the mutex lock to destroy the ObjectManager
	// for the following reasons:
	// i)  It may be a long operation
	// ii) It may involve the execution of user destructors which
	//     may call back into System functions.
	//==================================================================

	if(bFreeGlobals)
	{
		if(s_pObjectManager)
		{
			ObjectManager* pObjMgr = 0;
			{ // scope for auto_lock
				QC_AUTO_LOCK(FastMutex, SystemMutex);
				pObjMgr = s_pObjectManager;
				s_pObjectManager = 0;
			}

			if(pObjMgr)
				pObjMgr->unregisterAllObjects();

			delete pObjMgr;
		}
	}
}

//==============================================================================
// System::GetMessageFactory
//
/**
   Returns the MessageFactory for the application if one has been previously
   installed.

   @returns The previously installed MessageFactory or null
            if no MessageFactory has been installed.
   @mtsafe
*/
//==============================================================================
AutoPtr<MessageFactory> System::GetMessageFactory()
{
	return s_pMessageFactory;
}

//==============================================================================
// System::SetMessageFactory
//
/**
   Sets the MessageFactory global object.

   Installs a MessageFactory that will be used for subsequent message
   retrieval.

   To ensure that the passed object exists for as long as the application needs
   it, the MessageFactory object is registered with the system's ObjectManager
   which holds a (counted) reference to it until system termination.
   @mtsafe
*/
//==============================================================================
void System::SetMessageFactory(MessageFactory* pFactory)
{
	//==================================================================
	// Multi-threaded locking strategy
	//
	// Update access to static variables is mutex protected.
	// To avoid holding our mutex and then waiting for the ObjectManager's
	// mutex, we release our mutex before unregistering the old object.
	//==================================================================

	//
	// Register the new factory with the ObjectManager.
	// This can be done before the SystemMutex is locked.
	//
	if(pFactory)
	{
		GetObjectManager().registerObject(pFactory);
	}

	MessageFactory* pExisting;

	// Create a scope for the mutex lock
	{
		QC_AUTO_LOCK(FastMutex, SystemMutex);
		pExisting = s_pMessageFactory;
		s_pMessageFactory = pFactory;
	}

	// Note: There is no need to keep the mutex locked
	// while we unregister the existing factory
	if(pExisting) 
	{
		GetObjectManager().unregisterObject(pExisting);
	}
}

//==============================================================================
// System::GetLineEnding
//
/**
   Returns a String containing the line ending character(s) for the platform.

   @returns A String containing the line-ending characters.  On Unix this
            is @c '\n', on Windows it is @c '\r\n'.
   @mtsafe
*/
//==============================================================================
const String& System::GetLineEnding()
{
	return sEOL;
}

//==============================================================================
// System::GetEnvironmentString
//
/**
   Returns the specified environment variable as a String.

   @param name the name of the environment variable to return
   @returns the value of the environment variable @c name or an empty String if
            the environment variable is not set
   @mtsafe
*/
//==============================================================================
String System::GetEnvironmentString(const String& name)
{
	const char* const pEnv = getenv(StringUtils::ToNativeMBCS(name).c_str());
	if(pEnv)
	{
		return StringUtils::FromNativeMBCS(pEnv);
	}
	else
	{
		return String();
	}
}

//==============================================================================
// System::GetVersionAsString
//
/**
   Returns the version of @QuickCPP as a formatted string.

   The returned string takes the form "a.b.c" where
   -a is the major version @c (_QUICKCPP_MAJOR_VERSION)
   -b is the minor version number @c (_QUICKCPP_MINOR_VERSION)
   -c is the minor release number @c (_QUICKCPP_MINOR_RELEASE)
   @mtsafe
*/
//==============================================================================
String System::GetVersionAsString()
{
	return StringUtils::FromLatin1(
		StringUtils::Format("%d.%d.%d",
		                    _QUICKCPP_MAJOR_VERSION,
		                    _QUICKCPP_MINOR_VERSION,
		                    _QUICKCPP_MINOR_RELEASE));
}

//==============================================================================
// System::GetVersion
//
/**
   Returns the current version of the @QuickCPP library.

   The version number is made up of a major version, a minor version
   and a minor release number combined together to make an unsigned long
   integer like this:

   @code
   unsigned long version = (_QUICKCPP_MAJOR_VERSION * 100000)
                         + (_QUICKCPP_MINOR_VERSION * 1000)
                         + _QUICKCPP_MINOR_RELEASE;
   @endcode

   The @QuickCPP version may also be obtained at compile time from the 
   @c _QUICKCPP_VERSION macro.
   @mtsafe
*/
//==============================================================================
unsigned long System::GetVersion()
{
	return _QUICKCPP_VERSION;
}

//==============================================================================
// System::GetProperty
//
/**
   Returns the system property with the supplied name.

   @returns the requested property or @c defaultValue if it doesn't exist.
   @param name the name of the property
   @param defaultValue the default value
   @mtsafe
*/
//==============================================================================
String System::GetProperty(const String& name, const String& defaultValue)
{
	QC_AUTO_LOCK(FastMutex, SystemMutex);
	const PropertyMap::const_iterator i = SystemProperties.find(name);
	if(i != SystemProperties.end())
		return (*i).second;
	else
		return defaultValue;
}

//==============================================================================
// System::GetProperty
//
/**
   Returns the system property with the supplied name.

   @returns the requested property or the empty string if it doesn't exist.
   @param name the name of the property
   @mtsafe
*/
//==============================================================================
String System::GetProperty(const String& name)
{
	QC_AUTO_LOCK(FastMutex, SystemMutex);
	const PropertyMap::const_iterator i = SystemProperties.find(name);
	if(i != SystemProperties.end())
		return (*i).second;
	else
		return String();
}

//==============================================================================
// System::SetProperty
//
/**
   Creates a system property with the specified name and assigns it the 
   supplied value.

   @param name the name of the property
   @param value the required value of the property

   @mtsafe
*/
//==============================================================================
void System::SetProperty(const String& name, const String& value)
{
	QC_AUTO_LOCK(FastMutex, SystemMutex);
	SystemProperties[name] = value;
}

//==============================================================================
// System::SetPropertyLong
//
/**
   Creates a system property with the specified name and assigns it the 
   supplied value.

   @param name the name of the property
   @param value the required value of the property

   @mtsafe
*/
//==============================================================================
void System::SetPropertyLong(const String& name, long value)
{
	SetProperty(name, NumUtils::ToString(value));
}

//==============================================================================
// System::SetPropertyBool
//
/**
   Creates a system property with the specified name and assigns it the 
   supplied value.

   @param name the name of the property
   @param bSet the required value of the property

   @mtsafe
*/
//==============================================================================
void System::SetPropertyBool(const String& name, bool bSet)
{
	SetPropertyLong(name, (long)bSet);
}

//==============================================================================
// System::GetPropertyLong
//
/**
   Returns the value of the named property as a long integer.

   @returns the property as a long integer if it has been set; returns
            @c defaultValue otherwise.
   @param name the name of the property
   @param defaultValue the default value

   @mtsafe
*/
//==============================================================================
long System::GetPropertyLong(const String& name, long defaultValue)
{
	QC_AUTO_LOCK(FastMutex, SystemMutex);
	const PropertyMap::const_iterator i = SystemProperties.find(name);
	if(i != SystemProperties.end())
		return NumUtils::ToLong((*i).second);
	else
		return defaultValue;
}

//==============================================================================
// System::GetPropertyBool
//
/**
   Returns the value of the named property as a boolean value.

   @returns @c true if the property is set to "true" or "1"; false if it is
   set to any other value.  If the property is not set the @c bDefault value
   is returned.

   @param name the name of the property
   @param bDefault the default value

   @mtsafe
*/
//==============================================================================
bool System::GetPropertyBool(const String& name, bool bDefault)
{
	QC_AUTO_LOCK(FastMutex, SystemMutex);
	const PropertyMap::const_iterator i = SystemProperties.find(name);
	if(i != SystemProperties.end())
	{
		const String& value = (*i).second;
		return value == QC_T("1") || StringUtils::CompareNoCase(QC_T("true"), value) == 0;
	}
	else
		return bDefault;
}

QC_BASE_NAMESPACE_END

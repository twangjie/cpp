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
// Class CodeConverterFactory
/**
	@class qc::cvt::CodeConverterFactory
	
	@brief A factory class for creating instances of a CodeConverter
	for a given encoding name.

    @QuickCPP is supplied with CodeConverters that can understand
	a broad range of different encodings.  The CodeConverterFactory base class
	is aware of the supplied encodings, and will return an appropriate
	CodeConverter instance for every encoding name that it recognizes.

    Applications can extend @QuickCPP by supplementing their own encodings.  To
	achieve this the application must create a new factory class @a derived
	from CodeConverterFactory and set an instance of the @a derived class as
	the global CodeConverterFactory by calling SetInstance().
*/
//==============================================================================

#include "CodeConverterFactory.h"
#include "ASCIIConverter.h"
#include "UTF8Converter.h"
#include "UTF16Converter.h"
#include "ISO88591Converter.h"
#include "ASCII8BitConverter.h"

#include "QcCore/base/FastMutex.h"
#include "QcCore/base/ObjectManager.h"
#include "QcCore/base/System.h"
#include "QcCore/base/StringUtils.h"

QC_CVT_NAMESPACE_BEGIN

//==================================================================
// Multi-threaded locking strategy
//
// Update access to static variables is mutex protected, but to 
// minimise the runtime cost, read access is not protected.  This
// gives an exposure to the so-called "relaxed" memory model that
// exists on some multi-processor machines.  We minimise this
// exposure by declaring the static variables as 'volatile'.
//==================================================================

#ifdef QC_MT
	FastMutex CodeConverterFactoryMutex;
#endif //QC_MT


CodeConverterFactory* QC_MT_VOLATILE CodeConverterFactory::s_pInstance = NULL;

#include "IBM-850.tab"
#include "windows-1250.tab"
#include "windows-1251.tab"
#include "windows-1252.tab"
#include "windows-1253.tab"
#include "windows-1254.tab"
#include "windows-1255.tab"
#include "windows-1256.tab"
#include "windows-1257.tab"
#include "windows-1258.tab"
#include "ISO-8859-2.tab"
#include "ISO-8859-3.tab"
#include "ISO-8859-4.tab"
#include "ISO-8859-5.tab"
#include "ISO-8859-6.tab"
#include "ISO-8859-7.tab"
#include "ISO-8859-8.tab"
#include "ISO-8859-9.tab"
#include "ISO-8859-10.tab"
#include "ISO-8859-13.tab"
#include "ISO-8859-14.tab"
#include "ISO-8859-15.tab"
#include "ISO-8859-16.tab"


//==============================================================================
// CodeConverterFactory::GetInstance
//
/**
   Returns the global CodeConverterFactory.

   If a factory has not been registered by the application,
   a default CodeConverterFactory is created by the system and returned.

   @sa SetInstance()
   @mtsafe
*/
//==============================================================================
CodeConverterFactory& CodeConverterFactory::GetInstance()
{
	//==================================================================
	// Multi-threaded locking strategy
	//
	// This uses the "double-checked locking pattern" (Schmidt 1996)
	// with a volatile storage member to minimise race conditions due
	// to the so-called "relaxed memory model"..
	//==================================================================
	if(s_pInstance == NULL)
	{
		QC_AUTO_LOCK(FastMutex, CodeConverterFactoryMutex);
		if(s_pInstance == NULL)
		{
			s_pInstance = new CodeConverterFactory;
			// registerObject() will increment the new object's ref count
			System::GetObjectManager().registerObject(s_pInstance);
		}
	}
	return *s_pInstance;
}

//==============================================================================
// CodeConverterFactory::SetInstance
//
/**
   Sets the global CodeConverterFactory instance.  This global
   factory is used by the InputStreamReader and OutputStreamWriter
   classes when they require a CodeConverter for a named encoding.

   Like other @QuickCPP global objects, CodeConverterFactory is a QCObject.
   This enables the passed object to be registered with the system's
   ObjectManager, thereby freeing the application from having to manage the lifetime
   of the factory object.  In other words, the application does not need to maintain
   the object's reference-count once it has been registered.

   A typical application may do the following:-

   @code
   CodeConverterFactory::SetInstance(new MyCodeConverterFactory);
   @endcode

   @sa GetInstance()
   @mtsafe
*/
//==============================================================================
void CodeConverterFactory::SetInstance(CodeConverterFactory* pFactory)
{
	CodeConverterFactory* pExisting;

	//
	// Register the new factory with the ObjectManager.
	// This can be done before the mutex is locked.
	//
	if(pFactory) 
	{
		System::GetObjectManager().registerObject(pFactory);
	}

	//
	// Create a limited scope for the mutex lock
	//
	{
		QC_AUTO_LOCK(FastMutex, CodeConverterFactoryMutex);
		pExisting = s_pInstance;
		s_pInstance = pFactory;
		// Note: To ensure that the reference count is correctly managed
		// we must register the object while the mutex lock is still held
	}

	// The existing object can bu unregistered at leisure, ie not under control
	// of the mutex lock
	if(pExisting) 
	{
		System::GetObjectManager().unregisterObject(pExisting);
	}
}

//==============================================================================
// CodeConverterFactory::getConverter
//
/**
   Returns a new instance of a CodeConverter
   that can encode and decode the requested encoding.

   @param encoding the name of the encoding which is used to 
          select a matching CodeConverter without regard to case
   @mtsafe
*/
//==============================================================================
AutoPtr<CodeConverter> CodeConverterFactory::getConverter(const String& encoding) const
{
	//
	// First check if the requested encoding matches one of the custom encoders.
	//
	if(StringUtils::CompareNoCase(encoding, QC_T("ISO-8859-1")) == 0)
		return new ISO88591Converter;
	else if(StringUtils::CompareNoCase(encoding, QC_T("UTF-8")) == 0)
		return new UTF8Converter;
	else if(StringUtils::CompareNoCase(encoding, QC_T("US-ASCII")) == 0)
		return new ASCIIConverter;
	else if(StringUtils::CompareNoCase(encoding, QC_T("ASCII")) == 0)
		return new ASCIIConverter;
	else if(StringUtils::CompareNoCase(encoding, QC_T("UTF-16BE")) == 0)
		return new UTF16Converter(UTF16Converter::big_endian);
	else if(StringUtils::CompareNoCase(encoding, QC_T("UTF-16LE")) == 0)
		return new UTF16Converter(UTF16Converter::little_endian);
	else if(StringUtils::CompareNoCase(encoding, QC_T("UTF-16")) == 0)
		return new UTF16Converter();

	//
	// None of the custom encodings match.  What about the ASCII-boys?
	// These all share the ASCII encodings up to 0x7F, but above that they
	// are all different.
	//
	struct EncodingTableEntry
	{
		typedef const unsigned short ExtAsciiTab[128];
		const CharType* name;
		ExtAsciiTab* table;
	};

	EncodingTableEntry encodingTable[] = 
	{
		{QC_T("windows-1250"), &Encode_CP1250_Table},
		{QC_T("windows-1251"), &Encode_CP1251_Table},
		{QC_T("windows-1252"), &Encode_CP1252_Table},
		{QC_T("windows-1253"), &Encode_CP1253_Table},
		{QC_T("windows-1254"), &Encode_CP1254_Table},
		{QC_T("windows-1255"), &Encode_CP1255_Table},
		{QC_T("windows-1256"), &Encode_CP1256_Table},
		{QC_T("windows-1257"), &Encode_CP1257_Table},
		{QC_T("windows-1258"), &Encode_CP1258_Table},
		{QC_T("ISO-8859-2"),   &Encode_8859_2_Table},
		{QC_T("ISO-8859-3"),   &Encode_8859_3_Table},
		{QC_T("ISO-8859-4"),   &Encode_8859_4_Table},
		{QC_T("ISO-8859-5"),   &Encode_8859_5_Table},
		{QC_T("ISO-8859-6"),   &Encode_8859_6_Table},
		{QC_T("ISO-8859-7"),   &Encode_8859_7_Table},
		{QC_T("ISO-8859-8"),   &Encode_8859_8_Table},
		{QC_T("ISO-8859-9"),   &Encode_8859_9_Table},
		{QC_T("ISO-8859-10"),  &Encode_8859_10_Table},
		{QC_T("ISO-8859-13"),  &Encode_8859_13_Table},
		{QC_T("ISO-8859-14"),  &Encode_8859_14_Table},
		{QC_T("ISO-8859-15"),  &Encode_8859_15_Table},
		{QC_T("IBM850"),       &Encode_IBM_850_Table},
		{0, 0}
	};

	size_t i=0;
	while(true)
	{
		const EncodingTableEntry& entry = encodingTable[i];
		if(entry.name == 0)
		{
			break;
		}
		else if(StringUtils::CompareNoCase(encoding, entry.name) == 0)
		{
			return new ASCII8BitConverter(entry.name, *entry.table);
		}
		i++;
	}

	return 0;
}

//==============================================================================
// CodeConverterFactory::getDefaultConverter
//
/**
   Returns a new instance of a CodeConverter
   that is suitable for the target platform.

   The returned CodeConverter is not guaranteed to perform one particular
   encoding.  
   
   In the current implementation a CodeConverter that encodes and decodes
   ISO-8859-1 (Latin1) is always returned.

   @mtsafe
*/
//==============================================================================
AutoPtr<CodeConverter> CodeConverterFactory::getDefaultConverter() const
{
	//return new UTF8Converter;
	//return new ASCII8BitConverter(QC_T("IBM850"), Encode_IBM_850_Table);
	return new ISO88591Converter;
}

QC_CVT_NAMESPACE_END

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
// Class XMLReaderFactory
/**
	@class qc::sax::XMLReaderFactory
	
	@brief Factory class for creating an XMLReader.
	
	<p>This class contains a static function for creating an XMLReader: CreateXMLReader().
	This function delegates the create request to the registered XMLReaderFactory instance
	thereby enabling the application to register a factory object and, in doing so,
	control the kind of XMLReader produced.</p>
	<p>If an instance of XMLReaderFactory is not registered by the application, an
	instance is created automatically the first time CreateXMLReader() or GetInstance()
	is called.</p>
	
    @code
    AutoPtr<XMLReader> rpReader = XMLReaderFactory::CreateXMLReader();
    @endcode
*/
//==============================================================================

#include "XMLReaderFactory.h"
#include "SAXParser.h"

#include "QcCore/base/System.h"
#include "QcCore/base/ObjectManager.h"
#include "QcCore/base/FastMutex.h"

QC_SAX_NAMESPACE_BEGIN

//==================================================================
// Multi-threaded locking strategy
//
// XMLReaderFactory is a global object, created on demand.
// To ensure that resources are not created by multiple
// concurrent threads, the pointers are protected using a
// single static mutex.
//
// Update access to static variables is mutex protected, but to 
// minimise the runtime cost, read access is not protected.  This
// gives an exposure to the so-called "relaxed" memory model that
// exists on some multi-processor machines.  We minimise this
// exposure by declaring the static variables as 'volatile'.
//==================================================================
#ifdef QC_MT
	FastMutex XMLReaderFactoryMutex;
#endif //QC_MT


XMLReaderFactory* QC_MT_VOLATILE XMLReaderFactory::s_pInstance = NULL; // mutex protected

//==============================================================================
// XMLReaderFactory::GetInstance
//
/**
   Returns the currently registered XMLReaderFactory.  If a factory has not
   yet been registered, this method creates a default XMLReaderFactory and
   registers that.
   @mtsafe
*/
//==============================================================================
AutoPtr<XMLReaderFactory> XMLReaderFactory::GetInstance()
{
	if(s_pInstance == NULL)
	{
		SetInstance(new XMLReaderFactory);
	}
	QC_DBG_ASSERT(s_pInstance!=0);
	return s_pInstance;
}

//==============================================================================
// XMLReaderFactory::SetInstance
//
/**
   Allows an application to register a custom XMLReaderFactory object
   which will be used to create instances of XMLReader.

   Like other @QuickCPP global objects, XMLReaderFactory is a ManagedObject.
   This enables the passed object to be registered with the system's
   ObjectManager, thereby freeing the application from having to manage the lifetime
   of the factory object.  In other words, the application does not need to maintain
   the object's reference-count once it has been registered.

   A typical application may do the following:-

   @code
   XMLReaderFactory::SetInstance(new MyXMLReaderFactory);
   @endcode

   @param pFactory a pointer to an instance of XMLReaderFactory or null
          to unregister the currently registered factory.
   @mtsafe
*/
//==============================================================================
void XMLReaderFactory::SetInstance(XMLReaderFactory* pFactory)
{
	//==================================================================
	// Multi-threaded locking strategy
	//
	// See top of file.
	//==================================================================

	if(pFactory)
	{
		System::GetObjectManager().registerObject(pFactory);
	}

	XMLReaderFactory* pExisting;
	// create a scope for the mutex lock
	{
		QC_AUTO_LOCK(FastMutex, XMLReaderFactoryMutex);
		pExisting = s_pInstance;
		s_pInstance = pFactory;
	}

	if(pExisting) 
	{
		System::GetObjectManager().unregisterObject(pExisting);
	}
}

//==============================================================================
// XMLReaderFactory::CreateXMLReader
//
/**
   Static function to create an instance of an XMLReader.

   This static function simply delegates the request to the currently registered
   XMLReaderFactory object's doCreate() method.  If the application has not
   registered an instance of XMLReaderFactory, a default factory is registered
   automatically.

   @mtsafe
*/
//==============================================================================
AutoPtr<XMLReader> XMLReaderFactory::CreateXMLReader()
{
	return GetInstance()->doCreate();
}

//==============================================================================
// XMLReaderFactory::doCreate
//
/**
   Creates an instance of XMLReader.

   An application may register a class @a derived from XMLReaderFactory
   to create whatever derivative of XMLReader it requires.
*/
//==============================================================================
AutoPtr<XMLReader> XMLReaderFactory::doCreate() const
{
	return new SAXParser;
}

QC_SAX_NAMESPACE_END

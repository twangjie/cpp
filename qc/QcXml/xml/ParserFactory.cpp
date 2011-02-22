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

#include "ParserFactory.h"
#include "XMLInputSource.h"
#include "ParserImpl.h"
#include "QcCore/base/System.h"
#include "QcCore/base/ObjectManager.h"
#include "QcCore/base/FastMutex.h"

QC_XML_NAMESPACE_BEGIN

//==================================================================
// Multi-threaded locking strategy
//
// ParserFactory is a singleton object, created on demand.
// To ensure that singleton resources are not created by multiple
// concurrent threads, the pointers are protexted using a
// single static mutex.
//
// To minimize the time that the mutex is held, we do
// not lock the mutex for the creation of a singleton object,
// only when a complete object has been created and we need to swap
// the address into our static pointer variable.
//==================================================================
#ifdef QC_MT
	FastMutex ParserFactoryMutex;
#endif //QC_MT

ParserFactory* QC_MT_VOLATILE ParserFactory::s_pInstance = 0; // mutex protected

ParserFactory::ParserFactory()
{
}

ParserFactory::~ParserFactory()
{
}

//==============================================================================
// ParserFactory::GetInstance
//
//==============================================================================
AutoPtr<ParserFactory> ParserFactory::GetInstance()
{
	if(s_pInstance == NULL)
	{
		SetInstance(new ParserFactory);
	}
	QC_DBG_ASSERT(s_pInstance!=0);
	return s_pInstance;
}

//==============================================================================
// ParserFactory::SetInstance
//
//==============================================================================
void ParserFactory::SetInstance(ParserFactory* pFactory)
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

	ParserFactory* pExisting;

	// create a scope for the mutex lock
	{
		QC_AUTO_LOCK(FastMutex, ParserFactoryMutex);
		pExisting = s_pInstance;
		s_pInstance = pFactory;
	}

	if(pExisting) 
	{
		System::GetObjectManager().unregisterObject(pExisting);
	}
}

//==============================================================================
// ParserFactory::CreateXMLParser
//
/**
   Creates a Parser by delegating the request to the currently registered
   ParserFactory object.
*/
//==============================================================================
AutoPtr<Parser> ParserFactory::CreateXMLParser()
{
	return GetInstance()->doCreate();
}

//==============================================================================
// ParserFactory::doCreate
//
// Return an instance of an XML Parser
//==============================================================================
AutoPtr<Parser> ParserFactory::doCreate() const
{
	return new ParserImpl();
}

QC_XML_NAMESPACE_END


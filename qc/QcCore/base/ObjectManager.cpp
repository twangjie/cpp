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
// Class ObjectManager
/**
	@class qc::ObjectManager
	
	@brief Manages the lifetime of ManagedObject instances
	that need to exist for the duration of the application. 
	
	@QuickCPP contains several classes that use global objects which can be
	created by application code or internally within the
	library.  To enable these objects (and others) to be deleted at system termination,
	they are registered with a singleton ObjectManager which is responsible for
	keeping a reference to them until the process terminates.

    The task of deleting objects at process termination is seldom strictly
	required because the underlying operating system will free any resources held
	by the process upon termination.  However, it is sometimes useful in a development
	environment to help to detect memory leaks in long-running processes.

	@sa System::GetObjectManager()
*/
//==============================================================================

#include "ObjectManager.h"
#include "ManagedObject.h"

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// ObjectManager::registerObject
//
/**
   Registers an instance of ManagedObject with this ObjectManager.

   The ObjectManager adds a reference to the passed object to its collection
   of object references.  In so doing, the reference count of the passed object 
   is incremented to prevent it from being destroyed until system termination
   or until the object is removed by calling unregisterObject().

   @sa unregisterObject()
   @param pObject pointer to a ManagedObject whose lifetime needs to be managed
          by the ObjectManager
*/
//==============================================================================
void ObjectManager::registerObject(ManagedObject* pObject)
{
	QC_AUTO_LOCK(FastMutex, m_mutex);
	m_list.push_back(pObject);
}

//==============================================================================
// ObjectManager::unregisterObject
//
/**
   Removes the reference to @c pObject from the managed collection of object
   references.
   
   It is not an error if the object designated by @c pObject does not exist
   in the managed collection.

   @param pObject pointer to a ManagedObject which will be removed from the
          managed collection of object references

*/
//==============================================================================
void ObjectManager::unregisterObject(ManagedObject* pObject)
{
	QC_AUTO_LOCK(FastMutex, m_mutex);
	m_list.remove(pObject);
}

//==============================================================================
// ObjectManager::unregisterAllObjects
//
// Private function used by System::Terminate to sweep through and unregister
// all objects before this ObjectManager is destroyed.
//==============================================================================
void ObjectManager::unregisterAllObjects()
{
	//
	// Note: when an object is unregistered, it's destrictor may be called
	// which could possibly choose to unregister another object.  This is
	// not recommended behaviour (the object will have been unregistered
	// anyway due to system termination), but to prevent a hang on the
	// mutex lock if this situation were to occur, a copy of the registered
	// objects list is made while under mutex control, the mutex is then
	// released and the copied list allowed to go out of scope.

#ifdef QC_MT

	FastMutex::Lock scoped_lock(m_mutex);
	ObjectList copy = m_list;
	m_list.clear();
	scoped_lock.unlock();

#else  // !QC_MT

	m_list.clear();

#endif //QC_MT
}

QC_BASE_NAMESPACE_END

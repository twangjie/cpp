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
// Class: ResourceDescriptor
/**
	@class qc::io::ResourceDescriptor
	
	@brief An abstract base class used to connect @QuickCPP with operating system
	resources such as open files and network sockets.

	A ResourceDescriptor represents an open handle.  It @a derives from
	ManagedObject and uses the standard @QuickCPP reference-counting scheme.  By using
	ResourceDescriptors, @QuickCPP is able to share handles between multiple
	objects, preventing the underlying resource from being closed until
	all references to the ResourceDescriptor have been destroyed.  This
	is often more efficient and flexible than duplicating operating system 
	handles for the same purpose.

	ResourceDescriptor declares a private copy constructor and assignment
	operator to prevent the copying of instance objects.
*/
//==============================================================================

#include "ResourceDescriptor.h"

QC_IO_NAMESPACE_BEGIN

//==============================================================================
// ResourceDescriptor::ResourceDescriptor
//
/**
   Constructs a ResourceDescriptor, specifying whether the related operating
   system resource should be closed when this ResourceDescriptor is destroyed.

   @param bAutoClose true if the resource should be closed when this
          ResourceDescriptor is destroyed; false otherwise
*/
//==============================================================================
ResourceDescriptor::ResourceDescriptor(bool bAutoClose) :
	m_bAutoClose(bAutoClose)
{
}

//==============================================================================
// ResourceDescriptor::~ResourceDescriptor
//
/**
   Destructor.  This method does nothing but @a derived classes are 
   expected to call getAutoClose() and close the related resource if
   it returns @c true.
   
   To indicate that they have performed the required task,
   all @a derived class destructors should call setAutoClose(false)
   before exiting.
*/
//==============================================================================
ResourceDescriptor::~ResourceDescriptor()
{
	// Derived classes should set m_bAutoClose to false in their
	// destructors.  Failure to do so implies that they have not
	// honoured the committment to close the managed resource.
	QC_DBG_ASSERT(!m_bAutoClose);
}

//==============================================================================
// ResourceDescriptor::getAutoClose
//
/**
   Returns a boolean value to indicate whether the underlying resource will
   be closed when the reference count for this ResourceDescriptor reaches zero.

   @returns true if the resource will be closed automatically; false otherwise
*/
//==============================================================================
bool ResourceDescriptor::getAutoClose() const
{
	return m_bAutoClose;
}

//==============================================================================
// ResourceDescriptor::setAutoClose
//
/**
   Sets the value of the auto close indicator.

   @sa getAutoClose()
*/
//==============================================================================
void ResourceDescriptor::setAutoClose(bool bAutoClose)
{
	m_bAutoClose = bAutoClose;
}

#ifdef QC_DOCUMENTATION_ONLY

//==============================================================================
// ResourceDescriptor::toString
//
/**
   Returns a string representation of this ResourceDescriptor.  
   
   @QuickCPP uses the string representation to identify the resource descriptor
   when tracing via the Tracer class.
   @since 1.3
*/
//==============================================================================
String ResourceDescriptor::toString() const;

#endif //QC_DOCUMENTATION_ONLY

QC_IO_NAMESPACE_END

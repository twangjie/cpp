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
// Class: FileDescriptor
/**
	@class qc::io::FileDescriptor
	
	@brief An abstract base class representing open file handles.

    It @a derives from ManagedObject and uses the standard @QuickCPP reference-counting
	scheme.  By using ResourceDescriptors, @QuickCPP is able to share file handles
	between multiple objects, preventing the file from being closed until
	all references to the FileDescriptor have been removed.

    A FileDescriptor is associated with a FileSystem object when it is created.
	When the file needs to be closed, the FileSystem associated with the file
	performs the action.
*/
//==============================================================================

#include "FileDescriptor.h"
#include "FileSystem.h"

#include "QcCore/base/NullPointerException.h"

QC_IO_NAMESPACE_BEGIN

//==============================================================================
// FileDescriptor::FileDescriptor
//
/**
   Constructs a FileDescriptor.

   @param pFS the FileSystem associated with the open file designated by this
          FileDescriptor.
   @param bAutoClose true if the file should be closed when this FileDescriptor
          is destroyed
   @throws NullPointerException if @c pFS is null
*/
//==============================================================================
FileDescriptor::FileDescriptor(const FileSystem* pFS, bool bAutoClose) :
	ResourceDescriptor(bAutoClose),
	m_rpFS(const_cast<FileSystem*>(pFS))
{
	if(!pFS) throw NullPointerException();
}

//==============================================================================
// FileDescriptor::~FileDescriptor
//
/**
   Destructor.  This method does nothing, but @a derived classes are 
   expected to call getAutoClose() and close the related resource if
   it returns @c true.
   
   To indicate that they have performed the required task,
   all @a derived class destructors should call setAutoClose(false)
   before exiting.
*/
//==============================================================================
FileDescriptor::~FileDescriptor()
{
}

//==============================================================================
// FileDescriptor::close
//
/**
   Closes the file.
*/
//==============================================================================
void FileDescriptor::close()
{
	m_rpFS->closeFile(this);
	setAutoClose(false);
}

//==============================================================================
// FileDescriptor::getFileSystem
//
/**
   Returns the FileSystem associated with this FileDescriptor.
*/
//==============================================================================
AutoPtr<FileSystem> FileDescriptor::getFileSystem() const
{
	return m_rpFS;
}

//==============================================================================
// FileDescriptor::Err
//
/**
   Returns a FileDescriptor for the standard Console stream @c 'stderr'.

   FileDescriptors for standard Console streams are not auto-close descriptors
   which means that the stream is (correctly) left open when then the
   returned FileDescriptor is destroyed.
*/
//==============================================================================
AutoPtr<FileDescriptor> FileDescriptor::Err()
{
	return FileSystem::GetFileSystem()->getConsoleFD(FileSystem::Stderr);
}

//==============================================================================
// FileDescriptor::Out
//
/**
   Returns a FileDescriptor for the standard Console stream @c 'stdout'.

   FileDescriptors for standard Console streams are not auto-close descriptors
   which means that the stream is (correctly) left open when then the
   returned FileDescriptor is destroyed.
*/
//==============================================================================
AutoPtr<FileDescriptor> FileDescriptor::Out()
{
	return FileSystem::GetFileSystem()->getConsoleFD(FileSystem::Stdout);
}

//==============================================================================
// FileDescriptor::In
//
/**
   Returns a FileDescriptor for the standard Console stream @c 'stdin'.

   FileDescriptors for standard Console streams are not auto-close descriptors
   which means that the stream is (correctly) left open when then the
   returned FileDescriptor is destroyed.
*/
//==============================================================================
AutoPtr<FileDescriptor> FileDescriptor::In()
{
	return FileSystem::GetFileSystem()->getConsoleFD(FileSystem::Stdin);
}

QC_IO_NAMESPACE_END

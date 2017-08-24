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
// Class: FileInputStream
/**
	@class qc::io::FileInputStream
	
	@brief An InputStream that uses the contents of a file as its source.

    When a FileInputStream is created, a connection is established with an
	open file in the file system.  The open file is represented by a
	FileDescriptor which ensures the file is closed when 
	the FileInputStream is destroyed.
*/
//==============================================================================

#include "FileInputStream.h"
#include "File.h"
#include "FileSystem.h"
#include "FileDescriptor.h"
#include "IOException.h"

#include "QcCore/base/NullPointerException.h"
#include "QcCore/base/SystemUtils.h"

QC_IO_NAMESPACE_BEGIN

//==============================================================================
// FileInputStream::FileInputStream
//
/**
   Constructs a FileInputStream by opening a connection to the file with
   the abstract pathname denoted by @c file.

   @param file the abstract pathname of the file to open
   @throws FileNotFoundException if a file with the specified name does not
           exist on the file system.  
   @throws IOException if the specified file could not be opened.  This includes
           the case where @c file refers to a directory instead of a normal file.
*/
//==============================================================================
FileInputStream::FileInputStream(const File& file)
{
	open(file.getPath());
}

//==============================================================================
// FileInputStream::FileInputStream
//
/**
   Constructs a FileInputStream by opening a connection to the named file @c name.

   @param name the name of the file to open
   @throws FileNotFoundException if a file with the specified name does not
           exist on the file system.  
   @throws IOException if the specified file name could not be opened.  This includes
           the case where @c name refers to a directory instead of a normal file.
*/
//==============================================================================
FileInputStream::FileInputStream(const String& name)
{
	open(name);
}

//==============================================================================
// FileInputStream::FileInputStream
//
/**
   Constructs a FileInputStream and connects it with an open file
   denoted by the FileDescriptor @c pFD.

   @param pFD the FileDescriptor to connect to this FileInputStream
   @throws NullPointerException if @c pFD is null.
*/
//==============================================================================
FileInputStream::FileInputStream(FileDescriptor* pFD) : 
	m_rpFD(pFD)
{
	if(!pFD) throw NullPointerException();
}

//==============================================================================
// FileInputStream::close
//
//==============================================================================
void FileInputStream::close()
{
	if(m_rpFD)
	{
		// must call close on the FD rather than the FileSystem
		// in case the FD has AutoClose enabled
		m_rpFD->close();
		m_rpFD.release();
	}
}

//==============================================================================
// FileInputStream::read
//
//==============================================================================
long FileInputStream::read(Byte* pBuffer, size_t bufLen)
{
	SystemUtils::TestBufferIsValid(pBuffer, bufLen);
	if(!m_rpFD) throw IOException(QC_T("stream is closed"));

	size_t bytesRead = m_rpFD->getFileSystem()->readFile(m_rpFD.get(), pBuffer, bufLen);

	if(bytesRead == 0)
	{
		return EndOfFile;
	}
	return bytesRead;
}

//==============================================================================
// FileInputStream::getFD
//
/**
   Returns a FileDescriptor for the open file
   connected to this FileInputStream.  A null AutoPtr is returned
   if this FileInputStream has been closed.
*/
//==============================================================================
AutoPtr<FileDescriptor> FileInputStream::getFD() const
{
	return m_rpFD;
}

//==============================================================================
// FileInputStream::open
//
// Private helper function.
//==============================================================================
void FileInputStream::open(const String& fileName) 
{
	if(fileName.empty())
		throw IOException(QC_T("empty filename"));
	else if(FileSystem::GetFileSystem()->getFileAttributeFlags(fileName) & FileSystem::Directory)
		throw IOException(fileName + QC_T(" is a directory"));

	m_rpFD = 
		FileSystem::GetFileSystem()->openFile(fileName,
		                                      FileSystem::ReadAccess,
		                                      FileSystem::OpenExisting,
		                                      0);
}

QC_IO_NAMESPACE_END

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
// Class: FileOutputStream
/**
	@class qc::io::FileOutputStream
	
	@brief An OutputStream that writes bytes to a file.

    When a FileOutputStream is created, a connection is established with an
	open file in the file system.  The open file is represented internally 
	using a FileDescriptor which ensures that the file is closed when 
	the FileOutputStream is destroyed.
*/
//==============================================================================

#include "FileOutputStream.h"
#include "File.h"
#include "FileNotFoundException.h"
#include "FileSystem.h"
#include "IOException.h"

#include "QcCore/base/NullPointerException.h"

QC_IO_NAMESPACE_BEGIN

//==============================================================================
// FileOutputStream::FileOutputStream
//
/**
   Constructs a FileOutputStream by opening a connection to the file with
   the abstract pathname denoted by @c file.

   If a file with the abstract pathname already exists then it is truncated
   and its contents discarded.

   @param file the abstract pathname of the file to open
   @throws IOException if the specified file could not be opened.  This includes
           the case where @c file refers to a directory instead of a normal file.
*/
//==============================================================================
FileOutputStream::FileOutputStream(const File& file)
{
	open(file.getPath(), false /*bAppend*/);
}

//==============================================================================
// FileOutputStream::FileOutputStream
//
/**
   Constructs a FileOutputStream by opening a connection to the named file @c name.

   If the file already exists its contents are discarded.

   @param name the name of the file to open
   @throws IOException if the specified file name could not be opened.  This includes
           the case where @c name refers to a directory instead of a normal file.
*/
//==============================================================================
FileOutputStream::FileOutputStream(const String& name)
{
	open(name, false /*bAppend*/);
}

//==============================================================================
// FileOutputStream::FileOutputStream
//
/**
   Constructs a FileOutputStream by opening a connection to the named file @c name.

   @param name the name of the file to open
   @param bAppend @c true if the contents of an existing file should be kept; @c false
          if the file should be truncated
   @throws IOException if the specified file name could not be opened.  This includes
           the case where @c name refers to a directory instead of a normal file.
*/
//==============================================================================
FileOutputStream::FileOutputStream(const String& name, bool bAppend)
{
	open(name, bAppend);
}

//==============================================================================
// FileOutputStream::FileOutputStream
//
/**
   Constructs a FileInputStream and connects it with an open file
   denoted by the FileDescriptor @c pFD.

   @param pFD the FileDescriptor to connect to this FileInputStream
   @throws NullPointerException if @c pFD is null.
*/
//==============================================================================
FileOutputStream::FileOutputStream(FileDescriptor* pFD) : 
	m_rpFD(pFD)
{
	if(!pFD) throw NullPointerException();
}

//==============================================================================
// FileOutputStream::close
//
/**
   Any buffered data is written to the output file before it is closed
   and any system resources associated with the stream are released.  

   Once an OutputStream is closed further calls to write(), flush() or
   flushBuffers() will result in an IOException being thrown. Further calls
   to close() are legal but have no effect.
   
   @throws IOException if an I/O error occurs.
*/
//==============================================================================
void FileOutputStream::close()
{
	if(m_rpFD)
	{
		m_rpFD->close();
		m_rpFD.release();
	}
}

//==============================================================================
// FileOutputStream::write
//
//==============================================================================
void FileOutputStream::write(const Byte* pBuffer, size_t bufLen)
{
	if(m_rpFD)
	{
		if(bufLen > 0)
		{
			m_rpFD->getFileSystem()->writeFile(m_rpFD.get(), pBuffer, bufLen);
		}
	}
	else
	{
		throw IOException(QC_T("stream closed"));
	}
}

//==============================================================================
// FileOutputStream::getFD
//
/**
   Returns a reference to the FileDescriptor for the open file
   connected to this FileOutputStream.
*/
//==============================================================================
AutoPtr<FileDescriptor> FileOutputStream::getFD() const
{
	return m_rpFD;
}

//==============================================================================
// FileOutputStream::open
//
// Open (and optionally create) the file associated with this OutputStream.
//==============================================================================
void FileOutputStream::open(const String& fileName, bool bAppend) 
{
	int accessFlags = FileSystem::WriteAccess;
	FileSystem::CreationDisp disp = bAppend ? FileSystem::OpenCreateAppend
	                                        : FileSystem::OpenCreateTruncate;
	
	m_rpFD = 
		FileSystem::GetFileSystem()->openFile(fileName,
		                                      accessFlags,
		                                      disp,
		                                      0);
}

QC_IO_NAMESPACE_END

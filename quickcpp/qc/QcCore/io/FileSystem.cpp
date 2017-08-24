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
// Class: FileSystem
//
/**
	@class qc::io::FileSystem
	
	@brief An abstract base class representing a file system.  

    <b>This documentation is provided to aid installations that wish to implement
	their own customized FileSystem.  FileSystem methods
	should not normally be called directly, use the methods on the File class
	instead.</b>

    @QuickCPP treats file pathnames and handles as abstract data types which
	only have meaning when combined with a FileSystem object.  
	
	A FileSystem @a knows how pathnames are constructed and how relative pathnames can
	be combined to create an absolute pathname.  A FileSystem also @a knows how
	to perform file i/o, with the consequence that all @QuickCPP file i/o is delegated
	to a FileSystem object.

    There can be more than one FileSystem object, each one representing
	different file systems supported by the target platform.  However, there 
	can be only one @a global FileSystem.  The @a global FileSystem is used
	by most @QuickCPP classes that have to resolve or open files using abstract pathnames.

	The @a global FileSystem object can be set by applications by use of the
	SetFileSystem() method.

    @QuickCPP provides concrete FileSystem implementations that are adequate for
	most circumstances.
*/
//==============================================================================

#include "FileSystem.h"
#include "File.h"
#include "FileDescriptor.h"

#if defined(WIN32)
#	include "Win32FileSystem.h"
#else 
#	include "PosixFileSystem.h"
#endif

#include "QcCore/base/ObjectManager.h"
#include "QcCore/base/FastMutex.h"
#include "QcCore/base/System.h"

#include <algorithm>

QC_IO_NAMESPACE_BEGIN

//==================================================================
// Multi-threaded locking strategy
//
// There may be more than one FileSystem object in the system, but
// there is only one default FileSystem returned from GetFileSystem().
//
// Update access to static variables is mutex protected, but to 
// minimise the runtime cost, read access is not protected.  This
// gives an exposure to the so-called "relaxed" memory model that
// exists on some multi-processor machines.  We minimise this
// exposure by declaring the static variables as 'volatile'.
//==================================================================
#ifdef QC_MT
	FastMutex FileSystemMutex;
#endif //QC_MT

FileSystem* QC_MT_VOLATILE FileSystem::s_pFileSystem = 0;

//==============================================================================
// FileSystem::SetFileSystem
//
/**
   Sets the @a global FileSystem object.  The @a global FileSystem
   is used to open files and resolve filenames.

   To ensure that the passed object exists for as long as the application needs
   it, the FileSystem object is registered with the system's ObjectManager
   which holds a (counted) reference to it until system termination.

   @sa GetFileSystem()
   @mtsafe
*/
//==============================================================================
void FileSystem::SetFileSystem(FileSystem* pFileSystem)
{
	//==================================================================
	// Multi-threaded locking strategy
	//
	// See top of file.
	//==================================================================

	if(pFileSystem)
	{
		System::GetObjectManager().registerObject(pFileSystem);
	}

	FileSystem* pExisting;

	// create a scope for the mutex lock
	{
		QC_AUTO_LOCK(FastMutex, FileSystemMutex);
		pExisting = s_pFileSystem;
		s_pFileSystem = pFileSystem;
	}

	if(pExisting) 
	{
		System::GetObjectManager().unregisterObject(pExisting);
	}
}

//==============================================================================
// FileSystem::GetFileSystem
//
/**
   Returns a reference to the @a global FileSystem object.  If a @a global
   FileSystem has not yet been registered, a default FileSystem is created.  This
   is platform-dependent and is suitable for most applications.

   @sa SetFileSystem()
*/
//==============================================================================
AutoPtr<FileSystem> FileSystem::GetFileSystem()
{
	if(!s_pFileSystem)
	{
		SetFileSystem
#if defined(WIN32)
		(new Win32FileSystem);
#else 
		(new PosixFileSystem);
#endif
	}

	return s_pFileSystem;
}

//==============================================================================
// FileSystem::normalize
//
/**
   Converts a pathname into @a normalized form.

   The normalized form uses a platform-dependent filename separator character.
   Contiguous separator characters (following the system-dependent prefix)
   are replaced with a single one and trailing separator characters are removed.
   
   @param path the pathname to normalize
*/
//==============================================================================
String FileSystem::normalize(const String& path) const
{
	if(path.empty())
	{
		return path;
	}
	else
	{
		String ret = path;

		//
		// Replace separator characters with the appropriate type
		//
		const CharType sep = getSeparatorChar();
		const CharType badSep = (sep == '/') ? '\\' : '/';
		std::replace(ret.begin(), ret.end(), badSep, sep);

		// Replace strings of separators with a single separator
		// starting at position 0 for UNIX and 1 for Windows!
		size_t pos = (sep == '\\') ? 1 : 0;
		const CharType doubleSep[] = {sep, sep, 0};
		while(pos != String::npos)
		{
			pos = ret.find(doubleSep, pos);
			if(pos != String::npos)
			{
				ret.erase(pos, 1);
			}
		}

		//
		// Remove terminating separator if any
		//
		if(sep == ret[ret.length()-1] && getPrefixLength(ret) < ret.length())
		{
			ret = ret.erase(ret.length()-1);
		}

		return ret;
	}
}

//==============================================================================
// FileSystem::resolve
//
/**
   Resolves a relative child pathname against a parent pathname to create an
   absolute pathname.

   If @c child is already absolute, a @a normalized form of @c child 
   is returned without attempting to resolve it further.

   @param parent a directory name to resolve the child pathname against.  The
          directory name need not exist in the file system.
   @param child a relative or absolute pathname.  If @c child is a relative
          pathname it is made absolute by resolving it against @c parent

*/
// If the child is absolute, then we do not resolve the child against
// the parent.  This seems to make the most logical sense, even though
// that is not the apparent behaviour of the JDK.
//
// If the parent directory is empty, this is not treated as the current
// working directory, but simply as the empty string.
// This allows this class to be used in URL resolution where the current
// directory has no meaning.
//==============================================================================
String FileSystem::resolve(const String& parent, const String& child) const
{
	String normalizedChild = normalize(child);
	if(isAbsolute(normalizedChild))
	{
		return normalizedChild;
	}
	else
	{
		size_t prefixLen = getPrefixLength(normalizedChild);
		String normalizedParent = normalize(parent);
		const CharType lastChar = normalizedParent[normalizedParent.length()-1];
		if(lastChar == getSeparatorChar())
		{
			return normalizedParent + normalizedChild.substr(prefixLen);
		}
		else
		{
			return normalizedParent + getSeparator() + normalizedChild.substr(prefixLen);
		}
	}
}

//==============================================================================
// FileSystem::getSeparator
//
/**
   Returns the separator character as a String.

   @sa getSeparatorChar()
*/
//==============================================================================
String FileSystem::getSeparator() const
{
	return String(1, getSeparatorChar());
}

//==============================================================================
// FileSystem::canonicalize
//
/**
   Returns a @a canonical form of @c path.

   The canonical form of a file path name gives the shortest possible
   name that can uniquely identify the file.  The canonical form is
   obtained by resolving the @c path and passing the result to
   File::GetCanonicalPath().

   For example, the @a canonical form of the path 
   @c docs/../images/image1.jpg may be
   @c /home/http/images/image1.jpg if @c /home/http/ is the current directory.

   @sa File::GetCanonicalPath()
*/
//==============================================================================
String FileSystem::canonicalize(const String& path) const
{
	return File::GetCanonicalPath(resolve(path));
}

//==============================================================================
// FileSystem::resolve
//
/**
   Resolve a relative pathname.

   In the normal case a relative pathname is resolved against the
   current directory.  On the UNIX platform this is always the case.
   On Windows platforms, a relative pathname is resolved against the current directory
   unless it starts with the separator character, in which case it is resolved
   against the root of the current drive.
*/
//==============================================================================
String FileSystem::resolve(const String& path) const
{
	QC_DBG_ASSERT(!isAbsolute(path));
	return resolve(getCurrentDirectory(), path);
}

#ifdef QC_DOCUMENTATION_ONLY
//=============================================================================
//
// Documentation for pure virtual methods follows:
//
//=============================================================================

//==============================================================================
// FileSystem::getCurrentDirectory
//
/**
   Returns the current directory for the active process.
*/
//==============================================================================
String FileSystem::getCurrentDirectory() const;

//==============================================================================
// FileSystem::getSeparatorChar
//
/**
   Returns the character which is used to separate the parts of a hierarchical
   path name.
*/
//==============================================================================
CharType FileSystem::getSeparatorChar() const;

//==============================================================================
// FileSystem::getPrefixLength
//
/**
   Returns the length of the prefix of an abstract path name.

   See File for a description of abstract pathnames.
*/
//==============================================================================
size_t FileSystem::getPrefixLength(const String& path) const;

//==============================================================================
// FileSystem::isAbsolute
//
/**
   Tests if @c path represents an absolute file name.
   @returns true if @c path is absolute; false otherwise
*/
//==============================================================================
bool FileSystem::isAbsolute(const String& path) const;

//==============================================================================
// FileSystem::isCaseSensitive
//
/**
   Tests if the FileSystem represents files in a case-sensitive manner.

   Unix/Linux systems are usually case-sensitive whereas Windows is not.
*/
//==============================================================================
bool FileSystem::isCaseSensitive() const;

//==============================================================================
// FileSystem::getFileAttributeFlags
//
/**
   Returns an integer used as a bit field, each bit representing a file attribute.
   The bit values are designated by the enum FileSystem::Attribute.
*/
//==============================================================================
int FileSystem::getFileAttributeFlags(const String& path) const;

//==============================================================================
// FileSystem::checkAccess
//
/**
   Tests if file @c path can be opened with the specified @c mode.

   @returns true if the file can be opened with the mode requested; false otherwise
*/
//==============================================================================
bool FileSystem::checkAccess(const String& path, AccessMode mode) const;

//==============================================================================
// FileSystem::getLastModifiedTime
//
/**
   Returns the time the file was last modified.
   @throws FileNotFoundException if the file does not exist
   @throws IOException if an error occurs accessing the file
*/
//==============================================================================
DateTime FileSystem::getLastModifiedTime(const String& path) const;

//==============================================================================
// FileSystem::getLength
//
/**
   Returns the length (in bytes) of the file.
   @throws FileNotFoundException if the file does not exist
   @throws IOException if an error occurs accessing the file
*/
//==============================================================================
size_t FileSystem::getLength(const String& path) const;

//==============================================================================
// FileSystem::openFile
//
/**
   Opens a file and returns a FileDescriptor representing the open file.
   @throws IOException if an error occurs opening the file
*/
//==============================================================================
AutoPtr<FileDescriptor> FileSystem::openFile(const String& path, int accessMode, CreationDisp creationDisp, int attributes) const;

//==============================================================================
// FileSystem::closeFile
//
/**
   Closes the file specified by the FileDescriptor @c pFD.
*/
//==============================================================================
void FileSystem::closeFile(FileDescriptor* pFD) const;

//==============================================================================
// FileSystem::deleteFile
//
/**
   Deletes the named file or directory from the file system.  Directories must
   be empty before they can be deleted.

   @throws FileNotFoundException if the file does not exist
   @throws IOException if an error occurs deleting the file
*/
//==============================================================================
void FileSystem::deleteFile(const String& path) const;

//==============================================================================
// FileSystem::listDirectory
//
/**
   Returns a list of the files contained within a directory.
*/
//==============================================================================
std::list<String> FileSystem::listDirectory(const String& path) const;

//==============================================================================
// FileSystem::createDirectory
//
/**
   Creates a directory.
*/
//==============================================================================
void FileSystem::createDirectory(const String& path) const;

//==============================================================================
// FileSystem::rename
//
/**
   Renames a file.
*/
//==============================================================================
void FileSystem::rename(const String& path1, const String& path2) const;

//==============================================================================
// FileSystem::setLastModifiedTime
//
/**
   Updates the date and time a file was last modified.
*/
//==============================================================================
void FileSystem::setLastModifiedTime(const String& path, const DateTime& time) const;

//==============================================================================
// FileSystem::setReadOnly
//
/**
   Changes a file's access permissions to be 'read only'
*/
//==============================================================================
void FileSystem::setReadOnly(const String& path, bool bReadOnly) const;

//==============================================================================
// FileSystem::getConsoleFD
//
/**
   Returns a FileDescriptor for one of the standard Console streams.
*/
//==============================================================================
AutoPtr<FileDescriptor> FileSystem::getConsoleFD(ConsoleStream stream) const;

//==============================================================================
// FileSystem::readFile
//
/**
   Reads up to @c bufLen bytes from an open file into the supplied buffer.

   @param pBuffer A pointer to the buffer into which the bytes will be copied.
          This must be capable of holding at least @c bufLen bytes.
   @param bufLen The maximum number of bytes to read into the passed buffer.

   @throws IllegalArgumentException if @c bufLen is zero
   @throws NullPointerException if @c pBuffer is null
   @throws IOException if an I/O error occurs
   @returns The number of bytes read or zero if the
            end of the file has been reached.
*/
//==============================================================================
size_t FileSystem::readFile(FileDescriptor* pFD, Byte* pBuffer, size_t bufLen) const;

//==============================================================================
// FileSystem::writeFile
//
/**
   Writes an array of bytes to an open file.

   @param pBuffer pointer to the start of an array of bytes to be written
   @param bufLen length of the byte array
   @throws NullPointerException if @c pBuffer is null.
   @throws IOException if an I/O error occurs.
*/
//==============================================================================
void FileSystem::writeFile(FileDescriptor* pFD, const Byte* pBuffer, size_t bufLen) const;

#endif // !QC_DOCUMENTATION_ONLY

QC_IO_NAMESPACE_END

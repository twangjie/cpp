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
// Class File
/**
	@class qc::io::File
	
	@brief A system-independent representation of file and directory names. 

	Different operating systems employ various conventions for the
	naming of files and directories.  
	This class presents an abstract, system-independent view of file and 
	directory pathnames, together with convenient methods for 
	manipulating them. 
	@par
	An abstract pathname has two components: 
	<ol>
		<li>
		An optional system-dependent prefix string,
		such as a DOS disk-drive identifier, the UNIX root directory name 
		@c /, or the Windows Universal Naming Convention (UNC) prefix 
		@c \\host\share\ ; and 
		<li>
		A sequence of zero or more string names representing the hierarchical
		directory structure
	</ol>
	Each name in an abstract pathname except for the last denotes a directory;
	the last name may denote either a directory or a file.
	@par
	This class provides the means to convert from a system-dependent
	pathname into a system-independent abstract pathname.  This is 
	achieved by splitting the string into a sequence of tokens
	separated by the system-specific name separator character which can be obtained
	using the static method GetSeparatorChar().
	@par
	A pathname may be either absolute or relative. An absolute pathname
	can be used to locate the file or directory which it denotes without
	the need for any other information.  A relative pathname, in contrast,
	must be interpreted in terms of some other absolute pathname. 
	@par
	Relative pathnames are resolved against the system's current directory which
	can be obtained using the static method GetCurrentDirectory().
	@par
	Whether or not an abstract pathname is relative depends on the optional
	prefix string.  It is interpreted as follows:-
	<ul>
		<li>
		For UNIX platforms, the prefix of an absolute pathname is always 
		@c /. Relative pathnames have no prefix. The abstract pathname denoting the 
		root directory has the prefix @c / and an empty name sequence. 
		<li>
		For Windows platforms, the prefix of a pathname that contains a drive
		specifier consists of the drive letter followed by ":" and 
		possibly followed by @c \ if the pathname is absolute.
		The prefix of a UNC pathname is @c \\<host>\<share>\. 
		UNC names are always absolute.
	</ul>
	The concept of a prefix and name sequence is an internal abstraction;
	these separate components are not exposed in any interfaces.
	@par
	Instances of the File class are immutable; that is, once created,
	the abstract pathname represented by a File object will never change. 
	@par
	The File class is a simple value-type class.  It is not reference-counted
	and can be passed by value or by reference.
*/
//==============================================================================

#include "File.h"
#include "IOException.h"
#include "FileSystem.h"
#include "FileDescriptor.h"

#include "QcCore/base/String.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/util/StringTokenizer.h"

#include <algorithm>

#include <fcntl.h>
#include <stdio.h>

QC_IO_NAMESPACE_BEGIN

using namespace util;

//==============================================================================
//  File::File()
/**
	Constructs a File from a pathname string.

	The pathname may be relative or absolute.  If, subsequent to construction,
	member functions such as canRead() are called, relative abstract pathnames
	are resolved into absolute pathnames by combining the pathname 
	with the current working directory.

	@param  pathname a relative or absolute pathname
*/
//==============================================================================
File::File(const String& pathname) :
	m_rpFS(FileSystem::GetFileSystem())
{
	m_path = m_rpFS->normalize(pathname);
	m_prefixLength = m_rpFS->getPrefixLength(m_path);
}

//==============================================================================
// File::File
//
/**
	Constructs a File using a parent pathname string and a child pathname string.

	Create an abstract pathname by resolving the child in relation to the
	parent.  If the parent represents an empty abstract pathname then 
	it is substituted with @c "/" which, on UNIX platforms represents the
	absolute root filename but under Windows this represents the
	root directory of the current drive.
	@par
	If you wish to resolve a relative pathname against the current working directory
	then you should use the File(const String& name) constructor or pass the value
	returned from GetCurrentDirectory() as the parent.

	@param  parent the parent abstract pathname
	@param  child  the child pathname string
*/
//==============================================================================
File::File(const File& parent, const String& child) :
	m_rpFS(FileSystem::GetFileSystem())
{
	m_path = m_rpFS->resolve(parent.getPath(), child);
	m_prefixLength = m_rpFS->getPrefixLength(m_path);
}

//==============================================================================
// File::File
//
/**
	Constructs a File using a parent pathname string and a child pathname string.

	The parent pathname is treated as if it were a directory, even if it is
	not one.  If the parent pathname is empty then 
	it is substituted with @c "/" which, on UNIX platforms represents the
	absolute root filename but under Windows this represents the
	root directory of the current drive.
	@par
	If you wish to resolve a relative pathname against the current working directory
	then you should use the File(const String& name) constructor or pass the value
	returned from GetCurrentDirectory() as the parent.

	@param  parent the parent abstract pathname
	@param  child  the child pathname string
*/
//==============================================================================
File::File(const String& parent, const String& child) :
	m_rpFS(FileSystem::GetFileSystem())
{
	m_path = m_rpFS->resolve(parent, child);
	m_prefixLength = m_rpFS->getPrefixLength(m_path);
}

//==============================================================================
// File::File
//
/**
	Constructs a File from another abstract pathname.

	@param  rhs the abstract pathname
*/
//==============================================================================
File::File(const File& rhs) :
	m_rpFS(rhs.m_rpFS),
	m_path(rhs.m_path),
	m_prefixLength(rhs.m_prefixLength)
{
}
	
//==============================================================================
// File::operator==
//
/**
	Tests to see if this abstract pathname represents the same file or
	directory as the abstract pathname @c rhs.
	
	The test is case-insensitive on systems that do not have a case-sensitive 
	file system (e.g. Windows).  Before comparing the abstract pathnames, they are
	first made absolute by calling getCanonicalPath().

    @param rhs the abstract pathname to be compared
	@sa operator!=()
	@returns true if the abstract pathname is the same; otherwise false.
	@throws IOException is an error occurs obtaining the canonical path
*/
//==============================================================================
bool File::operator==(const File& rhs) const
{
	if(m_rpFS->isCaseSensitive())
		return (getCanonicalPath() == rhs.getCanonicalPath());
	else
		return (StringUtils::CompareNoCase(getCanonicalPath(), rhs.getCanonicalPath())==0);
}

//==============================================================================
// File::operator!=
//
/**
	Tests to see if this abstract pathname represents a different file or
	directory from the abstract pathname @c rhs.

  	The test is case-insensitive on systems that do not have a case-sensitive 
	file system (e.g. Windows).  Before comparing the abstract pathnames, they are
	first made absolute by calling getCanonicalPath().

    @param rhs the abstract pathname to be compared
    @sa operator==()
	@returns true if the abstract pathname is not the same; false otherwise
	@throws IOException is an error occurs obtaining the canonical path
*/
//==============================================================================
bool File::operator!=(const File& rhs) const
{
	return !(*this==rhs);
}

//==============================================================================
// File::canRead
//
/**
	Tests to see if the file denoted by this abstract filename is readable.

	This resolves the abstract filename and uses it to probe the file system
	to see if a file with that name exists and, if so, whether it's permission
	flags allow it to be read by the current process.

	@returns true if the file exists and is readable; false otherwise
*/
//==============================================================================
bool File::canRead() const
{
	return m_rpFS->checkAccess(m_path, FileSystem::ReadAccess);
}

//==============================================================================
// File::canWrite
//
/**
	Tests to see if the file denoted by this abstract pathname is writable.

	This resolves the abstract filename and uses it to probe the file system
	to see if a file with that name exists and, if so, whether it's permission
	flags allow it to be written to by the current process.

	@returns true if the file exists and is writable; false otherwise 
*/
//==============================================================================
bool File::canWrite() const
{
	return m_rpFS->checkAccess(m_path, FileSystem::WriteAccess);
}

//==============================================================================
// File::deleteFile
//
/**
	Deletes the file.
	This resolves the abstract filename and uses it to request that
	the underlying file system deletes the file.

	@returns true if the file was deleted, false otherwise.
	@throws  FileNotFoundException if a file with this abstract pathname 
	         does not exist
	@throws  IOException if an error occurs while deleting the file.
*/
//==============================================================================
void File::deleteFile() const
{
	m_rpFS->deleteFile(m_path);
}

//==============================================================================
// File::exists
//
/**
	Tests to see if the file exists.
	This resolves the abstract filename and uses it to probe the file system
	to see if a file with that name exists.

	@returns true if the file exists.
*/
//==============================================================================
bool File::exists() const
{
	return (m_rpFS->getFileAttributeFlags(m_path) & FileSystem::Exists);
}

//==============================================================================
// File::isDirectory
//
/**
	Tests to see if the file denoted by this abstract pathname is a directory.

	This resolves the abstract filename and uses it to probe the file system
	to see if a file with that name exists and, if so, whether it is a directory.

	@returns true if the file exists and is a directory; false otherwise
*/
//==============================================================================
bool File::isDirectory() const
{
	return ((m_rpFS->getFileAttributeFlags(m_path) & FileSystem::Directory)!=0);
}

//==============================================================================
// File::isFile
//
/**
	Tests to see if the file denoted by this abstract pathname is a file.

	This resolves the abstract filename and uses it to probe the file system
	to see if a file with that name exists and, if so, whether it is a regular file.

	@sa isDirectory()
	@returns true if the file exists and is a file; false otherwise
*/
//==============================================================================
bool File::isFile() const
{
	return ((m_rpFS->getFileAttributeFlags(m_path) & FileSystem::RegularFile)!=0);
}

//==============================================================================
// File::length
//
/**
	Returns the length in Bytes of the file denoted by this abstract pathname.

	This resolves the abstract filename and uses it to probe the file system
	to see if a file with that name exists and, if so, obtains its size.

	@returns the size of the file.
	@throws  IOException if an error occurs while probing the file.
	@throws  FileNotFoundException if the file does not exist
*/
//==============================================================================
size_t File::length() const
{
	return m_rpFS->getLength(m_path);
}

//==============================================================================
// File::getAbsolutePath
//
/**
	Returns the absolute pathname string of this abstract pathname.

	If the abstract pathname is already absolute (which can be determined by calling
	isAbsolute()), then this method simply returns the pathname as if by calling
	getPath().  Otherwise the abstract pathname is resolved against the 
	current working directory using GetCurrentDirectory() on this class.

	@returns the absolute pathname string of this abstract pathname.
	@throws  IOException if an error occurs while getting the current working
			 directory.
*/
//==============================================================================
String File::getAbsolutePath() const
{
	if(isAbsolute())
	{
		return m_path;
	}

	return m_rpFS->resolve(m_path); 
}

//==============================================================================
// File::GetCurrentDirectory
//
/**
	Returns the current working directory as reported by the underlying
	operating system.

	@returns the current working directory.
	@throws  IOException if an error occurs while fetching the current working
			 directory.
*/
//==============================================================================
String File::GetCurrentDirectory()
{
	return FileSystem::GetFileSystem()->getCurrentDirectory();
}

//==============================================================================
// File::getCanonicalPath
//
/**
	Returns the canonical path for the abstract pathname denoted by this File.

	The canonical path is system dependent, but is always absolute
	and generally the shortest means of expressing a file name.

    @par
	If this File denotes a relative abstract filename, then it is first made
	absolute by calling getAbsolutePath().

    @par
	The canonical path of an existing file @em may differ to the canonical path of
	an abstract pathname that doesn't exist because the underlying operating
	system (esp. Windows) may change the case or absolute location of an
	existing file.

	@returns the current canonical path as a string.
	@throws  IOException if an error occurs while obtaining the current working
	         directory.
*/
//==============================================================================
String File::getCanonicalPath() const
{
	return m_rpFS->canonicalize(getAbsolutePath());
}

//==============================================================================
// File::GetCanonicalPath
//
/**
	Static function that performs a basic canonicalization algorithm
	to reduce an absolute pathname to its shortest form.

	The algorithm involves the following steps: -
		-# isolates the prefix, which remains static
		-# removes "/." sequences
		-# removes "<dir>/.." sequences
    
	@par
	This function does not perform case-folding on case-insensitive file systems such
	as Windows. Use the non-static version of getCanonicalPath() for that.

	@sa getCanonicalPath()
	@returns a canonical representation of the input pathname
	@param   path the pathname to be canonicalized
*/
//==============================================================================
String File::GetCanonicalPath(const String& path)
{
	//
	// Deal with the empty string right away
	//
	if(path.empty())
	{
		return String();
	}

	//
	// The prefix is copied to the output unchanged
	//
	size_t prefixLen = FileSystem::GetFileSystem()->getPrefixLength(path);
	String ret = path.substr(0, prefixLen);

	//
	// The remainder of the path represents a sequence of names
	// separated by the path separator character.
	//
	StringTokenizer tokenizer(path.substr(prefixLen), GetSeparator());
	String sequence;
	
	const CharType sep = GetSeparatorChar();
	
	while(tokenizer.hasMoreTokens())
	{
		const String& name = tokenizer.nextToken();
		if(name == QC_T("."))
		{
			continue;
		}
		else if(name == QC_T(".."))
		{
			size_t pos = sequence.find_last_of(sep);
			if(pos != String::npos)
			{
				sequence.erase(pos);
			}
			else
			{
				// This is an error.  We have a ".." with no preceding
				// name.  We've gone back as far as we could!
				sequence += sep;
				sequence += name;
			}
		}
		else
		{
			sequence += sep;
			sequence += name;
		}
	}

	if(!sequence.empty())
	{
		//
		// erase the trailing separator from the prefix (if any)
		//
		if(prefixLen && ret[prefixLen-1] == sep)
		{
			ret.erase(prefixLen-1);
		}
		ret += sequence;
	}

	return ret;
}

//==============================================================================
// File::getName
//
/**
	Returns the name denoted by the abstract pathname of this file or directory.

	The name is the last part of the name sequence, separated from the rest of the
	path by the final filename separator character.
	@par
	The filename separator character can be obtained using the static
	GetSeparatorChar() function or, in string form, using GetSeparator().
	@par
	If the abstract pathname is empty, or consists only of a prefix, then
	the empty string is returned.

	@returns the name of the file or directory.
*/
//==============================================================================
String File::getName() const
{
	size_t sepPos = m_path.find_last_of(GetSeparatorChar());
	if(sepPos != String::npos)
	{
		return m_path.substr(sepPos+1);
	}
	else
	{
		return m_path.substr(m_prefixLength);
	}
}

//==============================================================================
// File::getParent
//
/**
	Returns the pathname string denoting the parent directory of this abstract
	pathname, or the empty string if this does not name a parent directory.

	The parent consists of the prefix (if any) followed by 
	all names from the sequence except the last (which is called the file name
	and may be obtained using the getName() method).
	@par
	If the name sequence is contains only one name, then this abstract pathname
	does not name a parent.

	@sa getParentFile()    
	@returns the pathname string denoting the parent of this abstract pathname.
*/
//==============================================================================
String File::getParent() const
{
	size_t namePos = m_path.find_last_of(GetSeparatorChar());
	if(namePos != String::npos)
	{
		if(namePos < m_prefixLength)
		{
			namePos = m_prefixLength;
		}
		return m_path.substr(0, namePos);
	}
	else
	{
		return String();
	}
}

//==============================================================================
// File::getParentFile
//
/**
	Returns a File representing the abstract pathname denoting the parent
	directory of this abstract pathname.

	@sa getParent()
	@returns a File object representing the abstract pathname denoting
	         the parent directory
*/
//==============================================================================
File File::getParentFile() const
{
	return File(getParent());
}

//==============================================================================
// File::getPath
//
/**
	Returns the abstract pathname as a string.

	@returns the abstract pathname as a string.
*/
//==============================================================================
String File::getPath() const
{
	return m_path;
}

//==============================================================================
// File::isAbsolute
//
/**
	Tests whether the abstract pathname denotes an absolute pathname.

	An absolute pathname does not require any additional context information
	for the operating system to resolve into a physical file.  Conversely,
	a relative pathname needs to be resolved against an absolute pathname
	before it can be used to access a file.
	
	@par
	The definition of an absolute pathname is system-dependent.  On UNIX
	platforms a pathname is absolute if is has a leading "/".  On Windows
	platforms it is absolute if a drive letter and "\\" are specified, or 
	it is a UNC name.

	@returns true if the abstract pathname denotes an absolute pathname; false
	         otherwise.
*/
//==============================================================================
bool File::isAbsolute() const
{
	return m_rpFS->isAbsolute(m_path);
}

//==============================================================================
// File::listDirectory
//
/**
	Returns a list of names of the files and directories
	contained in the directory denoted by this abstract pathname.

	If this abstract pathname does not denote a directory (which can
	be determined by calling the isDirectory() member function on this class)
	then an empty list is returned.

	@returns a list of strings of file and directory names.
	@throws IOException if an error occurs while listing the file names.
*/
//==============================================================================
std::list<String> File::listDirectory() const
{
	std::list<String> retList;
	
	if(isDirectory())
	{
		return m_rpFS->listDirectory(m_path);
	}

	return retList;
}

//==============================================================================
// File::mkdir
//
/**
	Creates a directory with the name of this abstract pathname.

	@throws IOException if an unexpected error occurs while attempting
		to create the directory or the directory already exists.
	@throws FileNotFoundException if any part of the pathname sequence
		except for the last does not exist
*/
//==============================================================================
void File::mkdir() const
{
	m_rpFS->createDirectory(m_path);
}

//==============================================================================
// File::mkdirs
//
/**
	Creates a directory with the name of this abstract pathname.  If any of the
	parent directories do not already exist, this function will attempt to
	create these also.

	@sa mkdir()
	@throws IOException if an error occurs while attempting to create directories
		or the directory already exists
*/
//==============================================================================
void File::mkdirs() const
{
	//
	// tokenize the pathname excluding the prefix, and turn it into
	// a vector;
	//
	std::vector<String> tokens = 
		StringTokenizer(m_path.substr(m_prefixLength), GetSeparator()).toVector();

	//
	// For each token in the path, check if the directory exists.
	// and if not ask the FileSystem to create it.
	//
	String path = m_path.substr(0, m_prefixLength);
	for(size_t i=0; i<tokens.size(); ++i)
	{
		if(!path.empty())
		{
			path += GetSeparator();
		}
		
		path += tokens[i];

		int fileAttr = m_rpFS->getFileAttributeFlags(path);
		if(fileAttr && FileSystem::Exists)
		{
			// If the path exists, but isn't a directory then we have an
			// exceptional case
			if(!(fileAttr && FileSystem::Directory))
			{
				throw IOException(QC_T("path exists but is not a directory"));
			}
		}
		else
		{
			m_rpFS->createDirectory(path);
		}
	}
}

//==============================================================================
// File::renameTo
//
/**
	Renames the file denoted by the abstract pathname to the abstract
	pathname @c file.

  	@param   file new abstract pathname
	@returns true if the file was renamed; false otherwise
	@throws  IOException if an error occurs while renaming the file.
	@throws  FileNotFoundException if a file with this abstract pathname does not
		     exist or if any part of the new pathname sequence does not exist
*/
//==============================================================================
void  File::renameTo(const File& file) const
{
	m_rpFS->rename(m_path, file.getPath());
}

//==============================================================================
// File::GetSeparatorChar
//
/**
	Returns the system-dependent filename separator character.  On Windows-based
	platforms this is '\\' and on Unix platforms it is '/'.

	@returns the system-dependent filename separator character
*/
//==============================================================================
CharType File::GetSeparatorChar()
{
	return FileSystem::GetFileSystem()->getSeparatorChar();
}

//==============================================================================
// File::GetSeparator
//
/**
	Returns the system-dependent filename separator character as a String with
	a length of 1.  On Windows-based platforms this is "\\" and on Unix platforms
	it is "/".

	@returns the system-dependent filename separator character as a String
*/
//==============================================================================
String File::GetSeparator()
{
	return FileSystem::GetFileSystem()->getSeparator();
}

//==============================================================================
// File::NormalizePath
//
// Private static function called by the the constructors to convert an
// external string into a normalized form. 
// The normalized form uses the correct separtator character
// and does not contain a trailing separator (unless it belongs to the prefix)
//==============================================================================
String File::NormalizePath(const String& path)
{
	if(path.empty())
	{
		return path;
	}
	else
	{
		String ret = path;
		const CharType sep = GetSeparatorChar();
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
		size_t prefixLen = FileSystem::GetFileSystem()->getPrefixLength(ret);
		if((sep == ret[ret.length()-1]) && (prefixLen < ret.length()))
		{
			ret = ret.erase(ret.length()-1);
		}
		return ret;
	}
}

//==============================================================================
// File::lastModified
//
/**
	Returns the DateTime that the file was last modified.

	@returns the DateTime that the file was last modified
	@throws IOException if an error occurs while obtaining the last modified DateTime
	@throws FileNotFoundException if a file with this abstract pathname does not
		exist
*/
//==============================================================================
DateTime File::lastModified() const
{
	return m_rpFS->getLastModifiedTime(m_path);
}

//==============================================================================
// File::setLastModified
//
/**
	Sets the date and time the file was last modified to @c time.  On some platforms
	the file system does not support the full resolution provided by the
	DateTime class.  In this case the time is truncated to the most precise
	value that is supported.

    @param time the date and time value to set
	@throws IOException if an error occurs while setting the modified DateTime
	@throws FileNotFoundException if a file with this abstract pathname does not
		    exist
*/
//==============================================================================
void File::setLastModified(const DateTime& time) const
{
	m_rpFS->setLastModifiedTime(m_path, time);
}

//==============================================================================
// File::setReadOnly
//
/**
	Updates the file's security permission flags.
	
	On Unix platforms, when setting the file to writable, only the 'user'
	permission flags are updated - the file's 'group' and 'other' permission
	flags are untouched.

	@param bReadOnly set to true to make the file read only, false to make it 
	       writable
	@throws IOException if an error occurs while setting the file's permission flags
	@throws FileNotFoundException if a file with this abstract pathname does not
	        exist
*/
//==============================================================================
void File::setReadOnly(bool bReadOnly) const
{
	m_rpFS->setReadOnly(m_path, bReadOnly);
}

//==============================================================================
// File::createNewFile
//
/**
	Creates a new file on the underlying file system.
	
	This function tests for the existence of the file and creates the file
	in a single atomic operation.  Because the test and the creation are
	performed in a single operation, it is possible to use this to create lock
	files.

	@throws IOException if an error occurs while creating the file
	@throws ExistingFileException if the file already exists
*/
//==============================================================================
void File::createNewFile() const
{
	// File will automatically be closed as the file descriptor goes
	// out of scope
	AutoPtr<FileDescriptor> rpFD = m_rpFS->openFile(m_path,
		FileSystem::WriteAccess,
		FileSystem::OpenCreateExclusive,
		0);
}

QC_IO_NAMESPACE_END

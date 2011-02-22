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

#ifdef WIN32

#include "Win32FileSystem.h"
#include "File.h"
#include "FileNotFoundException.h"
#include "ExistingFileException.h"
#include "IOException.h"
#include "Win32FileDescriptor.h"

#include "QcCore/base/Tracer.h"
#include "QcCore/base/NullPointerException.h"
#include "QcCore/base/IllegalArgumentException.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/base/SystemUtils.h"
#include "QcCore/util/Win32Utils.h"

QC_IO_NAMESPACE_BEGIN

using util::Win32Utils;

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

//==============================================================================
// Win32FileSystem::isAbsolute
//
/**
	tests whether the passed pathname string
	denotes an absolute pathname.

	An absolute pathname does not require any additional context information
	for the operating system to resolve into a physical file.  Conversely,
	a relative pathname needs to be resolved against an absolute pathname
	before it can be used to access a file.
	<p>
	The definition of an absolute pathname is system-dependent.  
	On Windows platforms it is absolute if a drive letter and "\\" 
	are specified, or it is a UNC name is provided.

	@param path must be a normalized pathname string
	@returns true if the abstract pathname denotes an absolute pathname; false
	otherwise.
*/
//==============================================================================
bool Win32FileSystem::isAbsolute(const String& path) const
{
	if(path.length() < 1)
		return false;

	const CharType sep = getSeparatorChar();

	//
	// Under Windows, filenames starting with the separator character
	// are not necessarily absolute.
	// The following examples ARE absolute: 
	//     c:\temp\x 
	//     \\kirk\docs\rob
	// The following examples ARE NOT absolute: 
	//     c:rob
	//     \rob
	//
	if(path.length() > 2)
	{

		if((path[1] == ':' && path[2] == sep) ||
		   (path[0] == sep && path[1] == sep))
		{
			return true;
		}
	}

	return false;
}

//==============================================================================
// Win32FileSystem::isCaseSensitive
//
/**
	Returns true if the underlying file system uses case-sensitive file names.
	Win32 does not use case-sensitive file names.

	@returns true if the underlying file system uses case-sensitive file names;
	         false otherwise.
*/
//==============================================================================
bool Win32FileSystem::isCaseSensitive() const
{
	return false;
}

//==============================================================================
// Win32FileSystem::getPrefixLength
//
/**
	calculate the length of the prefix of a given abstract pathname;

	The prefix is system-dependent.  As desribed in the Class preamble,
	on UNIX systems the prefix is sinply a leading "/", on Windows it is 
	a little more complicated.
    <p>
	For Windows platforms, the prefix of a pathname that contains a drive
	specifier consists of the drive letter followed by ":" and 
	possibly followed by "\" if the pathname is absolute.
	The prefix of a UNC pathname is "\\\\<host>\\<share>\\".

	The terminating separation character is part of the prefix.

	A relative pathname that does not specify a drive has no prefix.

	@param  pathname a relative or absolute pathname
	@return the length of the prefix or 0 if no prefix exists
*/
//  Note: our definition of the prefix differs from JDK with respect to
//  Win32 relative paths and UNC shares.  This is because the concept of a
//  prefix makes more sense when it is the static part of the pathname that
//  is not subject to canonicalization or resolution.
//
//==============================================================================
size_t Win32FileSystem::getPrefixLength(const String& pathname) const
{
	const CharType sep = getSeparatorChar();

	//
	// Windows prefixes are: "c:", "c:\", "\\host\share\" 
	//
	if(pathname.length() > 1)
	{
		//
		// Test for UNC name
		//
		if(pathname[0] == sep && pathname[1] == sep)
		{
			//
			// Our prefix includes the UNC host and share
			// as in "\\kirk\spock\"
			//
			if(pathname.length() > 2)
			{
				size_t pos = pathname.find(sep, 2);
				if(pos!=String::npos)
				{
					pos = pathname.find(sep, pos+1);
					if(pos!=String::npos)
					{
						return pos;
					}
				}
			}
			//
			// If we fall thru here the UNC was missing a terminator - 
			// so we'll grab the whole lot
			//
			return pathname.length();
		}
		else if(pathname[1] == ':')
		{
			if(pathname.length() > 2 && pathname[2] == sep)
			{
				return 3;
			}
			else
			{
				return 2;
			}
		}
	}

	//
	// Even under windows, a leading "/" is treated as part of the
	// prefix to aid in concatenating filenames.  It has no effect
	// on whether the filename is absolute however.
	//
	if(!pathname.empty() && pathname[0] == sep)
	{
		return 1;
	}
	
	return 0;
}

//==============================================================================
// Win32FileSystem::resolve
//
/**
	Resolves a relative pathname into an absolute pathname;

	In the normal case a relative pathname is resolved against the
	current directory.
	@par
	Under Win32, a relative pathname is resolved against the current directory
	unless it starts with the separation character, in which case it resolves
	against the root directory of the current drive.

	@param  path a relative pathname
	@return the resolved abstract pathname as a String
	@throws IOException is an error occurs while obtaining the current
	working directory
*/
//==============================================================================
String Win32FileSystem::resolve(const String& path) const
{
	QC_DBG_ASSERT(!isAbsolute(path));

	const String& currDir = getCurrentDirectory();
	if(path.length() && path[0] == getSeparatorChar())
	{
		size_t cdPrefix = getPrefixLength(currDir);
		return currDir.substr(0, cdPrefix) + path.substr(1);
	}

	return resolve(getCurrentDirectory(), path);
}

//==============================================================================
// Win32FileSystem::closeFile
//
/**
	Closes an open file described by a FileDescriptor object.

	@param  pFD a pointer to the file descriptor to close
	@throws NullPointerException if the file descriptor pointer is null
	@throws IOException is an error occurs while closing the file
*/
//==============================================================================
void Win32FileSystem::closeFile(FileDescriptor* pFD) const
{
	if(!pFD) throw NullPointerException();
	Win32FileDescriptor* pMyFD = static_cast<Win32FileDescriptor*>(pFD);

	if(!::CloseHandle(pMyFD->getHandle()))
	{
		throw IOException(SystemUtils::GetWin32ErrorString(GetLastError()));
	}
}

//==============================================================================
// Win32FileSystem::list
//
//==============================================================================
std::list<String> Win32FileSystem::listDirectory(const String& path) const
{
	QC_DBG_ASSERT(getFileAttributeFlags(path) & Directory);
	std::list<String> ret;

	//
	// Create a search string comprising the pathname as the base
	// with a wildcard "*" appended to find all files in the specified 
	// directory
	//
	String wildPath = path;
	wildPath += getSeparatorChar();
	wildPath += '*';

	WIN32_FIND_DATA findData;
	HANDLE hFind = ::FindFirstFile(GetWin32Filename(wildPath).get(), &findData);

	if(hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			String foundPath = StringUtils::FromWin32String(findData.cFileName);
			//
			// Filter out the ".." (parent) and "." (this) directory names
			//
			if(foundPath != QC_T(".") && foundPath != QC_T(".."))
			{
				ret.push_back(foundPath);
			}
		}
		while(FindNextFile(hFind, &findData));

		::FindClose(hFind);
	}

	return ret;
}

//==============================================================================
// Win32FileSystem::createDirectory
//
//==============================================================================
void Win32FileSystem::createDirectory(const String& path) const
{
	const BOOL bSuccess = ::CreateDirectory(GetWin32Filename(path).get(), NULL);
	const DWORD errCode = GetLastError();

	if(Tracer::IsEnabled())
	{
		String traceMsg = QC_T("create directory: ");
		traceMsg += path;
		Win32Utils::TraceSystemCall(Tracer::IO, Tracer::Medium, traceMsg, errCode);
	}

	if(!bSuccess)
	{
		TranslateCodeToException(errCode, path);
	}
};

//==============================================================================
// Win32FileSystem::rename
//
//==============================================================================
void Win32FileSystem::rename(const String& oldPath, const String& newPath) const
{
	const BOOL bSuccess = ::MoveFile(GetWin32Filename(oldPath).get(), GetWin32Filename(newPath).get());
	const DWORD errCode = ::GetLastError();
	if(Tracer::IsEnabled())
	{
		String traceMsg = QC_T("MoveFile: ");
		traceMsg += oldPath;
		traceMsg += QC_T(",");
		traceMsg += newPath;
		Win32Utils::TraceSystemCall(Tracer::IO, Tracer::Medium, traceMsg, errCode);
	}

	if(!bSuccess)
	{
		TranslateCodeToException(errCode, String());
	}
}

//==============================================================================
// Win32FileSystem::setLastModifiedTime
//
//==============================================================================
void Win32FileSystem::setLastModifiedTime(const String& path, const DateTime& time) const
{
	if(!time.isValid()) throw IllegalArgumentException();

	AutoPtr<FileDescriptor> rpFD = openFile(path, WriteAccess, OpenExisting, 0);
	Win32FileDescriptor* pMyFD = static_cast<Win32FileDescriptor*>(rpFD.get());
	FILETIME fileModTime;
	SYSTEMTIME sysModTime;
	Win32Utils::DateTimeToSystemTime(time, &sysModTime);
	BOOL bSuccess = ::SystemTimeToFileTime(&sysModTime, &fileModTime);
	DWORD errCode = ::GetLastError();
	if(bSuccess)
	{
		bSuccess = ::SetFileTime(pMyFD->getHandle(), NULL, NULL, &fileModTime);
		errCode = ::GetLastError();
		if(Tracer::IsEnabled())
		{
			String traceMsg = QC_T("SetFileTime: ");
			traceMsg += path;
			traceMsg += QC_T(" ");
			traceMsg += time.toString();
			Win32Utils::TraceSystemCall(Tracer::IO, Tracer::Medium, traceMsg, errCode);
		}
	}
	if(!bSuccess)
	{
		TranslateCodeToException(errCode, path);
	}
}

//==============================================================================
// Win32FileSystem::setReadOnly
//
//==============================================================================
void Win32FileSystem::setReadOnly(const String& path, bool bReadOnly) const
{
	TCharPtr win32Filename = GetWin32Filename(path);
	DWORD dwAttrs = ::GetFileAttributes(win32Filename.get());
	
	if(dwAttrs == (DWORD)-1)
	{
		TranslateCodeToException(0, path);
	}
	else
	{
		if(bReadOnly)
			dwAttrs |= FILE_ATTRIBUTE_READONLY;
		else
			dwAttrs &= ~FILE_ATTRIBUTE_READONLY;

		const BOOL bSuccess = ::SetFileAttributes(win32Filename.get(), dwAttrs);

		const DWORD errCode = ::GetLastError();

		if(Tracer::IsEnabled())
		{
			String traceMsg = QC_T("SetFileAttributes: ");
			traceMsg += path;
			Win32Utils::TraceSystemCall(Tracer::IO, Tracer::Medium, traceMsg, errCode);
		}

		if(!bSuccess)
		{
			TranslateCodeToException(errCode, path);
		}
	}
}

//==============================================================================
// Win32FileSystem::openFile
//
//==============================================================================
AutoPtr<FileDescriptor> Win32FileSystem::openFile(const String& path,
                                                 int accessMode,
                                                 CreationDisp creationDisp,
                                                 int attributes) const
{
	DWORD dwDesiredAccess=0;
	DWORD dwShareMode=FILE_SHARE_READ | FILE_SHARE_WRITE;
	LPSECURITY_ATTRIBUTES lpSecurityAttributes=NULL;
	DWORD dwCreationDisposition=0;
	DWORD dwFlagsAndAttributes=0;
	HANDLE hTemplateFile=NULL;

	if(accessMode & ReadAccess)  dwDesiredAccess |= GENERIC_READ;
	if(accessMode & WriteAccess) dwDesiredAccess |= GENERIC_WRITE;
		
	switch(creationDisp)
	{
	case OpenExisting:
        dwCreationDisposition = OPEN_EXISTING;
		break;
	case OpenCreateAppend:
		dwCreationDisposition = OPEN_ALWAYS;
		break;
	case OpenCreateExclusive:
		dwCreationDisposition = CREATE_NEW;
		break;
	case OpenCreateTruncate:
		dwCreationDisposition = CREATE_ALWAYS;
		break;
	}

	//
	// The only attributes that make any sense when creating a file
	// are:
	//  ReadOnly
	//  Hidden
	// And these are only useful when creating a new file
	//
	if(creationDisp == OpenCreateExclusive || creationDisp == OpenCreateTruncate)
	{
		if(attributes & ReadOnly) dwFlagsAndAttributes |= FILE_ATTRIBUTE_READONLY;
		if(attributes & Hidden)   dwFlagsAndAttributes |= FILE_ATTRIBUTE_HIDDEN;
	}

	HANDLE hFile = ::CreateFile(GetWin32Filename(path).get(), 
	                            dwDesiredAccess,
	                            dwShareMode,
	                            lpSecurityAttributes,
	                            dwCreationDisposition,
	                            dwFlagsAndAttributes,
	                            hTemplateFile);

	const DWORD errCode = ::GetLastError();

	if(Tracer::IsEnabled())
	{
		String traceMsg = QC_T("CreateFile: ");
		traceMsg += path;
		Win32Utils::TraceSystemCall(Tracer::IO, Tracer::Medium, traceMsg, errCode);
	}

	//
	// Translate errors into one of our Exceptions
	//
	if(hFile == INVALID_HANDLE_VALUE)
	{
		TranslateCodeToException(errCode, path);
	}

	//
	// If we have been asked to open the file in "append" mode, the file pointer
	// must be set appropriately
	//
	if(creationDisp == OpenCreateAppend)
	{
		if(::SetFilePointer(hFile, 0, 0, FILE_END) == INVALID_SET_FILE_POINTER)
		{
			throw IOException(SystemUtils::GetWin32ErrorString(::GetLastError()));
		}
	}

	//
	// Wrap the newly created file handle in a Win32FileDescriptor
	// which will ensure that the handle is closed when it is no longer
	// needed
	//
	return new Win32FileDescriptor(this, hFile, true);
}

//==============================================================================
// Win32FileSystem::checkAccess
//
// Does not throw exceptions
//==============================================================================
bool Win32FileSystem::checkAccess(const String& path, AccessMode mode) const
{
	const int attrs = getFileAttributeFlags(path);
	return (attrs && (mode == ReadAccess || !(attrs & ReadOnly)));
}

//==============================================================================
// Win32FileSystem::getFileAttributeFlags
//
// Does not throw any exceptions
//==============================================================================
int Win32FileSystem::getFileAttributeFlags(const String& path) const
{
	int ret=0;
	
	const DWORD dwAttrs = ::GetFileAttributes(GetWin32Filename(path).get());
	
	if(dwAttrs != (DWORD)-1)
	{
		ret = FileSystem::Exists;

		if(dwAttrs & FILE_ATTRIBUTE_DIRECTORY)
			ret |= FileSystem::Directory;
		else
			ret |= FileSystem::RegularFile;

		if(dwAttrs & FILE_ATTRIBUTE_HIDDEN)    ret |= FileSystem::Hidden;
		
		if(dwAttrs & FILE_ATTRIBUTE_READONLY)  ret |= FileSystem::ReadOnly;
	}

	return ret;
};

//==============================================================================
// Win32FileSystem::getLastModifiedTime
//
// In common with the Win32 CRT, we use FindFirstFile in place of stat.
// @throws FileNotFoundException
// @throws IOException
//==============================================================================
DateTime Win32FileSystem::getLastModifiedTime(const String& path) const
{
	WIN32_FIND_DATA findbuf;
	FindExistingFile(path, &findbuf);
	SYSTEMTIME sysTime;
	if(::FileTimeToSystemTime(&findbuf.ftLastWriteTime, &sysTime)==0)
	{
		throw IOException(SystemUtils::GetWin32ErrorString(::GetLastError()));
	}
	return Win32Utils::SystemTimeToDateTime(&sysTime);
}

//==============================================================================
// Win32FileSystem::getLength
//
// In common with the Win32 CRT, we use FindFirstFile in place of stat.
// @throws FileNotFoundException
// @throws IOException
//==============================================================================
size_t Win32FileSystem::getLength(const String& path) const
{
	WIN32_FIND_DATA findbuf;
	FindExistingFile(path, &findbuf);
	return findbuf.nFileSizeLow;
}

//==============================================================================
// Win32FileSystem::deleteFile
//
//==============================================================================
void Win32FileSystem::deleteFile(const String& path) const
{
	BOOL bSuccess;

	if(getFileAttributeFlags(path) & Directory)
	{
		bSuccess = ::RemoveDirectory(GetWin32Filename(path).get());
	}
	else
	{
		bSuccess = ::DeleteFile(GetWin32Filename(path).get());
	}

	const DWORD errCode = ::GetLastError();

	if(Tracer::IsEnabled())
	{
		String traceMsg = QC_T("delete file: ");
		traceMsg += path;
		Win32Utils::TraceSystemCall(Tracer::IO, Tracer::Medium, traceMsg, errCode);
	}

	if(!bSuccess)
	{
		TranslateCodeToException(errCode, path);
	}
}

//==============================================================================
// Win32FileSystem::GetWin32Filename
//
//==============================================================================
Win32FileSystem::TCharPtr Win32FileSystem::GetWin32Filename(const String& path)
{
	// todo: Unicode builds should add \\?\ prefix to take advantage of very
	//       long filenames
	return StringUtils::ToWin32String(path);
}

AutoPtr<FileDescriptor> Win32FileSystem::getConsoleFD(ConsoleStream stream) const
{
	DWORD type=0;
	switch(stream)
	{
		case Stdin:  type=STD_INPUT_HANDLE;  break;
		case Stdout: type=STD_OUTPUT_HANDLE; break;
		case Stderr: type=STD_ERROR_HANDLE;  break;
	}

	HANDLE h = ::GetStdHandle(type);

	if(h == INVALID_HANDLE_VALUE)
	{
		throw IOException(SystemUtils::GetWin32ErrorString(::GetLastError()));
	}
	return new Win32FileDescriptor(this, h, false);
}

size_t Win32FileSystem::readFile(FileDescriptor* pFD, Byte* pBuffer, size_t bufLen) const
{
	if(!pFD) throw NullPointerException();
	if(!pBuffer) throw NullPointerException();

	Win32FileDescriptor* pMyFD = static_cast<Win32FileDescriptor*>(pFD);
	DWORD bytesRead;
	const BOOL bSuccess = ::ReadFile(pMyFD->getHandle(), pBuffer, bufLen, &bytesRead, NULL);
	if(!bSuccess)
	{
		throw IOException(SystemUtils::GetWin32ErrorString(::GetLastError()));
	}
	return bytesRead;
}

void Win32FileSystem::writeFile(FileDescriptor* pFD, const Byte* pBuffer, size_t bufLen) const
{
	if(!pFD) throw NullPointerException();
	if(!pBuffer) throw NullPointerException();

	Win32FileDescriptor* pMyFD = static_cast<Win32FileDescriptor*>(pFD);
	DWORD bytesWritten;
	const BOOL bSuccess = ::WriteFile(pMyFD->getHandle(), pBuffer, bufLen, &bytesWritten, NULL);
	if(!bSuccess)
	{
		throw IOException(SystemUtils::GetWin32ErrorString(::GetLastError()));
	}
}

CharType Win32FileSystem::getSeparatorChar() const
{
	return '\\';
}

String Win32FileSystem::getCurrentDirectory() const
{
	const size_t nBufferLength = MAX_PATH+1;
	TCHAR buffer[nBufferLength];
	DWORD size = ::GetCurrentDirectory(nBufferLength, (TCHAR*)&buffer);
	if(!size)
	{
		throw IOException(SystemUtils::GetWin32ErrorString(::GetLastError()));
	}
	return StringUtils::FromWin32String(buffer);
};

//==============================================================================
// Win32FileSystem::canonicalize
//
// Perform case-folding on the case-insensitive file system under Win32.
//==============================================================================
String Win32FileSystem::canonicalize(const String& path) const
{
	//
	// The Win32 function ::GetLongPathName() does not reduce the 
	// filename to its shortest form, all it does it folds case and
	// translates between short and long filenames.  For this reason
	// we use the generic canonicalization algorithm provided by File
	// before calling ::GetLongPathName()
	//
	String ret = File::GetCanonicalPath(path);

	//
	// GetLongPathName requires Windows 2000 or Windows 95
	// and is not therefore an appropriate function to create
	// a runtime dependency on.
	//

#if 0

	TCharPtr pFileName(GetWin32Filename(ret));

	size_t buffSize = 255;
	while(true)
	{
		TCharPtr pBuffer(new TCHAR[buffSize]);
		DWORD size = ::GetLongPathName(pFileName.get(), pBuffer.get(), buffSize);
		if(size > buffSize)
		{
			buffSize = size;
		}
		else if(size == 0)
		{
			DWORD errCode = ::GetLastError();
			if(!IsNotFoundError(errCode))
			{
				throw IOException(SystemUtils::GetWin32ErrorString(errCode));
			}
			ret = StringUtils::FromWin32String(pFileName.get());
			break;
		}
		else
		{
			ret = StringUtils::FromWin32String(pBuffer.get());
			break;
		}
	}

#endif

	return ret;
}

//==============================================================================
// Win32FileSystem::FindExistingFile
//
//==============================================================================
void Win32FileSystem::FindExistingFile(const String& path, WIN32_FIND_DATA* pFindData)
{
	//
	// Don't allow wildcards to be interpreted by system
	//
	if(path.find_first_of(QC_T("?*")) != String::npos)
	{
		throw FileNotFoundException(path);
	}

	HANDLE hFind = ::FindFirstFile(GetWin32Filename(path).get(), pFindData);
	
	if(hFind != INVALID_HANDLE_VALUE)
	{
		::FindClose(hFind);
	}
	else
	{
		TranslateCodeToException(0, path);
	}
}

//==============================================================================
// Win32FileSystem::isNotFoundError
//
//==============================================================================
bool Win32FileSystem::IsNotFoundError(DWORD errCode)
{
	switch (errCode)
	{
	case ERROR_PATH_NOT_FOUND:
	case ERROR_FILE_NOT_FOUND:
	case ERROR_INVALID_NAME:
		return true;
	default:
		return false;
	}
}

//==============================================================================
// Win32FileSystem::TranslateCodeToException
//
//==============================================================================
void Win32FileSystem::TranslateCodeToException(DWORD errCode, const String& msg)
{
	if(errCode==0) errCode = ::GetLastError();

	switch (errCode)
	{
	case ERROR_PATH_NOT_FOUND:
	case ERROR_FILE_NOT_FOUND:
	case ERROR_BAD_PATHNAME:
		throw FileNotFoundException(msg);
	case ERROR_FILE_EXISTS:
		throw ExistingFileException(msg);
	default:
		{
			String exceptionMsg = msg;
			if(!msg.empty())
			{
				exceptionMsg += QC_T(" (");
				exceptionMsg += SystemUtils::GetWin32ErrorString(errCode); 
				exceptionMsg += QC_T(")");
			}
			else
			{
				exceptionMsg = SystemUtils::GetWin32ErrorString(errCode);
			}

			throw IOException(exceptionMsg);
		}
	}
}

QC_IO_NAMESPACE_END

#endif //WIN32


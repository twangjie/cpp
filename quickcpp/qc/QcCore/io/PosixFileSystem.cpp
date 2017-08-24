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

#include "PosixFileSystem.h"
#include "File.h"
#include "FileNotFoundException.h"
#include "ExistingFileException.h"
#include "IOException.h"
#include "PosixFileDescriptor.h"

#include "QcCore/base/IllegalArgumentException.h"
#include "QcCore/base/Tracer.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/base/SystemUtils.h"
#include "QcCore/base/NumUtils.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

//
// The microsoft SDK has a utime.h in sys,
// but BCC (on Windows) doesn't
//
#if defined(_MSC_VER)
#include <sys/utime.h>
#else
#include <dirent.h>
#include <utime.h>
#endif //WIN32

QC_IO_NAMESPACE_BEGIN

const size_t MaxPathLen = 256;
const String sNull;

//==============================================================================
// PosixFileSystem::GetCurrentDirectory
//
/**
	Returns the current working directory as reported by the underlying
	operating system.

	@returns the current working directory.
	@throws  IOException if an error occurs while fetching the current working
	         directory.
*/
//==============================================================================
String PosixFileSystem::getCurrentDirectory() const
{
	char buffer[MaxPathLen];
	if(getcwd(buffer, MaxPathLen)==0)
	{
		throw IOException(QC_T("unable to get current working directory"));
	}
	
	QC_DBG_ASSERT(strlen(buffer)!=0);
	return StringUtils::FromNativeMBCS(buffer);
}

//==============================================================================
// PosixFileSystem::GetSeparatorChar
//
//==============================================================================
CharType PosixFileSystem::getSeparatorChar() const
{
	return '/';
}

//==============================================================================
// PosixFileSystem::getFileAttributeFlags
//
// Does not throw any exceptions
//==============================================================================
int PosixFileSystem::getFileAttributeFlags(const String& path) const
{
	struct stat myStat;
	if(::stat(GetPosixFilename(path).c_str(), &myStat) == 0)
	{
		int ret = Exists;
		if(myStat.st_mode & S_IFDIR)       ret |= Directory;
		if(myStat.st_mode & S_IFREG)       ret |= RegularFile;
		if((myStat.st_mode & S_IWUSR)==0)  ret |= ReadOnly;
		return ret;
	}
	return 0;
}

//==============================================================================
// PosixFileSystem::checkAccess
//
// Does not throw exceptions
//==============================================================================
bool PosixFileSystem::checkAccess(const String& path, AccessMode mode) const
{
	struct stat myStat;
	if(::stat(GetPosixFilename(path).c_str(), &myStat) == 0)
	{
		return ((mode == ReadAccess  && myStat.st_mode & S_IRUSR) ||
				(mode == WriteAccess && myStat.st_mode & S_IWUSR));
	}
	else
	{
		return 0;
	}
}

//==============================================================================
// PosixFileSystem::deleteFile
//
/**
	Deletes the file.
	This resolves the abstract filename and uses it to request 
	the underlying operating system to delete the file or directory.

	@throws  IOException if an error occurs while deleting the file.
*/
//==============================================================================
void PosixFileSystem::deleteFile(const String& path) const
{
	int result=0;

	if(getFileAttributeFlags(path) & Directory)
	{	
		result = ::rmdir(GetPosixFilename(path).c_str());
	}
	else
	{
		result = ::remove(GetPosixFilename(path).c_str());
	}

	if(Tracer::IsEnabled())
	{
		String traceMsg = QC_T("delete file: ");
		traceMsg += path;
		SystemUtils::TraceSystemCall(Tracer::IO, Tracer::Medium, traceMsg, result);
	}

	if(result!=0)
	{
		TranslateCodeToException(0, path);
	}
}

//==============================================================================
// PosixFileSystem::isAbsolute
//
/**
	tests whether the passed pathname string
	denotes an absolute pathname.

	An absolute pathname does not require any additional context information
	for the operating system to resolve into a physical file.  Conversely,
	a relative pathname needs to be resolved against an absolute pathname
	before it can be used to access a file.
	<p>
	The definition of an absolute pathname is system-dependent.  Under UNIX
	a pathname is absolute if is has a leading "/".

	@param path must be a normalized pathname string
	@returns true if the abstract pathname denotes an absolute pathname; false
	otherwise.
*/
//==============================================================================
bool PosixFileSystem::isAbsolute(const String& path) const
{
	return (!path.empty() && path[0] == getSeparatorChar());
}

//==============================================================================
// PosixFileSystem::isCaseSensitive
//
/**
	Returns true if the underlying file system uses case-sensitive file names.

	@returns true if the underlying file system uses case-sensitive file names;
	         false otherwise.
*/
//==============================================================================
bool PosixFileSystem::isCaseSensitive() const
{
#ifndef WIN32
	return false;
#else
	return true;
#endif
}

//==============================================================================
// PosixFileSystem::getPrefixLength
//
/**
	calculate the length of the prefix of a given abstract pathname;

	The prefix is system-dependent.  As desribed in the Class preamble,
	on UNIX systems the prefix is simply a leading "/".

	@param  pathname a relative or absolute pathname
	@return the length of the prefix or 0 if no prefix exists
*/
//==============================================================================
size_t PosixFileSystem::getPrefixLength(const String& pathname) const
{
	if(!pathname.empty() && pathname[0] == getSeparatorChar())
	{
		return 1;
	}
	
	return 0;
}

//==============================================================================
// PosixFileSystem::getLastModifiedTime
//
//==============================================================================
DateTime PosixFileSystem::getLastModifiedTime(const String& path) const
{
	struct stat myStat;
	if(::stat(GetPosixFilename(path).c_str(), &myStat)==0)
	{
		return DateTime::FromAnsiTime(myStat.st_mtime, 0 /*no uSeconds*/);
	}
	TranslateCodeToException(0, path);
	return DateTime(); // never reached
}

//==============================================================================
// PosixFileSystem::getLength
//
//==============================================================================
size_t PosixFileSystem::getLength(const String& path) const
{
	struct stat myStat;

	if(::stat(GetPosixFilename(path).c_str(), &myStat)==0)
	{
		return myStat.st_size;
	}
	TranslateCodeToException(0, path);
	return 0; // never reached
}

//==============================================================================
// PosixFileSystem::openFile
//
//==============================================================================
AutoPtr<FileDescriptor> PosixFileSystem::openFile(const String& path,
                                                 int accessMode,
                                                 CreationDisp creationDisp,
                                                 int attributes) const
{
	int flags = 0;
	int permissionFlags = 0;

	if((accessMode & ReadAccess) && (accessMode & WriteAccess))  flags |= O_RDWR;
	else if(accessMode & ReadAccess)  flags |= O_RDONLY;
	else if(accessMode & WriteAccess) flags |= O_WRONLY;
	else
	{
		throw IllegalArgumentException(QC_T("invalid access mode"));
	}

	switch(creationDisp)
	{
		case OpenExisting:
			if(accessMode & WriteAccess) flags |= O_APPEND;
			break;
		case OpenCreateAppend:
			flags |= O_CREAT | O_APPEND;
			break;
		case OpenCreateExclusive:
			flags |= O_CREAT | O_EXCL;
			break;
		case OpenCreateTruncate:
			flags |= O_CREAT | O_TRUNC;
			break;
	}

	if(creationDisp == OpenCreateExclusive || creationDisp == OpenCreateTruncate)
	{
		if(attributes & ReadOnly)
			permissionFlags = S_IRUSR;
		else
			permissionFlags = (S_IRUSR | S_IWUSR);

#ifndef WIN32
		// Under Unix it is standard to give read permissions
		// to the group and others
		permissionFlags |= (S_IROTH | S_IRGRP);
#endif

	}

	int fd = ::open(GetPosixFilename(path).c_str(), flags, permissionFlags);

	if(Tracer::IsEnabled())
	{
		String traceMsg = QC_T("open: ");
		traceMsg += path;
		traceMsg += QC_T(", flags=");
		traceMsg += NumUtils::ToString(flags);
		SystemUtils::TraceSystemCall(Tracer::IO, Tracer::Medium, traceMsg, (fd==-1));
	}

	if(fd == -1)
	{
		TranslateCodeToException(0, path);
	}

#ifndef CREATEFILE_SETS_MODTIME

	// Some platforms do not set the file's modification time to the current time when it is
	// first created.  This is unusual (only darwin so far), so for consitency with other
	// platforms QuickCPP sets the modtime to the current time.

	if(::utime(GetPosixFilename(path).c_str(), 0) != 0)
	{
		TranslateCodeToException(0, path);
	}

#endif

	return new PosixFileDescriptor(this, fd, true);
}

//==============================================================================
// PosixFileSystem::closeFile
//
//==============================================================================
void PosixFileSystem::closeFile(FileDescriptor* pFD) const
{
	if(!pFD) throw NullPointerException();
	PosixFileDescriptor* pMyFD = static_cast<PosixFileDescriptor*>(pFD);

	if(::close(pMyFD->getFD()) != 0)
	{
		throw IOException(SystemUtils::GetSystemErrorString());
	}
}

//==============================================================================
// PosixFileSystem::list
//
//==============================================================================
std::list<String> PosixFileSystem::listDirectory(const String& path) const
{
	QC_DBG_ASSERT(getFileAttributeFlags(path) & Directory);
	std::list<String> ret;

#ifndef WIN32

	DIR* pDir = ::opendir(GetPosixFilename(path).c_str());

	if(pDir)
	{
		struct dirent* pEntry;
		
		while( (pEntry=::readdir(pDir)) != NULL)
		{
			String foundPath = StringUtils::FromNativeMBCS(pEntry->d_name);
			if(foundPath != QC_T(".") && foundPath != QC_T(".."))
			{
				ret.push_back(foundPath);
			}
		}

		::closedir(pDir);
	}

#endif //WIN32

	return ret;
}

//==============================================================================
// PosixFileSystem::createDirectory
//
//==============================================================================
void PosixFileSystem::createDirectory(const String& path) const
{

#if defined(WIN32)
	
	int rc = mkdir(GetPosixFilename(path).c_str());

#else

	int mode = 0777;
	int rc = mkdir(GetPosixFilename(path).c_str(), mode);

#endif

	if(Tracer::IsEnabled())
	{
		String traceMsg = QC_T("mkdir: ");
		traceMsg += path;
		SystemUtils::TraceSystemCall(Tracer::IO, Tracer::Medium, traceMsg, rc);
	}

	if(rc != 0)
	{
		TranslateCodeToException(0, path);
	}
}

//==============================================================================
// PosixFileSystem::rename
//
//==============================================================================
void PosixFileSystem::rename(const String& oldPath, const String& newPath) const
{
	int rc = ::rename(GetPosixFilename(oldPath).c_str(),
	                  GetPosixFilename(newPath).c_str());

	if(Tracer::IsEnabled())
	{
		String traceMsg = QC_T("rename: ");
		traceMsg += oldPath;
		traceMsg += QC_T(",");
		traceMsg += newPath;
		SystemUtils::TraceSystemCall(Tracer::IO, Tracer::Medium, traceMsg, rc);
	}

	if(rc != 0)
	{
		TranslateCodeToException(0, sNull);
	}
}

//==============================================================================
// PosixFileSystem::setLastModifiedTime
//
//==============================================================================
void PosixFileSystem::setLastModifiedTime(const String& path, const DateTime& time) const
{
	if(!time.isValid()) throw IllegalArgumentException();

#ifdef _MSC_VER
	_utimbuf buf;
#else
	struct utimbuf buf;
#endif

	DateTime now = DateTime::GetSystemTime();
	buf.actime  = now.toAnsiTime();
	buf.modtime = time.toAnsiTime();
	
#ifdef _MSC_VER
	int rc = ::_utime(GetPosixFilename(path).c_str(), &buf);
#else
	int rc = ::utime(GetPosixFilename(path).c_str(), &buf);
#endif
	
	if(rc != 0)
	{
		TranslateCodeToException(0, path);
	}
}

//==============================================================================
// PosixFileSystem::setReadOnly
//
//==============================================================================
void PosixFileSystem::setReadOnly(const String& path, bool bReadOnly) const
{
	struct stat myStat;
	if(::stat(GetPosixFilename(path).c_str(), &myStat) != 0)
	{
		TranslateCodeToException(0, path);
	}
	int mode = myStat.st_mode;

	if(bReadOnly)
	{
#ifdef WIN32
		mode &= ~(S_IWUSR);
#else
		mode &= ~(S_IWUSR | S_IWOTH | S_IWGRP);
#endif
	}
	else
	{
		mode |= (S_IWUSR);
	}
	if(::chmod(GetPosixFilename(path).c_str(), mode) != 0)
	{
		TranslateCodeToException(0, path);
	}
}

//==============================================================================
// PosixFileSystem::getConsoleFD
//
//==============================================================================
AutoPtr<FileDescriptor> PosixFileSystem::getConsoleFD(ConsoleStream stream) const
{
	return new PosixFileDescriptor(this, stream, false);
}

//==============================================================================
// PosixFileSystem::readFile
//
//==============================================================================
size_t PosixFileSystem::readFile(FileDescriptor* pFD, Byte* pBuffer, size_t bufLen) const
{
	if(!pFD) throw NullPointerException();
	if(!pBuffer) throw NullPointerException();

	PosixFileDescriptor* pMyFD = static_cast<PosixFileDescriptor*>(pFD);

	long bytesRead = ::read(pMyFD->getFD(), pBuffer, bufLen);
	if(bytesRead < 0)
	{
		throw IOException(SystemUtils::GetSystemErrorString());
	}
	return bytesRead;
}

//==============================================================================
// PosixFileSystem::writeFile
//
//==============================================================================
void PosixFileSystem::writeFile(FileDescriptor* pFD, const Byte* pBuffer, size_t bufLen) const
{
	if(!pFD) throw NullPointerException();
	if(!pBuffer) throw NullPointerException();

	PosixFileDescriptor* pMyFD = static_cast<PosixFileDescriptor*>(pFD);

	int bytesWritten = ::write(pMyFD->getFD(), pBuffer, bufLen);
	
	if(bytesWritten != (int)bufLen)
	{
		throw IOException(SystemUtils::GetSystemErrorString());
	}
}

//==============================================================================
// PosixFileSystem::GetPosixFilename
//
//==============================================================================
ByteString PosixFileSystem::GetPosixFilename(const String& path)
{
	return StringUtils::ToNativeMBCS(path);
}

//==============================================================================
// PosixFileSystem::canonicalize
//
//==============================================================================
String PosixFileSystem::canonicalize(const String& path) const
{
	// Under case-sensitive systems there is no case folding or
	// short-name to long-name translation, so just return the
	// shortest form a provided by File::GetCanonicalPath
	//
	return File::GetCanonicalPath(path);
}

//==============================================================================
// PosixFileSystem::TranslateCodeToException
//
//==============================================================================
void PosixFileSystem::TranslateCodeToException(int errCode, const String& msg)
{
	if(errCode==0) errCode = errno;

	switch (errCode)
	{
	case ENOENT:
		throw FileNotFoundException(msg);
	case EEXIST:
		throw ExistingFileException(msg);
	default:
		{
			String exceptionMsg;
			if(!msg.empty())
			{
				exceptionMsg = msg + QC_T(" (");
				exceptionMsg += SystemUtils::GetSystemErrorString(errCode);
				exceptionMsg += QC_T(")");
			}
			else
			{
				exceptionMsg = SystemUtils::GetSystemErrorString(errCode);
			}

			throw IOException(exceptionMsg);
		}
	}
}

QC_IO_NAMESPACE_END

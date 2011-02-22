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

#ifndef QC_IO_Win32FileSystem_h
#define QC_IO_Win32FileSystem_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "FileSystem.h"

#include "QcCore/base/ArrayAutoPtr.h"

QC_IO_NAMESPACE_BEGIN

class QC_IO_PKG Win32FileSystem : public FileSystem
{
public:

	virtual String getCurrentDirectory() const;
	virtual CharType getSeparatorChar() const;
	virtual size_t getPrefixLength(const String& path) const;
	using FileSystem::resolve;
    virtual String resolve(const String& path) const;
	virtual bool isAbsolute(const String& path) const;
	virtual bool isCaseSensitive() const;
	virtual String canonicalize(const String& path) const;
	virtual int getFileAttributeFlags(const String& path) const;
	bool checkAccess(const String& path, AccessMode mode) const;
	virtual DateTime getLastModifiedTime(const String& path) const;
	virtual size_t getLength(const String& path) const;

	virtual AutoPtr<FileDescriptor> openFile(const String& path,
	                                        int accessMode,
	                                        CreationDisp creationDisp,
	                                        int attributes) const;

	virtual void closeFile(FileDescriptor* pFD) const;
    virtual void deleteFile(const String& path) const;
	virtual std::list<String> listDirectory(const String& path) const;
	virtual void createDirectory(const String& path) const;
	virtual void rename(const String& path1, const String& path2) const;
	virtual void setLastModifiedTime(const String& path, const DateTime& time) const;
	virtual void setReadOnly(const String& path, bool bReadOnly) const;
	virtual AutoPtr<FileDescriptor> getConsoleFD(ConsoleStream stream) const;
	virtual size_t readFile(FileDescriptor* pFD, Byte* pBuffer, size_t bufLen) const;
	virtual void writeFile(FileDescriptor* pFD, const Byte* pBuffer, size_t bufLen) const;

private:
	typedef ArrayAutoPtr<TCHAR> TCharPtr;
	static TCharPtr GetWin32Filename(const String& path);
	static bool IsNotFoundError(DWORD errCode);
	static void FindExistingFile(const String& path, WIN32_FIND_DATA* pFindData);
	static void TranslateCodeToException(DWORD errCode, const String& msg);
};

QC_IO_NAMESPACE_END

#endif //QC_IO_Win32FileSystem_h

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

#ifndef QC_IO_FileSystem_h
#define QC_IO_FileSystem_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "FileDescriptor.h"
#include <list>

QC_IO_NAMESPACE_BEGIN

class QC_IO_PKG FileSystem : public virtual ManagedObject
{
public:

	enum {EndOfFile=-1 /*!< end of file reached */ };

	static AutoPtr<FileSystem> GetFileSystem();
	static void SetFileSystem(FileSystem* pFileSystem);

	virtual String getCurrentDirectory() const=0;
	virtual String getSeparator() const;
	virtual CharType getSeparatorChar() const=0;
	virtual String normalize(const String& path) const;
	virtual size_t getPrefixLength(const String& path) const=0;
    virtual String resolve(const String& path) const;
	virtual String resolve(const String& parent, const String& child) const;
    virtual bool isAbsolute(const String& path) const=0;
	virtual bool isCaseSensitive() const=0;
	virtual String canonicalize(const String& path) const;
	
	enum Attribute {Exists      = 0x01 /*!< File exists */,
	                RegularFile = 0x02 /*!< A regular file i.e. not a directory */,
	                Directory   = 0x04 /*!< File is a directory */,
	                Hidden      = 0x08 /*!< File is hidden */,
					ReadOnly    = 0x10 /*!< File is read-only for the current process */};

	virtual int getFileAttributeFlags(const String& path) const=0;

	enum AccessMode {ReadAccess  = 0x01 /*!< Request read access */,
	                 WriteAccess = 0x02 /*!< Request write access */};

	virtual bool checkAccess(const String& path, AccessMode mode) const=0;

	virtual DateTime getLastModifiedTime(const String& path) const=0;
	virtual size_t getLength(const String& path) const=0;

	enum CreationDisp {OpenExisting         /*!< open existing file only */,
	                   OpenCreateAppend     /*!< open existing or create new, preserve existing contents */,
	                   OpenCreateExclusive  /*!< create non-existing file only */,
	                   OpenCreateTruncate   /*!< open existing or create new, destroy existing contents */ };

	virtual AutoPtr<FileDescriptor> openFile(const String& path,
	                                        int accessMode,
	                                        CreationDisp creationDisp,
	                                        int attributes) const=0;

	virtual void closeFile(FileDescriptor* pFD) const=0;
    virtual void deleteFile(const String& path) const=0;
	virtual std::list<String> listDirectory(const String& path) const=0;
	virtual void createDirectory(const String& path) const=0;
	virtual void rename(const String& path1, const String& path2) const =0;
	virtual void setLastModifiedTime(const String& path, const DateTime& time) const =0;
	virtual void setReadOnly(const String& path, bool bReadOnly) const =0;

	enum ConsoleStream {Stdin  = 0   /*!< standard input */,
	                    Stdout = 1   /*!< standard output */,
						Stderr = 2   /*!< standard error */};

	virtual AutoPtr<FileDescriptor> getConsoleFD(ConsoleStream stream) const =0;
	virtual size_t readFile(FileDescriptor* pFD, Byte* pBuffer, size_t bufLen) const =0;
	virtual void writeFile(FileDescriptor* pFD, const Byte* pBuffer, size_t bufLen) const =0;

private:
	static FileSystem* QC_MT_VOLATILE s_pFileSystem;
};

QC_IO_NAMESPACE_END

#endif //QC_IO_FileSystem_h


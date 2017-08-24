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

#ifndef QC_IO_File_h
#define QC_IO_File_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "FileSystem.h"
#include "QcCore/util/DateTime.h"

#include <list>

QC_IO_NAMESPACE_BEGIN

using util::DateTime;

class QC_IO_PKG File : public virtual QCObject
{
public:
	File(const File& parent, const String& child);
	File(const String& pathname);
	File(const String& parent, const String& child);
	File(const File& rhs);
	
	bool operator==(const File& rhs) const;
	bool operator!=(const File& rhs) const;
	
	bool canRead() const;
	bool canWrite() const;
	void deleteFile() const;
	bool exists() const;
	String getAbsolutePath() const;
	String getCanonicalPath() const;
	DateTime lastModified() const;
	String getName() const;
	String getParent() const;
	File getParentFile() const;
	String getPath() const;
	bool isAbsolute() const;
	bool isDirectory() const;
	bool isFile() const;
	size_t length() const;
	std::list<String> listDirectory() const;
	void createNewFile() const;
	void mkdir() const;
	void mkdirs() const;
	void renameTo(const File& file) const;

	void setLastModified(const DateTime& time) const;
	void setReadOnly(bool bReadOnly) const;

	static CharType GetSeparatorChar();
	static String GetSeparator();
	static String GetCurrentDirectory();

	static String GetCanonicalPath(const String& path);

private: // implementation routines

	static String NormalizePath(const String& path);
	static bool IsAbsolutePath(const String& path);

private: // not implemented
	File& operator=(const File& rhs);

private: // member variables
	AutoPtr<FileSystem> m_rpFS;
	String m_path;
	size_t m_prefixLength;
};

QC_IO_NAMESPACE_END

#endif //QC_IO_File_h


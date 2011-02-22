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
// 
//==============================================================================

#ifndef QC_IO_FileOutputStream_h
#define QC_IO_FileOutputStream_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "OutputStream.h"
#include "FileDescriptor.h"

QC_IO_NAMESPACE_BEGIN

class File;

class QC_IO_PKG FileOutputStream : public OutputStream
{
public:
	FileOutputStream(const File& file);
	FileOutputStream(const String& name);
	FileOutputStream(const String& name, bool bAppend);
	FileOutputStream(FileDescriptor* pFD);

	virtual void close();

#ifdef QC_USING_DECL_BROKEN
	virtual void write(Byte x) {OutputStream::write(x);}
#else
	using OutputStream::write; 	// unhide inherited write method
#endif

	virtual void write(const Byte* pBuffer, size_t bufLen);
	
	AutoPtr<FileDescriptor> getFD() const;

private:
	FileOutputStream(const FileOutputStream& rhs);            // cannot be copied
	FileOutputStream& operator=(const FileOutputStream& rhs); // nor assigned

	void open(const String& fileName, bool bAppend);

private:
	AutoPtr<FileDescriptor> m_rpFD;
};

QC_IO_NAMESPACE_END

#endif //QC_IO_FileOutputStream_h




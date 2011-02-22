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
// Class: Win32FileDescriptor
// 
//==============================================================================

#ifndef QC_IO_Win32FileDescriptor_h
#define QC_IO_Win32FileDescriptor_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "FileDescriptor.h"

QC_IO_NAMESPACE_BEGIN

class QC_IO_PKG Win32FileDescriptor : public FileDescriptor
{
public:
	Win32FileDescriptor(const FileSystem* pFS, HANDLE hFile, bool bAutoClose);
	virtual ~Win32FileDescriptor();

	HANDLE getHandle();
	virtual String toString() const;

private:
	HANDLE m_hFile;
};

QC_IO_NAMESPACE_END

#endif //QC_IO_Win32FileDescriptor_h

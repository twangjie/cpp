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

#ifdef WIN32

#include "Win32FileDescriptor.h"
#include "FileSystem.h"

#include "QcCore/base/NumUtils.h"

QC_IO_NAMESPACE_BEGIN

Win32FileDescriptor::Win32FileDescriptor(const FileSystem* pFS, HANDLE hFile, bool bAutoClose) :
	FileDescriptor(pFS, bAutoClose),
	m_hFile(hFile)
{
}

Win32FileDescriptor::~Win32FileDescriptor()
{
	if(getAutoClose())
	{
		try
		{
			close();
		}
		catch(Exception& /*e*/)
		{
		}
	}
}

HANDLE Win32FileDescriptor::getHandle()
{
	return m_hFile;
}

String Win32FileDescriptor::toString() const
{
	return NumUtils::ToString((DWORD)m_hFile);
}

QC_IO_NAMESPACE_END
#endif //WIN32


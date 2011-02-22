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
// Class: PosixFileDescriptor
// 
//==============================================================================

#include "PosixFileDescriptor.h"
#include "FileSystem.h"

#include "QcCore/base/NumUtils.h"

QC_IO_NAMESPACE_BEGIN

PosixFileDescriptor::PosixFileDescriptor(const FileSystem* pFS, int fd, bool bAutoClose) :
	FileDescriptor(pFS, bAutoClose),
	m_fd(fd)
{
}

PosixFileDescriptor::~PosixFileDescriptor()
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

int PosixFileDescriptor::getFD()
{
	return m_fd;
}

String PosixFileDescriptor::toString() const
{
	return NumUtils::ToString(m_fd);
}

QC_IO_NAMESPACE_END

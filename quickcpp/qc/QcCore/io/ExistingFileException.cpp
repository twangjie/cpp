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
// Class ExistingFileException
/**
	@class qc::io::ExistingFileException

    @brief Thrown when attempting to create a file exclusively
	and the file already exists.
	
	@sa File::createNewFile
*/
//==============================================================================

#include "ExistingFileException.h"
#include "messages.h"

#include "QcCore/base/System.h"

QC_IO_NAMESPACE_BEGIN

String ExistingFileException::getExceptionType() const
{
	return QC_T("ExistingFileException");
}

String ExistingFileException::getLocalizedDescription() const
{
	return System::GetSysMessage(QC_IO_MSG, EIO_FILEEXISTS, "file already exists");
}

QC_IO_NAMESPACE_END


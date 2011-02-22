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
// Class: Console
/**
	@class qc::io::Console
	
	@brief Class module providing access to the standard console i/o streams:
	In (@c stdin), Out (@c stdout) and Err (@c stderr).

	It cannot be instantiated - all methods are static.

    @mtsafeClass
*/
//==============================================================================

#include "Console.h"

#include "FileOutputStream.h"
#include "FileInputStream.h"
#include "FileDescriptor.h"
#include "InputStreamReader.h"
#include "FileSystem.h"
#include "PrintWriter.h"

#include "QcCore/base/System.h"
#include "QcCore/base/ObjectManager.h"

#include <memory>

QC_IO_NAMESPACE_BEGIN

using namespace io;

//==================================================================
// Multi-threaded locking strategy
//
// To ensure that singleton resources are not created by multiple
// concurrent threads, the pointers are protected using a
// single static mutex.
//
// To minimize the time that the ConsoleMutex is held, we do
// not lock the mutex for the creation of a singleton object,
// only when a complete object has been created and we need to swap
// the address into our static pointer variable.
//==================================================================
#ifdef QC_MT
	FastMutex ConsoleMutex;
#endif //QC_MT

InputStreamReader* QC_MT_VOLATILE Console::s_pIn = 0;
PrintWriter* QC_MT_VOLATILE Console::s_pOut = 0;
PrintWriter* QC_MT_VOLATILE Console::s_pErr = 0;

//==============================================================================
// Console::Out
//
/**
   Returns a PrintWriter connected to the @c stdout file handle.
   @mtsafe
*/
//==============================================================================
AutoPtr<PrintWriter> Console::Out()
{
	//==================================================================
	// Multi-threaded locking strategy
	//
	// This uses the "double-checked locking pattern" (Schmidt 1996)
	// with a volatile storage member to minimise race conditions due
	// to the so-called "relaxed memory model"..
	//==================================================================
	if(!s_pOut)
	{
		QC_AUTO_LOCK(FastMutex, ConsoleMutex);
		// test again now that mutex has been locked
		if(!s_pOut)
		{
			s_pOut = new PrintWriter(
				new FileOutputStream(FileDescriptor::Out().get()), false);
			System::GetObjectManager().registerObject(s_pOut);
		}
	}

	return s_pOut;
}

//==============================================================================
// Console::Err
//
/**
   Returns a PrintWriter connected to the @c stderr file handle.
   
   @mtsafe
*/
//==============================================================================
AutoPtr<PrintWriter> Console::Err()
{
	//==================================================================
	// Multi-threaded locking strategy
	//
	// This uses the "double-checked locking pattern" (Schmidt 1996)
	// with a volatile storage member to minimise race conditions due
	// to the so-called "relaxed memory model"..
	//==================================================================
	if(!s_pErr)
	{
		QC_AUTO_LOCK(FastMutex, ConsoleMutex);
		// test again now that mutex has been locked
		if(!s_pErr)
		{
			s_pErr = new PrintWriter(
				new FileOutputStream(FileDescriptor::Err().get()), true);
			System::GetObjectManager().registerObject(s_pErr);
		}
	}

	return s_pErr;
}

//==============================================================================
// Console::In
//
/**
   Returns an InputStreamReader connected to the @c stdin
   file handle.

   @mtsafe
*/
//==============================================================================
AutoPtr<InputStreamReader> Console::In()
{
	//==================================================================
	// Multi-threaded locking strategy
	//
	// This uses the "double-checked locking pattern" (Schmidt 1996)
	// with a volatile storage member to minimise race conditions due
	// to the so-called "relaxed memory model"..
	//==================================================================
	if(!s_pIn)
	{
		QC_AUTO_LOCK(FastMutex, ConsoleMutex);
		// test again now that mutex has been locked
		if(!s_pIn)
		{
			s_pIn = new InputStreamReader(
				new FileInputStream(FileDescriptor::In().get()));
			System::GetObjectManager().registerObject(s_pIn);
		}
	}

	return s_pIn;
}

QC_IO_NAMESPACE_END

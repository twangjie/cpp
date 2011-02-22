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
// Class: SystemMonitor
// 
/**
	@class qc::SystemMonitor
	
	@brief Application helper class to terminate the @QuickCPP library cleanly.

    Normally instantiated in the @c main() function, SystemMonitor uses its destructor
    to ensure a clean shutdown of the application by calling System::Terminate().

    @code
    #include "QcCore/base/SystemMonitor.h"
    using namespace qc;

    int main(int argc, char* argv[])
    {
        // create a SystemMonitor instance to ensure clean termination
        SystemMonitor monitor;
        
        // do anything here...

        return (0);
    }
    @endcode

	@sa System::Terminate()
*/
//==============================================================================

#include "SystemMonitor.h"
#include "System.h"
#include "Thread.h"

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// SystemMonitor::~SystemMonitor
//
/**
   Destructor.

   Performs any essential termination of the @QuickCPP library, including
   calling the System::Terminate() function.
*/
//==============================================================================
SystemMonitor::~SystemMonitor()
{
	try
	{
		System::Terminate();
	}
	catch(...)
	{
	}
}

QC_BASE_NAMESPACE_END

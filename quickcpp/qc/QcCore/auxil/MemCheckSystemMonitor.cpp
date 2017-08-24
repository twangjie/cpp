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
// Class: MemCheckSystemMonitor
// 
/**
	@class qc::auxil::MemCheckSystemMonitor
	
	@brief Auxiliary helper class to terminate the @QuickCPP library cleanly and
	       employ the memory-leak detection facilities of the C runtime library
	       (if any such facilities exist on a given platform)
	       to aid in application memory leak detection.

	Normally instantiated in the @c main() function, this class uses its constructor
	to initialize memory leak detection in the C runtime library.

	This class derives from SystemMonitor, so its destructor will 
	facilitate a clean shutdown of the application by calling System::Terminate().

    @code
    #include "QcCore/auxil/MemCheckSystemMonitor.h"
    using namespace qc;
    using namespace qc::auxil;

    int main(int argc, char* argv[])
    {
        // create a MemCheckSystemMonitor instance to ensure clean termination
        // and detect memory leaks on some platforms
        MemCheckSystemMonitor monitor;
        
        // create a deliberate memory leak...
        int* pInt = new int;

        return (0);
    }
    @endcode

	@note The only compiler currently supplying memory-leak detection within the
	      C-runtime library is Visual C++ under Microsoft Windows.  When running a 
	      program under the Visual Studio debugger, messages will be produced during
	      program termination to indicate the existence of any detected memory leaks.
	      The @c MemCheckSystemMonitor class does not currently perform 
	      memory-leak detection on any other platforms/compilers.
	@sa SystemMonitor
*/
//==============================================================================

#include "MemCheckSystemMonitor.h"

#ifdef _MSC_VER
#include <crtdbg.h>
#endif

QC_AUXIL_NAMESPACE_BEGIN

//==============================================================================
// MemCheckSystemMonitor::~MemCheckSystemMonitor
//
/**
   Constructor.

   Under some platforms (e.g. Visual C++ under Windows), the constructor
   initializes the C-runtime library to perform memory-leak detection at 
   application termination.
*/
//==============================================================================
MemCheckSystemMonitor::MemCheckSystemMonitor()
{

#ifdef _MSC_VER
	int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(tmpDbgFlag);
	// use _CrtSetBreakAlloc(nAlloc) to trap a memory allocation;
#endif 

}

QC_AUXIL_NAMESPACE_END

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

#ifndef QC_IO_Console_h
#define QC_IO_Console_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "PrintWriter.h"
#include "InputStreamReader.h"

QC_IO_NAMESPACE_BEGIN

class QC_IO_PKG Console
{
public:

	static AutoPtr<PrintWriter> Out();
	static AutoPtr<PrintWriter> Err();
	static AutoPtr<InputStreamReader> In();

	static PrintWriter& cout();
	static PrintWriter& cerr();
	static InputStreamReader& cin();

private: // not implemented
	Console(const Console& rhs);            // cannot be copied
	Console& operator=(const Console& rhs); // nor assigned
	friend class NO_FRIENDS;                // to satisfy gcc 2.91

private:
	static PrintWriter* QC_MT_VOLATILE s_pOut;
	static PrintWriter* QC_MT_VOLATILE s_pErr;
	static InputStreamReader* QC_MT_VOLATILE s_pIn;
};

//==============================================================================
// Console::cout
//
/**
Returns a raw C++ reference to the PrintWriter which is connected
to the @c stdout file handle.

This function has been provided to make it more convenient to use
the C++ IO operators on the standard output streams.

@code
int age = 5;
Console::cout() << QC_T("I am ") << age
<< QC_T(" years old") << endl;
@endcode

It is unusual for a @QuickCPP function to return a raw C++ reference
to a ManagedObject.  However, in this case, @QuickCPP will ensure that
the PrintWriter object exists for the lifetime of the application,
thereby removing the need for application code to maintain
the reference count.

This function is also remarkable because it deviates from the @QuickCPP
convention of starting static member functions with upper case.  This
is to be consistent with the standard C++ @c ostream called @c cout.
*/
//==============================================================================
inline PrintWriter& Console::cout()
{
	return *Out().get();
}

//==============================================================================
// Console::cerr
//
/**
Returns a raw C++ reference to the PrintWriter which is connected
to the @c stderr file handle.

This function has been provided to make it more convenient to use
the C++ IO operators on the standard output streams.

@code
try
{
...
}
catch(Exception& e)
{
Console::cerr() << e.toString() << endl;
}
@endcode

It is unusual for a @QuickCPP function to return a raw C++ reference
to a ManagedObject.  However, in this case, @QuickCPP will ensure that
the PrintWriter object exists for the lifetime of the application,
thereby removing the need for application code to maintain
the reference count.

This function is also remarkable because it deviates from the @QuickCPP
convention of starting static member functions with upper case.  This
is to be consistent with the standard C++ ostream called @c cerr.
*/
//==============================================================================
inline PrintWriter& Console::cerr()
{
	return *Err().get();
}

//==============================================================================
// Console::cin
//
/**
Returns a raw C++ reference to the InputStreamReader which is connected
to the @c stdin file handle.

It is unusual for a @QuickCPP function to return a raw C++ reference
to a ManagedObject.  However, in this case, @QuickCPP will ensure that
the InputStreamReader object exists for the lifetime of the application,
thereby removing the need for application code to maintain
the reference count.

This function is also remarkable because it deviates from the @QuickCPP
convention of starting static member functions with upper case.  This
is to be consistent with the standard C++ ostream called @c cin.
*/
//==============================================================================
inline InputStreamReader& Console::cin()
{
	return *In().get();
}

QC_IO_NAMESPACE_END

#endif //QC_IO_Console_h

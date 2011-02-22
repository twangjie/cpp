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
// Class: ObjectManager
// 
// Overview
// --------
// The ObjectManager class is used as a surrogate parent for objects that
// have a lifetime that is not contained by the lifetime of the creating object.
//
// This is typically the case when global objects are lazily created on first use
// but then live on until the application terminates.  It is sometimes
// necessary to keep track of and delete these objects so that memory leak
// detection software does not report false errors.
//
// The System is thus able to aid in the tracking-down of memory
// leaks by ensuring that the library is well behaved.
//
// An instance of the ObjectManager is available from the System 
// object via System::GetObjectManager().
//
//==============================================================================

#ifndef QC_BASE_ObjectManager_h
#define QC_BASE_ObjectManager_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include <list>
#include "AutoPtr.h"
#include "FastMutex.h"

QC_BASE_NAMESPACE_BEGIN

class ManagedObject;

class QC_BASE_PKG ObjectManager
{
	friend class System;

public:
	
	void registerObject(ManagedObject* pObject);
	void unregisterObject(ManagedObject* pObject);

private:
	void unregisterAllObjects();

private:
	typedef std::list<AutoPtr<ManagedObject> > ObjectList;
	ObjectList m_list;

#ifdef QC_MT
	FastMutex m_mutex;
#endif //QC_MT
};

QC_BASE_NAMESPACE_END

#endif //QC_BASE_ObjectManager_h


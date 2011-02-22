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
// Class: ManagedObject
//
//==============================================================================

#ifndef QC_BASE_ManagedObject_h
#define QC_BASE_ManagedObject_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include "AtomicCounter.h"
#include "debug.h"

QC_BASE_NAMESPACE_BEGIN

class QC_BASE_PKG ManagedObject
{
public:

	ManagedObject();
	ManagedObject(const ManagedObject& rhs);
	ManagedObject& operator=(const ManagedObject& rhs); 

	virtual ~ManagedObject();

	void addRef();
	void release();
	virtual void onFinalRelease();
	unsigned long getRefCount() const;

private:


private:

#if defined(QC_MT)
	AtomicCounter m_refCount;
#else
	unsigned long m_refCount;
#endif
};

//==============================================================================
// ManagedObject::addRef
//
/**
Increments the reference-count of the object.

In multi-threaded versions of the library, the reference-count is
incremented using an atomic, thread-safe operation.

In the debug build, an assertion is made that the reference-count is 
not equal to zero after it has been incremented.  This check can help
to trap errors where an object has been erroneously deleted.

@sa release()
*/
//==============================================================================
inline
	void ManagedObject::addRef()
{
	++m_refCount;
	QC_DBG_ASSERT(m_refCount != 0);
}

//==============================================================================
// ManagedObject::release
//
/**
Decrements the reference-count of the object.

When the reference-count becomes zero, the virtual function onFinalRelease()
is called which will normally delete the object.

In multi-threaded versions of the library, the reference-count is
decremented using an atomic, thread-safe operation.
@sa addRef()
*/
//==============================================================================
inline
	void ManagedObject::release()
{
	QC_DBG_ASSERT(m_refCount != 0);

	if(--m_refCount == 0)
		onFinalRelease();
}

QC_BASE_NAMESPACE_END

#endif //QC_BASE_ManagedObject_h

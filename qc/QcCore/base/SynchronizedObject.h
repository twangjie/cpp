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
// Class: SynchronizedObject
// 
//==============================================================================

#ifndef QC_BASE_SynchronizedObject_h
#define QC_BASE_SynchronizedObject_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include "AutoLock.h"
#include "ManagedObject.h"
#include "RecursiveMutex.h"

QC_BASE_NAMESPACE_BEGIN

class QC_BASE_PKG SynchronizedObject : public virtual ManagedObject
{
public:

	/**
	Type used to create scoped locks when the lifetime of the
	SynchronizedObject is guaranteed to exceed the locked scope.
	*/
	typedef AutoLock<SynchronizedObject> Lock;

protected:
	friend class AutoLock<SynchronizedObject>;
	friend class Monitor;
	
	void lock();
	void unlock();

private:
#ifdef QC_MT
	RecursiveMutex m_mutex;
#endif //QC_MT
};

//
// Macro to aid in the creation of scoped synchronization locks
//
#ifdef QC_MT

	#define QC_SYNCHRONIZED\
		SynchronizedObject::Lock _scoped_lock_(const_cast<SynchronizedObject&>(*(static_cast<const SynchronizedObject*>(this))));

	#define QC_SYNCHRONIZED_PTR(_LOCK_PTR)\
		QC_DBG_ASSERT(_LOCK_PTR!=0);\
		SynchronizedObject* _p_object_ = _LOCK_PTR;\
		SynchronizedObject::Lock _scoped_lock_(*_p_object_);

	#define QC_SYNCHRONIZED_PTR_ADD(_LOCK_PTR)\
		QC_DBG_ASSERT(_LOCK_PTR!=0);\
		SynchronizedObject* _p_object_ = _LOCK_PTR;\
		AutoPtr<SynchronizedObject> _rp_object_(_p_object_);\
		SynchronizedObject::Lock _scoped_lock_(*_p_object_);

#else

	#define QC_SYNCHRONIZED
	#define QC_SYNCHRONIZED_PTR(_LOCK_PTR)
	#define QC_SYNCHRONIZED_PTR_ADD(_LOCK_PTR)

#endif //QC_MT

QC_BASE_NAMESPACE_END

#endif //QC_BASE_SynchronizedObject_h

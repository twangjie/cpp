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
// Class: Writer
// 
// Overview
// --------
// Based on the java.io.Writer class.  Provides an abstract base class
// with the capability to write characters (as opposed to bytes).
//
// Abstract class for writing to character streams. The only methods that
// a subclass must implement are write(const CharType* pStr),
// flush(), flushBuffers() and close(). Most subclasses, however, will 
// override some of the methods defined here in order to provide higher
// efficiency, additional functionality, or both. 
//
//=============================================================================

#ifndef QC_IO_Writer_h
#define QC_IO_Writer_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "QcCore/base/SynchronizedObject.h"

QC_IO_NAMESPACE_BEGIN

class QC_IO_PKG Writer : public SynchronizedObject
{
public:

	typedef CharType unit_type;

	Writer();
	Writer(SynchronizedObject* pLockObject);

	virtual void close()=0;
	virtual void flush(); 
	virtual void flushBuffers();
	virtual void write(const CharType* pStr, size_t len)=0; 
	virtual void write(CharType c);
	virtual void write(const Character& ch);
	virtual void write(const String& str);

	AutoPtr<SynchronizedObject> getLock() const;

protected:
	/**
	* The SynchronizedObject used to control concurrent
	* multi-threaded access to synchronized methods.
	*/
	AutoPtrMember<SynchronizedObject> m_rpLock;

private:
	Writer(const Writer& rhs);            // cannot be copied
	Writer& operator=(const Writer& rhs); // nor assigned
};

QC_IO_NAMESPACE_END

#endif //QC_IO_Writer_h

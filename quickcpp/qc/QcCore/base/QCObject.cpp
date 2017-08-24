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
// $Revision: 107 $
// $Date: 2011-02-23 17:22:27 +0800 (周三, 23 二月 2011) $
//
//==============================================================================
//
// Class QCObject
/**
	@class qc::QCObject
	
	@brief Common base class for all classes that rely on
	reference-counting to perform automatic object lifetime management.

    Object lifetime management (aka 'garbage collection') is the process
	of deciding when an object is no longer needed and destroying it when it is
	safe to do so. 
	
	Unlike many other languages, C++ does not offer any native 
	object lifetime management facilities - the programmer is generally responsible
	for keeping track of when objects are created and destroyed.

    While the manual approach to memory management is good and efficient for some systems,
	it can place an unnecessary burden on the programmer and some useful programming
	practices can become virtually impossible.  A case in point is the idea of 'filters'.
	A filter class implements the same interface as another class, overriding some
	methods and delegating all other calls to a contained object (see FilterInputStream
	for an example).  When using a manual approach, the user of a filter would have to decide who was
	responsible for the ownership of the contained object - the creator @a or the filter.
	Should the filter delete the object when it is itself destroyed?  What if something
	else is relying on that object elsewhere in the program?  These types of questions
	make systems development complicated.  @QuickCPP employs a reference-counting scheme
	that allows the ownership of an object to be shared by all users of it - and the 
	object is responsible for destroying itself when it is no longer referenced.

    Much of the @QuickCPP library is inspired by the Java API (JDK).
	The Java API is considerably easier to use and richer than the equivalent
	C++ standard library.  The designers of @QuickCPP believe that the JDK achieves
	much of its power and flexibility by being able to create objects <em>on the fly</em>
	in the knowledge that they will be destroyed when they are no longer used.

    <hr><h4>Reference-Counting</h4>
    The QCObject class implements a reference-counting scheme similar to the scheme
	used by the Microsoft <a href="http://www.microsoft.com/com/tech/com.asp">Compound Object Model (COM)</a>.
	Each object has a reference-count which is incremented each time a reference to the
	object is taken and decremented when the reference is no longer needed.  At the point 
	that the reference-count is finally decremented to zero the object will destroy itself.

    Using reference-counting in this way makes the behaviour of a class deterministic.
	Resources are always freed at the point when the object is no longer referenced.

    Just like in COM, this scheme puts a requirement on each client program to release
    references to objects that it has received.  However, as @QuickCPP is written
	in C++, this burden is delegated to a template class called AutoPtr<T> which
	takes care of most of the boiler plate code required to correctly perform 
	object reference-counting.

    <hr><h4>Cyclic References</h4>
    Simple reference-counting schemes generally work well, but they are subject to
	problems with cyclic references.  A cycle occurs when two objects
	reference each other either directly or indirectly, with the result that neither
	object's reference-count will ever be decremented to zero.  In this situation
	the objects in the cycle plus all those objects they reference will never be deleted.

    There are various techniques that can be employed to prevent cyclic references.
    One effective technique is to establish the part of a class which is causing
	the dependency and factor that out into another class that both objects can reference.
	In this way we go from A<-->B which is cyclic to A-->C<--B which isn't.

    Another possible technique involves the use of native pointers or references
	to classes that always have a lifetime that envelopes the referring object.

    Whichever technique is chosen, it is important to monitor applications built using
	reference-counting to ensure that resource exhaustion does not occur due to
	cyclic references.  It is a good idea to use a memory leak detector during development
	to help isolate these problems.  When using Visual C++ on the Microsoft Windows platform,
	the	MemCheckSystemMonitor class can be used to invoke the built-in memory-leak detection
	of the C run-time library.

    <hr><h4>Object Construction</h4>
    When an instance is constructed its internal reference-count is set to zero.  This is
    a deliberate choice so that objects created on the stack which never
    have another reference taken can go out of scope and be destroyed
	with a reference-count still equal to zero.  This allows the implementation
	to test for erroneous destruction by ensuring that the reference-count is zero
	at destruction.

    This scheme also allows derived class constructors to throw exceptions without
	leaving an invalid reference-count in the base object.

    Please note, however, that great care should be taken when creating a QCObject
	on the stack.  
	In particular the object should not be passed to another function that may 
	increment the reference-count.  This is because, when the reference-count is
	decremented, the object will attempt to destroy itself - which is not legal for
	stack-based objects.  Unless you are absolutely sure about how the object will be used
	it is often preferable to avoid stack-based QCObjects altogether.

    <hr><h4>Reference-counting rules</h4>
	There are a small number of rules that should be followed to ensure that
	QCObjects are used correctly.  Note that implementing these rules is made
	much easier by using the AutoPtr<> template class.

	-# Do not call addRef() or release() on objects created on the stack.
	-# Feel free to pass bare pointers to functions.  Functions can use the 
	   passed pointer freely, including passing it to other functions.  If it needs to
	   store the pointer for use beyond the lifetime of the function,
	   it must first increment the reference-count by calling addRef() (or store the
	   pointer in a AutoPtr<> variable).
	   When writing functions that accept bare pointers, it should be noted that
	   the reference-count of any objects passed to the function may be zero.  This
	   means that a single increment/decrement of the reference-count could result
	   in the object's destruction.  If the function passes the bare pointer to
	   another function which may manipulate the objects reference-count, then it
	   is recommended to take temporary ownership of the object by storing its
	   pointer in a AutoPtr<T> variable first.
	-# When returning a pointer from a function, (as a return value or a 
	   return parameter) the caller is responsible for incrementing
	   (and decrementing) the reference-count.  This behaviour
	   mimics the behaviour of new().
	   It is strongly recommended when returning pointers from
	   functions to use AutoPtr<> in place of a bare pointer.  This makes life
	   easier for the caller because he is no longer responsible for calling 
	   addRef() and release() on the returned pointer.  This also makes it possible
	   to chain function calls together like this:
	   <code>url.openConnection()->getInputStream()->read();</code>.

    <hr><h4>What should be a QCObject?</h4>
    Reference-counting is not free, so QCObjects have a run-time and storage
	cost associated with them.  For this reason some thought should be given to
	deciding what should be a QCObject.

	When deciding whether or not a class should derive from QCObject,
	the following guidelines are useful:-

    Factors indicating a preference towards QCObject:-
	- contains virtual functions (indicating polymorphic behaviour)
	- instances may need to be shared

    Factors indicating a preference towards a common class or struct:-
	- small, lightweight, instances can be passed by value

	<hr><h4>Multi-threaded Applications</h4>
	When compiled with the QC_MT flag defined, incrementing and 
	decrementing the reference-count is guaranteed to be performed in a
	thread-safe, atomic fashion.  This means that reference-counting will work
	as advertised, even when sharing QCObjects between multiple threads.

    This does not mean, however, that classes derived from QCObject are
	automatically thread-safe.  In order to be safely used 
	by multiple threads, derived classes will have to ensure that
	they protect their internal state from the potential memory corruption
	that can result from conflicting concurrent access.

    In general, with the exception of the reference-count, classes within the
	@QuickCPP library do not protect their internal state from concurrent
	multi-threaded access.

	@sa AutoPtr<>

*/
//  <hr><h4>The goals of object lifetime management</h4>
//	The QCObject reference-counting scheme was designed to:
//	-# support the sharing of objects without having to be aware of or concerned about
//	   their precise lifetime
//  -# delete objects automatically when they are no longer referenced
//  -# be simple and intuitive (but not necessarily fool-proof!)
//  -# be as inconspicuous as possible, avoiding complicating the application
//     with arcane reference-counting paraphernalia
//  -# support polymorphism by substituting sub-types in place of base types
//  -# enable passing the return value of one function as a parameter to another
//     e.g.  @c doSomething(getSomething())
//  -# enable the chaining of function calls together e.g. @c getHouse()->getDoor()->getColor()
//  -# support the sharing of objects between multiple threads
//==============================================================================

#include "QCObject.h"
#include "debug.h"

QC_BASE_NAMESPACE_BEGIN

//==============================================================================
// QCObject::QCObject
//
/**
   Default constructor.

   Initializes the reference-count to zero.
*/
//==============================================================================
QCObject::QCObject()
#if !defined(QC_MT)
: m_refCount(0)
#endif
{
}

//==============================================================================
// QCObject::QCObject
//
/**
   Copy constructor.

   A compiler-generated copy constructor would be unsuitable because the
   reference-count for a new object must be initialized to zero.

   @param rhs QCObject being copied.
*/
//==============================================================================
QCObject::QCObject(const QCObject& /*rhs*/)
#if !defined(QC_MT)
: m_refCount(0)
#endif
{
}

//==============================================================================
// QCObject::operator=
//
/**
   Assignment operator.

   A compiler-generated assignment operator would be unsuitable because the
   reference-count for an object must remain unchanged.

   @param rhs QCObject being copied.
*/
//==============================================================================
QCObject& QCObject::operator=(const QCObject& /*rhs*/)
{
	return *this;
}

//==============================================================================
// QCObject::~QCObject
//
/**
   Destructor.

   In the debug build, an assertion is made that the reference-count is zero.
   This is done to trap programming errors where the reference-counting rules
   have not been correctly followed.
*/
//==============================================================================
QCObject::~QCObject()
{
	QC_DBG_ASSERT(m_refCount == 0);
	// Set the reference count to -1 so that erronious use of the object
	// is likely to be caught if the object is subsequently addref'd
	m_refCount = (unsigned long)-1;
}

//==============================================================================
// QCObject::onFinalRelease
//
/**
   Virtual method called when the object's reference-count has been decremented
   to zero.

   A zero reference-count indicates that the object is no longer needed
   and should be deleted.  The default implementation calls
   @c delete @ this.  
   
   Only override this method if you need to perform some other custom garbage 
   collection.
*/
//==============================================================================
void QCObject::onFinalRelease()
{
	delete this;
}

//==============================================================================
// QCObject::getRefCount
//
/**
   Returns the current reference-count of the object.

   @returns the current reference-count.
*/
//==============================================================================
unsigned long QCObject::getRefCount() const
{
	return m_refCount;
}

QC_BASE_NAMESPACE_END

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
// Class: Tracer
//
/**
	@class qc::Tracer
	
	@brief Provides an abstraction for a run-time tracing service.

    There are many situations where it is useful to be able to
	record an application's activity while it is running.  This facility
    is usually called tracing or event logging.
	
	Traditionally, C++ applications use a pre-processor macro-based approach 
	to tracing.  The implementations are sometimes inefficient and often
	restricted to debug builds of the program.

	@QuickCPP takes a different approach.  The Tracer class provides an abstract
	interface for raising trace events and for controlling which events should
	be recorded.  We do provide an example implementation in the form of the
	TraceHelper class, but applications are free to implement their own Tracer
	class which offers the desired level of performance and functionality.  For example,
	the TraceHelper class writes trace events to a file, but it would be possible
	for other implementations to write to a memory buffer, to a console or
	both.  One important feature of @QuickCPP tracing is that it is available in
	both the debug and release builds of the library.

    The Tracer class is both a class module, containing static methods to
	raise and control the output of trace events, as well as an abstract base class which
	must be @a derived from when creating a concrete implementation.

    The static tracing methods are called from many @QuickCPP service routines
	when performing significant tasks such as creating network connections, opening
	files or starting and stopping threads.  These static methods, in turn, call
	virtual member functions which must be implemented by a concrete Tracer class.

    Finally, the @QuickCPP tracing service is extensible, allowing application code
	to be traced as well as @QuickCPP system calls.
*/
//==============================================================================

#include "Tracer.h"
#include "System.h"
#include "ObjectManager.h"
#include "FastMutex.h"
#include "StringUtils.h"

QC_BASE_NAMESPACE_BEGIN

//==================================================================
// Multi-threaded locking strategy
//
// Update access to static variables is mutex protected, but to 
// minimise the runtime cost, read access is not protected.  This
// gives an exposure to the so-called "relaxed" memory model that
// exists on some multi-processor machines.  We minimise this
// exposure by declaring the static variables as 'volatile'.
//==================================================================
#ifdef QC_MT
	FastMutex TracerMutex;
#endif //QC_MT


Tracer* QC_MT_VOLATILE Tracer::s_pTracer = 0;
QC_MT_VOLATILE bool Tracer::s_bEnabled = false;

const CharType* SectionNames[Tracer::User] = {
	QC_T("all"),
	QC_T("qc:base"),
	QC_T("qc:util"),
	QC_T("qc:io"),
	QC_T("qc:net"),
	0, /* unassigned */
	0, /* unassigned */
	0, /* unassigned */
	0, /* unassigned */
	QC_T("qc:auxil"),
	QC_T("qc:xml"),
	0, /* unassigned */
	0, /* unassigned */
	0, /* unassigned */
	0, /* unassigned */
	0  /* unassigned */ };

//==============================================================================
// Tracer::SetTracer
//
/**
   Sets the system Tracer object.

   There is only one Tracer object active at any point in time, use this method
   to register an instance of a Tracer object as the active Tracer.

   To ensure that the passed object exists for as long as the application needs
   it, the Tracer object is registered with the system's ObjectManager
   which holds a (counted) reference to it until system termination.
   @mtsafe
*/
//==============================================================================
void Tracer::SetTracer(Tracer* pTracer)
{
	if(pTracer) 
	{
		System::GetObjectManager().registerObject(pTracer);
	}

	Tracer* pExisting;

	// create a scope for the mutex lock
	{
		QC_AUTO_LOCK(FastMutex, TracerMutex);
		pExisting = s_pTracer;
		s_pTracer = pTracer;
	}

	// Note: There is no need to keep the mutex locked
	// while we unregister the old tracer
	if(pExisting) 
	{
		System::GetObjectManager().unregisterObject(pExisting);
	}
}
 
//==============================================================================
// Tracer::Trace
//
/**
   Raises a trace event.  If tracing has been enabled and a concrete Tracer class
   has been registered, its doTrace() method is called with the parameters
   passed.  The concrete implementation of Tracer which has been registered is
   responsible for processing the event.

   @param nSection the identifier of the section of code that's raising the event.
          @QuickCPP trace events use a member from the Tracer::Sections enumeration, user
		  trace events can use an assigned number starting with Tracer::User.

   @param nLevel a short integer indicating the relative importance of the trace event.
          The value should be between Tracer::Highest and Tracer::Min.  The
		  Tracer implementation can use this value when deciding how to process
		  an event.  In general, a lower number indicates a more severe condition
		  which is more likely to be processed.
   @param message a string describing the trace event

   @sa SetTracer()
   @sa doTrace()
   @mtsafe
*/
//==============================================================================
void Tracer::Trace(short nSection, short nLevel, const String& message)
{
	if(s_bEnabled && s_pTracer)
	{
		s_pTracer->doTrace(nSection, nLevel, message.data(), message.size());
	}
}

//==============================================================================
// Tracer::Trace
//
/**
   Raises a trace event.  If tracing has been enabled and a concrete Tracer class
   has been registered, its doTrace() method is called with the parameters
   passed.  The concrete implementation of Tracer which has been registered is
   responsible for processing the event.

   @param nSection the identifier of the section of code that's raising the event.
          @QuickCPP trace events use a member from the Tracer::Sections enumeration, user
		  trace events can use an assigned number starting with Tracer::User.

   @param nLevel a short integer indicating the relative importance of the trace event.
          The value should be between Tracer::Highest and Tracer::Min.  The
		  Tracer implementation can use this value when deciding how to process
		  an event.  In general, a lower number indicates a more severe condition
		  which is more likely to be processed.
   @param message a string describing the trace event
   @param len the length of the @c message string

   @sa SetTracer()
   @sa doTrace()
   @mtsafe
*/
//==============================================================================
void Tracer::Trace(short nSection, short nLevel, const CharType* message, size_t len)
{
	if(s_bEnabled && s_pTracer)
	{
		s_pTracer->doTrace(nSection, nLevel, message, len);
	}
}

//==============================================================================
// Tracer::TraceBytes
//
/**
   Raises a trace event.  If tracing has been enabled and a concrete Tracer class
   has been registered, its doTrace() method is called with the parameters
   passed.  The concrete implementation of Tracer which has been registered is
   responsible for processing the event.

   @param nSection the identifier of the section of code that's raising the event.
          @QuickCPP trace events use a member from the Tracer::Sections enumeration, user
		  trace events can use an assigned number starting with Tracer::User.

   @param nLevel a short integer indicating the relative importance of the trace event.
          The value should be between Tracer::Highest and Tracer::Min.  The
		  Tracer implementation can use this value when deciding how to process
		  an event.  In general, a lower number indicates a more severe condition
		  which is more likely to be processed.
   @param message a string describing the trace event.
   @param bytes a pointer to an array of bytes representing the detail of the event
   @param len the length of the byte array

   @sa SetTracer()
   @sa doTrace()
   @mtsafe
*/
//==============================================================================
void Tracer::TraceBytes(short nSection, short nLevel, const String& message,
                        const Byte* bytes, size_t len)
{
	if(s_bEnabled && s_pTracer)
	{
		s_pTracer->doTraceBytes(nSection, nLevel, message, bytes, len);
	}
}
	
//==============================================================================
// Tracer::Enable
//
/**
   Enables or disables tracing.

   This acts as a simple on/off switch for tracing.  Unless tracing has been
   enabled, the doTrace() method is never called.  
   
   Enable() has no effect on the activation options in effect, so it is possible
   to specify which sections of code should be traced, but not actually have
   tracing enabled.  This makes it easy to enable and disable tracing at various
   points within an application without having to deal with the individual
   activation options each time.

   @param bEnable @c true to enable tracing; @c false to disable tracing
   @sa Activate()
   @sa IsEnabled()
   @mtsafe
*/
//==============================================================================
void Tracer::Enable(bool bEnable)
{
	s_bEnabled = bEnable;
}

//==============================================================================
// Tracer::Activate
//
/**
   Informs the registered Tracer that events for the specified section with a
   level less than or equal to @c nLevel should be processed.  This passes the
   request to the registered Tracer object's virtual doActivate() method.

   A concrete Tracer implementation may maintain a table of section names together
   with the level of tracing that has been requested for each one.  For example,
   it may record that the user wants all network access to be logged, but only
   exceptions from other sections of the library.

   Note that the Tracer must be both activated and enabled before any trace events
   will be processed.

   @param nSection the identifier of the section of code that raises events.
          @QuickCPP trace events use a member from the Tracer::Sections enumeration, user
		  trace events can use an assigned number starting with Tracer::User.

   @param nLevel a short integer indicating the level to which trace events should
          be processed.  The higher the number, the more tracing that will be produced.

   @sa Enable()
   @mtsafe
*/
//==============================================================================
void Tracer::Activate(short nSection, short nLevel)
{
	if(s_pTracer)
	{
		s_pTracer->doActivate(nSection, nLevel);
	}
}

//==============================================================================
// Tracer::GetSectionName
//
/**
   Translates a numeric section identifier into a null-terminated ::CharType
   string.  The returned string will be used when formatting trace output
   for display.

   If @c nSection is within the @QuickCPP assigned range, a pre-determined
   section name is returned.  Otherwise the request is delegated to the
   registered Tracer object (if any).

   @param nSection the identifier of the section.
   @returns a null-terminated ::CharType array or 0 if the section is not
            recognized.
   @mtsafe
*/
//==============================================================================
const CharType* Tracer::GetSectionName(short nSection)
{
	if(nSection < Tracer::User)
	{
		return SectionNames[nSection];
	}
	else if(s_pTracer)
	{
		return s_pTracer->getUserSectionName(nSection);
	}
	else
	{
		return 0;
	}
}

//==============================================================================
// Tracer::getUserSectionName
//
/**
   Returns the name of the code section with the supplied identifier.

   If an application makes use of tracing in sections of user code, an identifier
   must be selected starting with the value Tracer::User.  A concrete Tracer implementation
   should be registered which will translate between the section identifier
   and its name.

   For optimum tracing efficiency, a ::CharType pointer is returned instead of
   a String.  The puts a requirement onto concrete implementations to ensure that
   the returned pointer references a valid memory location which is constant.
   Further, regard should be given to the fact that this method may
   be called concurrently from multiple threads, therefore it would not
   be appropriate to allocate a single static buffer and update its contents
   with the results from each call.

   @param nSection the section identifier for the user code section.
   @returns a pointer to a static ::CharType array or null.  The caller does
   not receive ownership of the returned array pointer and should not attempt
   to free it.
   @sa getUserSectionNumber()
   @mtsafe
*/
//==============================================================================
const CharType* Tracer::getUserSectionName(short nSection)
{
	return 0;
}

//==============================================================================
// Tracer::GetSectionNumber
//
/**
   Translates a section name into a numeric identifier.

   if @c section is a @QuickCPP assigned name, a pre-determined
   section identifier is returned.  Otherwise the request is delegated to the
   registered Tracer object (if any).

   @param section the name of the section.
   @returns a numeric section identifier or -1 if the section is not
            recognized.
   @mtsafe
*/
//==============================================================================
short Tracer::GetSectionNumber(const String& section)
{
	for(int i=0; i<Tracer::User; ++i)
	{
		if(SectionNames[i] != 0 && StringUtils::CompareNoCase(section, SectionNames[i]) == 0)
		{
			return i;
		}
	}
	if(s_pTracer)
	{
		return s_pTracer->getUserSectionNumber(section);
	}
	else
	{
		return -1;
	}
}

//==============================================================================
// Tracer::getUserSectionNumber
//
/**
   Returns the name of the code section with the supplied identifier or -1 if
   the identifier is not recognized.

   If an application makes use of tracing in sections of user code, an identifier
   must be selected starting with the value Tracer::User.  A Concrete implementation
   of Tracer should be registered which will translate between the section identifier
   and its name.

   @param section the name of the user code section.
   @sa getUserSectionName()
   @mtsafe
*/
//==============================================================================
short Tracer::getUserSectionNumber(const String& section)
{
	return -1;
}

#ifdef QC_DOCUMENTATION_ONLY
//==============================================================================
// Tracer::doTrace
//
/**
   Virtual function called in response to Trace().

   @param nSection the identifier of the section of code that's raising the event.
          @QuickCPP trace events use a member from the Tracer::Sections enumeration, user
		  trace events can use an assigned number starting with Tracer::User.

   @param nLevel a short integer indicating the relative importance of the trace event.
          The value should be between Tracer::Highest and Tracer::Min.  The
		  Tracer implementation can use this value when deciding how to process
		  an event.  In general, a lower number indicates a more severe condition
		  which is more likely to be processed.
   @param message a string describing the trace event.
   @param len the length of the @c message.
   @mtsafe
*/
//==============================================================================
void Tracer::doTrace(short nSection, short nLevel, const CharType* message, size_t len);

//==============================================================================
// Tracer::doTraceBytes
//
/**
   Virtual function called in response to Trace().

   @param nSection the identifier of the section of code that's raising the event.
          @QuickCPP trace events use a member from the Tracer::Sections enumeration, user
		  trace events can use an assigned number starting with Tracer::User.

   @param nLevel a short integer indicating the relative importance of the trace event.
          The value should be between Tracer::Highest and Tracer::Min.  The
		  Tracer implementation can use this value when deciding how to process
		  an event.  In general, a lower number indicates a more severe condition
		  which is more likely to be processed.
   @param message a string describing the trace event.
   @param bytes a pointer to an array of bytes representing the detail of the event
   @param len the length of the byte array
   @mtsafe
*/
//==============================================================================
void Tracer::doTraceBytes(short nSection, short nLevel, const String& message, const Byte* bytes, size_t len);

//==============================================================================
// Tracer::doActivate
//
/**
   Virtual function called in response to Activate().

   Concrete implementations are expected to maintain a table representing
   the sections of code which have had tracing enabled, and the level of tracing
   to be performed for each section.

   @param nSection the identifier of the section of code that raises events.
          @QuickCPP trace events use a member from the Tracer::Sections enumeration, user
		  trace events can use an assigned number starting with Tracer::User.

   @param nLevel a short integer indicating the level to which trace events should
          be processed for the specified @c section.
   @sa Activate()
   @mtsafe
*/
//==============================================================================
void Tracer::doActivate(short nSection, short nLevel);

#endif //QC_DOCUMENTATION_ONLY

QC_BASE_NAMESPACE_END

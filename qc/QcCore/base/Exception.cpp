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
// Class: Exception
// 
/**
	@class qc::Exception
	
	@brief Base class for all exceptions thrown from the @QuickCPP library.

	Functions within @QuickCPP rarely throw an exception of type @c Exception,
	but usually throw one of the many derivatives such as net::SocketException
	when a socket error occurs.  However, applications may @a catch exceptions
	of type @c Exception and extract a meaningful error message by calling the
	toString() member function.

    @code
    try
    {
        //  open a FileInputStream (may throw a FileNotFoundException)
        AutoPtr<FileInputStream> rpFIS = new FileInputStream(QC_T("test.txt"));
    }
    catch(Exception& e)
    {
        std::err << StringUtils::ToAscii(e.toString()) << std::endl;
    }
    @endcode
*/
//==============================================================================

#include "Exception.h"

QC_BASE_NAMESPACE_BEGIN

const String sColon = QC_T(": ");

//==============================================================================
// Exception::Exception
//
/**
   Constructs an Exception without any additional message description.
*/
//==============================================================================
Exception::Exception()
{
}

//==============================================================================
// Exception::Exception
//
/**
   Constructs an Exception with a description message.
   @param message a String providing further information about the exception
*/
//==============================================================================
Exception::Exception(const String& message) :
	m_message(message)
{
}

//==============================================================================
// Exception::~Exception
//
/**
   Virtual destructor.  Ensures that derived class' destructor will be called
   if a pointer to an Exception is ever deleted.  Note, however, that @QuickCPP
   always throws exceptions by value and catches them by reference.
*/
//==============================================================================
Exception::~Exception()
{
}

//==============================================================================
// Exception::setMessage
//
/**
   Sets the message text for an Exception.  Intended to be called by the
   constructors of derived exception classes.
   @param message the message text for the exception
*/
//==============================================================================
void Exception::setMessage(const String& message)
{
	m_message = message;
}

//==============================================================================
// Exception::getMessage
//
/**
   Returns the message text for this Exception.
*/
//==============================================================================
String Exception::getMessage() const
{
	return m_message;
}

//==============================================================================
// Exception::toString
//
/**
   Returns a string representation of this Exception.

   If this Exception has a non-empty message text, the return value is
   made up as follows:-
   - the description returned from getDescription()
   - a colon followed by a space (": ")
   - the message text

   If the message text is empty then toString() returns the same value as
   getDescription().
*/
//==============================================================================
String Exception::toString() const
{
	const String& msg = getMessage();

	if(msg.empty())
	{
		return getDescription();
	}
	else
	{
		return String(getDescription() + sColon + msg);
	}
}

//==============================================================================
// Exception::getExceptionType
//
/**
   Returns the class name as a String.

   This method should be overridden in all derived classes to return the name
   of the derived class.
   
   @sa getDescription()
*/
//==============================================================================
String Exception::getExceptionType() const
{
	return QC_T("Exception");
}

//==============================================================================
// Exception::getLocalizedDescription
//
/**
   Returns a localized description of the Exception type or, if a native
   language translation is unavailable, an empty string.

   The base class implementation always returns an empty string.
   
   @sa getDescription()
*/
//==============================================================================
String Exception::getLocalizedDescription() const
{
	return String();
}

//==============================================================================
// Exception::getDescription
//
/**
   Returns a description of this Exception type.

   If getLocalizedDescription() returns a non-empty string, then it is
   returned, otherwise the exception class name returned from getExceptionType()
   is returned.
*/
//==============================================================================
String Exception::getDescription() const
{
	const String& localizedDescr = getLocalizedDescription();
	if(localizedDescr.empty())
	{
		return getExceptionType();
	}
	else
	{
		return localizedDescr;
	}
}

QC_BASE_NAMESPACE_END

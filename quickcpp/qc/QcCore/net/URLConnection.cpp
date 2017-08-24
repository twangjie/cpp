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
// Class URLConnection
/**
	@class qc::net::URLConnection
	
	@brief An @a abstract base class that represents a connection
	to a resource specified by a URL.
	
	Instances of URLConnection are created by classes derived from
	URLStreamHandler, which are themselves created by a URLStreamHandlerFactory.
	
	When a URL is created, the protocol (or scheme) is used to request
	the URLStreamHandlerFactory for an instance of a URLStreamHandler that knows
	how to (a) parse URLs of for that protocol and (b) create instances of
	URLConnection that know how to connect to a resource using the
	protocol.

	Many programs do not need to use a URLConnection directly, they
	can simply create a URL and ask it for an InputStream using
	@c getInputStream().  However, the URLConnection class gives the 
	application greater control over how to connect to the resource
	and even allows the program to write to some types of resource via an
	OutputStream.
	
	Programs that do need to deal with a URLConnection directly generally
	follow the following basic sequence:-
	-# Construct a URL object
	-# Obtain a URLConnection object by calling @c openConnection() on the URL
	-# Manipulate the URLConnection's parameters
	-# Connect to the remote resource by calling connect()
	-# Read header fields and/or the resource using getInputStream()

	Here is a sample console application that connects to a URL and prints
	out the header details to the standard output.

@code

#include "QcCore/io/Console.h"
#include "QcCore/net/URL.h"
#include "QcCore/net/URLConnection.h"

using namespace qc;
using namespace net;
using namespace io;

int main(int argc, char* argv[])
{
    // create a SystemMonitor to ensure clean library termination
    SystemMonitor monitor;

    if(argc != 2)
    {
        Console::cout() << QC_T("Enter a single URL") << endl;
        exit(1);
    }

    // convert the multi-byte input into a QuickCPP (Unicode) string
    String urlSpec = StringUtils::FromNativeMBCS(argv[1]);

    try
    {
        URL url(urlSpec);

        AutoPtr<URLConnection> rpUrlConn = url.openConnection();

        rpUrlConn->connect();

        for(int i=0; i<rpUrlConn->getHeaderFieldCount(); i++)
        {
            const String& hdr = rpUrlConn->getHeaderField(i);
            if(rpUrlConn->getHeaderFieldKey(i).empty())
                Console::cout() << QC_T("null");
            else
                Console::cout() << rpUrlConn->getHeaderFieldKey(i);

            Console::cout() << QC_T(": ") << hdr << endl;
        }
    }
    catch(Exception& e)
    {
        Console::err() << e.toString() << endl;
    }

    return 0;
}

@endcode

*/
//==============================================================================

#include "URLConnection.h"
#include "ProtocolException.h"

#include "QcCore/base/StringUtils.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/base/Tracer.h"
#include "QcCore/base/IllegalStateException.h"
#include "QcCore/util/InvalidDateException.h"

QC_NET_NAMESPACE_BEGIN

using namespace util;

bool URLConnection::s_bUseCachesDefault = true;

//==============================================================================
// URLConnection::URLConnection
//
/**
   Constructs a URLConnection for the specified URL.  No attempt
   is made to connect to the URL resource at this stage.
*/
//==============================================================================
URLConnection::URLConnection(const URL& url) :
	m_bConnected(false),
	m_url(url),
	m_bUseCaches(s_bUseCachesDefault),
	m_doInput(TriUnspecified),
	m_doOutput(TriUnspecified)
{
}

//==============================================================================
// URLConnection::getContentEncoding
//
/**
   Returns the value of the "content-encoding" header field.
*/
//==============================================================================
String URLConnection::getContentEncoding()
{
	return getHeaderField(QC_T("content-encoding"));
}

//==============================================================================
// URLConnection::getContentLength
//
/**
   Returns the length of the content for this URLConnection.

   For @c http URLs this is the value returned in the @c content-length
   header field (if present).  For URLs that use the @c file: protocol the
   file length is read directly from the file system.

   When using HTTP 1.1, the content length is not always available.  In this case
   the resource will be transmitted using the "Chunked" encoding method.

   @returns the length of the content or @c -1 if the content length is not known.
*/
//==============================================================================
long URLConnection::getContentLength()
{
	return getHeaderFieldLong(QC_T("content-length"), -1L);
}

//==============================================================================
// URLConnection::getContentType
//
/**
   Returns the value of the "content-type" header field.
*/
//==============================================================================
String URLConnection::getContentType()
{
	return getHeaderField(QC_T("content-type"));
}

//==============================================================================
// URLConnection::getDate
//
/**
   Returns the value of the "date" header field parsed into a DateTime
   object.  If the header field does not exist an invalid DateTime
   object is returned.

   @sa DateTime::isValid()
*/
//==============================================================================
DateTime URLConnection::getDate()
{
	return getHeaderFieldDate(QC_T("date"));
}

//==============================================================================
// URLConnection::getLastModified
//
/**
   Returns the value of the "last-modified" header field parsed into a DateTime
   object.  If the header field does not exist an invalid DateTime
   object is returned.
*/
//==============================================================================
DateTime URLConnection::getLastModified()
{
	return getHeaderFieldDate(QC_T("Last-Modified"));
}

//==============================================================================
// URLConnection::getHeaderFieldDate
//
/**
   Parse the specified header field as a Date and return as a DateTime.

   @returns a DateTime object representing the @c name header field.  If the
   header field does not exist, or is not a valid Date an invalid DateTime
   is returned.
*/
//==============================================================================
DateTime URLConnection::getHeaderFieldDate(const String& name)
{
	const String& val = getHeaderField(name);
	if(!val.empty())
	{
		try
		{
			DateTime ret(val);
			return ret;
		}
		catch(InvalidDateException& e)
		{
			Tracer::Trace(Tracer::Net, Tracer::Exceptions, e.toString());
		}
	}
	return DateTime();
}

//==============================================================================
// URLConnection::getHeaderFieldLong
//
/**
   Parse the specified header field as a long integer.  If the header is not
   present then return the passed default value.

   @param name the header field key
   @param defaultValue the value to return if the specified header field is 
          not present
*/
//==============================================================================
long URLConnection::getHeaderFieldLong(const String& name, long defaultValue)
{
	String field = getHeaderField(name);
	if(field.empty())
	{
		return defaultValue; 
	}
	else
	{
		return NumUtils::ToLong(field);
	}
}

//==============================================================================
// URLConnection::getURL
//
/**
   Returns the URL for this URLConnection.
*/
//==============================================================================
const URL& URLConnection::getURL() const
{
	return m_url;
}

//==============================================================================
// URLConnection::setURL
//
/**
   Sets the URL for this URLConnection.  Called by @a derived classes
   when a URL needs to be updated - such as during HTTP redirection.

   @sa getURL()
*/
//==============================================================================
void URLConnection::setURL(const URL& url)
{
	m_url = url;
}

//==============================================================================
// URLConnection::setConnected
//
/**
   Sets the @c connected property to @c bConnected.
*/
//==============================================================================
void URLConnection::setConnected(bool bConnected)
{
	m_bConnected = bConnected;
}

//==============================================================================
// URLConnection::isConnected
//
/**
   Returns the value of the @c connected property.

   @returns true if this URLConnection is connected to the resource; false
   otherwise
*/
//==============================================================================
bool URLConnection::isConnected() const
{
	return m_bConnected;
}

//==============================================================================
// URLConnection::SetDefaultUseCaches
//
/**
   Static method to set the default value for the @c UseCaches property
   for new instances of URLConnection.
*/
//==============================================================================
void URLConnection::SetDefaultUseCaches(bool bUseCaches)
{
	s_bUseCachesDefault = bUseCaches;
}

//==============================================================================
// URLConnection::GetDefaultUseCaches
//
/**
   Returns the default value for the @c UseCaches property that will be
   used for new instances of URLConnection.
*/
//==============================================================================
bool URLConnection::GetDefaultUseCaches()
{
	return s_bUseCachesDefault;
}

//==============================================================================
// URLConnection::setUseCaches
//
/**
   Sets the value of the @c UseCaches property for this URLConnection instance.
   When set to @c true, the URLConnection will permit the use of cached
   resources.
*/
//==============================================================================
void URLConnection::setUseCaches(bool bUseCaches)
{
	m_bUseCaches = bUseCaches;
}

//==============================================================================
// URLConnection::getUseCaches
//
/**
   Returns the value of the @c UseCaches property for this URLConnection.
*/
//==============================================================================
bool URLConnection::getUseCaches() const
{
	return m_bUseCaches;
}

//==============================================================================
// URLConnection::getDoInput
//
/**
   Returns a boolean value indicating whether this URLConnection may be
   used for input operations.

   A URLConnection may be used for input, output or both depending on the
   protocol and the settings of the @c doInput/doOutput flags.

   The default value is @c true, unless this URLConnection has been
   explicitly enabled for output operations, in which case the default
   is @c false.

   @sa setDoInput()
   @sa setDoOutput()
*/
//==============================================================================
bool URLConnection::getDoInput() const
{
	return (m_doInput==TriTrue || (m_doInput!=TriFalse && m_doOutput != TriTrue));
}

//==============================================================================
// URLConnection::setDoInput
//
/**
   Enables this URLConnection for input processing.

   @throws IllegalStateException if this URLConnection is already connected.

   @sa getDoInput()
   @sa getInputStream()
   @sa setDoOutput()
*/
//==============================================================================
void URLConnection::setDoInput(bool bEnable)
{
	if(isConnected())
		throw IllegalStateException(QC_T("already connected"));

	m_doInput = bEnable ? TriTrue : TriFalse;
}

//==============================================================================
// URLConnection::getDoOutput
//
/**
   Returns a boolean value indicating whether this URLConnection may be
   used for output operations.

   A URLConnection may be used for input, output or both depending on the
   protocol and the settings of the @c doInput/doOutput flags.

   The default value is @c false..

   @sa setDoOutput()
*/
//==============================================================================
bool URLConnection::getDoOutput() const
{
	return (m_doOutput == TriTrue);
}

//==============================================================================
// URLConnection::setDoOutput
//
/**
   Enables this URLConnection for output processing.

   Enabling output processing will disable input processing unless input
   processing is explicitly enabled using setDoInput().

   @throws IllegalStateException if this URLConnection is already connected.

   @sa getDoOutput()
   @sa getOutputStream()
   @sa setDoInput()
*/
//==============================================================================
void URLConnection::setDoOutput(bool bEnable)
{
	if(isConnected())
		throw IllegalStateException(QC_T("already connected"));
	
	m_doOutput = bEnable ? TriTrue : TriFalse;
}

//==============================================================================
// URLConnection::setRequestProperty
//
/**
   Sets a request MIME header value.  If a MIME header with the specified
   name already exists, its value is replaced with the supplied value.

   @param name the name by which the property is known.
   @param value the value to be associated with the named property.
   
   @sa getRequestProperty()
   @throws ProtocolException if the header is not supported or cannot be set
   at this time.
*/
//==============================================================================
void URLConnection::setRequestProperty(const String& name, const String& value)
{
	throw ProtocolException(QC_T("The URLConnection does not support request properties"));
}

//==============================================================================
// URLConnection::getRequestProperty
//
/**
   Returns the value of the specified MIME header.
   @param name the name by which the property is known.
   @returns the requested header value, or an empty string if a header
   with the supplied name does not exist.

   @sa setRequestProperty()
*/
//==============================================================================
String URLConnection::getRequestProperty(const String& name) const
{
	return String();
}

#ifdef QC_DOCUMENTATION_ONLY
//=============================================================================
//
// Documentation for pure virtual methods follows:
//
//=============================================================================

//==============================================================================
// URLConnection::getHeaderFieldCount
//
/**
   Returns the number of header fields.

   @note This function may attempt to connect to the URL resource if it is not
         already connected.
*/
//==============================================================================
size_t URLConnection::getHeaderFieldCount();

//==============================================================================
// URLConnection::getHeaderField
//
/**
   Returns the value of a header field where the key is equal to @c name.

   If the key is not found in the collection an empty String is returned.
   Key comparison is performed without regard to case.

   @param name the name of the header.

   @note This function may attempt to connect to the URL resource if it is not
         already connected.
*/
//==============================================================================
String URLConnection::getHeaderField(const String& name);

//==============================================================================
// URLConnection::getHeaderField
//
/**
   Returns the value of a header field by index.

   If @c index exceeds the number of entries in the list an empty
   String is returned.

   @param index a 0-based index into the header sequence.
   @note This function may attempt to connect to the URL resource if it is not
         already connected.
*/
//==============================================================================
String URLConnection::getHeaderField(size_t index);

//==============================================================================
// URLConnection::getHeaderFieldKey
//
/**
   Returns the value of a header field key by index.

   If @c index exceeds the number of entries in the list an empty
   String is returned

   @param index a 0-based index into the header sequence.

   @note This function may attempt to connect to the URL resource if it is not
         already connected.
*/
//==============================================================================
String URLConnection::getHeaderFieldKey(size_t index);

//==============================================================================
// URLConnection::getInputStream
//
/**
   Returns an InputStream which reads bytes from the connection.

   Calling getInputStream() will automatically call connect()
   to connect to the resource if it is not already connected.

   @throws IOException if an error occurs while connecting or reading from the
           connection
*/
//==============================================================================
AutoPtr<InputStream> URLConnection::getInputStream();

//==============================================================================
// URLConnection::getOutputStream
//
/**
   Returns an OutputStream which can be used to write bytes to the connection.

   Calling getOutputStream() does not necessarily mean that a connection will
   be established to the resource specified by the URL.  For some protocols,
   the connection will only become established once getInputStream() or
   connect() is called.

   @throws IOException if an error occurs
   @throws IllegalStateException if a connection has already been established
           and the protocol requires the output stream to obtained prior
		   to connecting (as is the case with HTTP).
   @throws ProtocolException if the URLConnection has not been made ready for
           output operations using setDoOutput()
*/
//==============================================================================
AutoPtr<OutputStream> URLConnection::getOutputStream();

#endif //QC_DOCUMENTATION_ONLY

QC_NET_NAMESPACE_END

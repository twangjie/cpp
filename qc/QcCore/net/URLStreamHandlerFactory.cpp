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
/**
	@class qc::net::URLStreamHandlerFactory
	
	@brief Factory class for creating instances of URLStreamHandler.

    @QuickCPP is supplied with URLStreamHandlers that can understand
	a range of different protocols.  The URLStreamHandlerFactory base class
	is aware of the supplied protocols, and will return an appropriate
	URLStreamHandler instance for every protocol that it recognizes.

    Applications can extend @QuickCPP by supplementing their own protocols.  To
	achieve this the application must create a new factory class @a derived
	from URLStreamHandlerFactory and set an instance of the @a derived class as
	the global URLStreamHandlerFactory by calling SetInstance().

	An example of creating a custom URLStreamHandlerFactory is shown below.  In
	this example, the application has implemented classes for dealing with URLs
	for the "mailto" protocol.  The MailtoUrlConnection class is not shown but
	its sister class, MailtoURLStreamHandler, is a complete (but trivial)
	implementation of a URLStreamHandler.

    @code
    class MailtoURLStreamHandler : public URLStreamHandler {
    public:
        virtual AutoPtr<URLConnection> openConnection(const URL& url) const
        {
            return new MailtoURLConnection(url);
        }

        virtual int getDefaultPort() const
        {
            return 25;  // SMTP port
        }
    };

    class MyURLStreamHandlerFactory : public URLStreamHandlerFactory {
    public:
        AutoPtr<URLStreamHandler> createURLStreamHandler(const String& protocol) const
        {
            if(StringUtils::CompareNoCase(protocol, QC_T("mailto")) == 0)
                return new MailtoURLStreamHandler;
            else
                return URLStreamHandlerFactory::createURLStreamHandler(protocol);
        }
    };

    int main(int argc, char* argv[])
    {
        // create a SystemMonitor instance to ensure clean termination
        SystemMonitor monitor;
        
        // register our custom URLStreamHandlerFactory
        URLStreamHandlerFactory::SetInstance(new MyURLStreamHandlerFactory);

        // 
        // Manipulate URLs, including our mailto: URL...
        //

        return (0);
    }
    @endcode
	
	@sa URLStreamHandler
*/
//==============================================================================

#include "URLStreamHandlerFactory.h"
#include "URLStreamHandler.h"
#include "FileURLStreamHandler.h"
#include "HttpURLStreamHandler.h"
#include "FtpURLStreamHandler.h"

#include "QcCore/base/System.h"
#include "QcCore/base/ObjectManager.h"
#include "QcCore/base/FastMutex.h"
#include "QcCore/base/StringUtils.h"

QC_NET_NAMESPACE_BEGIN

//==================================================================
// Multi-threaded locking strategy
//
// There is a singleton URLStreamHandlerFactory object
// lazily created on demand.
//
// To ensure that singleton resources are not created by multiple
// concurrent threads, the pointers are protected using a
// single static mutex.
//
// Update access to static variables is mutex protected, but to 
// minimise the runtime cost, read access is not protected.  This
// gives an exposure to the so-called "relaxed" memory model that
// exists on some multi-processor machines.  We minimise this
// exposure by declaring the static variables as 'volatile'.
//==================================================================
#ifdef QC_MT
	FastMutex URLStreamHandlerFactoryMutex;
#endif //QC_MT

URLStreamHandlerFactory* QC_MT_VOLATILE URLStreamHandlerFactory::s_pInstance = NULL;

//==============================================================================
// URLStreamHandlerFactory::GetInstance
//
/**
   Returns the global URLStreamHandlerFactory.

   If a factory has not been registered by the application,
   a default URLStreamHandlerFactory is created by the system and returned.

   @sa SetInstance()
   @mtsafe
*/
//==============================================================================
AutoPtr<URLStreamHandlerFactory> URLStreamHandlerFactory::GetInstance()
{
	if(s_pInstance == NULL)
	{
		SetInstance(new URLStreamHandlerFactory);
	}
	QC_DBG_ASSERT(s_pInstance!=0);
	return s_pInstance;
}

//==============================================================================
// URLStreamHandlerFactory::SetInstance
//
/**
   Sets the global URLStreamHandlerFactory instance.  This global
   factory is used by the URL class to obtain a URLStreamHandler
   that can parse and understand URLs for a given protocol.

   Like other @QuickCPP global objects, URLStreamHandlerFactory is a ManagedObject.
   This enables the passed object to be registered with the system's
   ObjectManager, thereby freeing the application from having to manage the lifetime
   of the factory object.  In other words, the application does not need to maintain
   the object's reference-count once it has been registered.

   A typical application may do the following:-

   @code
   URLStreamHandlerFactory::SetInstance(new MyURLStreamHandlerFactory);
   @endcode

   @sa GetInstance()
   @mtsafe
*/
//==============================================================================
void URLStreamHandlerFactory::SetInstance(URLStreamHandlerFactory* pFactory)
{
	//==================================================================
	// Multi-threaded locking strategy
	//
	// See top of file.
	//==================================================================

	if(pFactory)
	{
		System::GetObjectManager().registerObject(pFactory);
	}

	URLStreamHandlerFactory* pExisting;
	// create a scope for the mutex lock
	{
		QC_AUTO_LOCK(FastMutex, URLStreamHandlerFactoryMutex);
		pExisting = s_pInstance;
		s_pInstance = pFactory;
	}

	if(pExisting) 
	{
		System::GetObjectManager().unregisterObject(pExisting);
	}
}

//==============================================================================
// URLStreamHandlerFactory::createURLStreamHandler
//
/**
   Returns an instance of a URLStreamHandler
   that can understand URLs for the given protocol.

   @param protocol the name of the protocol
   @mtsafe
*/
//==============================================================================
AutoPtr<URLStreamHandler> URLStreamHandlerFactory::createURLStreamHandler(const String& protocol) const
{
	if(StringUtils::CompareNoCase(protocol, QC_T("file")) == 0 || protocol.empty())
		return new FileURLStreamHandler;
	else if(StringUtils::CompareNoCase(protocol, QC_T("http")) == 0)
		return new HttpURLStreamHandler;
	else if(StringUtils::CompareNoCase(protocol, QC_T("ftp")) == 0)
		return new FtpURLStreamHandler;
	else
		return NULL;
}

QC_NET_NAMESPACE_END


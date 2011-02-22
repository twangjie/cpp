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
// Class Authenticator
/**
	@class qc::net::Authenticator
	
	@brief An @a abstract base class that defines a call-back
	mechanism invoked when a network protocol requires authentication.

    One example of when an Authenticator is required is when an application
	attempts to connect to a HTTP resource.  If the HTTP server
	requires client connections to authenticate themselves, the HttpURLStreamHandler
	obtains the necessary credentials from the default (global) Authenticator.

    @QuickCPP does not create a default Authenticator, so the default action
	is for access requests for authenticated services to be denied.  However,
	an example implementation of the Authenticator interface is
	provided by the NetAccessHelper class.

    Applications use Authenticator by creating a @a derived class, and registering
	an instance of that class using the SetDefault() method. When authentication
	is required, the network component calls RequestPasswordAuthentication()
	which delegates the request to the registered Authenticator.
*/
//
// Note: This class deviates significantly from the JDK Authenticator
//       interface.  This is deliberate as the JDK interface has significant
//       state and would therefore require synchronization in a multi-threaded
//       environment.
//
//==============================================================================

#include "Authenticator.h"

#include "QcCore/base/System.h"
#include "QcCore/base/ObjectManager.h"
#include "QcCore/base/FastMutex.h"

QC_NET_NAMESPACE_BEGIN

//==================================================================
// Multi-threaded locking strategy
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
	FastMutex AuthenticatorMutex;
#endif //QC_MT

Authenticator* QC_MT_VOLATILE Authenticator::s_pAuthenticator = 0;

//==============================================================================
// Authenticator::SetDefault
//
/**
   Sets the default (and global) Authenticator for the application.

   In common with all @QuickCPP global objects, the system ensures the
   passed object will exist for the lifetime of the application
   by registering it with the ObjectManager.  Therefore, the application
   does not need to concern itself with managing the lifetime of the object.

   For example, the following is perfectly valid and does not present 
   a resource leak:

   @code
   Authenticator::SetDefault(new MyCustomAuthenticator);
   @endcode
*/
//==============================================================================
void Authenticator::SetDefault(Authenticator* pAuthenticator)
{
	//==================================================================
	// Multi-threaded locking strategy
	//
	// See top of file.
	//==================================================================

	if(pAuthenticator) 
	{
		System::GetObjectManager().registerObject(pAuthenticator);
	}

	Authenticator* pExisting;

	// create a scope for the mutex lock
	{
		QC_AUTO_LOCK(FastMutex, AuthenticatorMutex);
		pExisting = s_pAuthenticator;
		s_pAuthenticator = pAuthenticator;
	}

	if(pExisting) 
	{
		System::GetObjectManager().unregisterObject(pExisting);
	}
}
 
//==============================================================================
// Authenticator::RequestPasswordAuthentication
//
/**
   Static method that simply passes the request to the registered global
   Authenticator (if any).

   @param pAddr the IP address of the server requesting authentication
   @param port the port number of the remote server
   @param protocol the name of the protocol being used, e.g. 'HTTP'
   @param prompt a string from the server indicating why a password is required
   @param scheme the authentication scheme being used
   @param bProxy indicates if the authentication request is coming from
          the local proxy server
   @param userRet a return parameter for the required user name
   @param passwordRet a return parameter for the required password
   @returns the return value from the global Authenticator's method or false
   if no global Authenticator has been registered.
*/
//==============================================================================
bool Authenticator::RequestPasswordAuthentication(InetAddress* pAddr, int port,
		const String& protocol, const String& prompt, const String& scheme,
		bool bProxy, String& userRet, String& passwordRet)
{
	if(s_pAuthenticator)
	{
		return s_pAuthenticator->getPasswordAuthentication(pAddr, port,
			protocol, prompt, scheme,
			bProxy, userRet, passwordRet);
	}
	else
	{
		return false;
	}
}

#ifdef QC_DOCUMENTATION_ONLY
//=============================================================================
//
// Documentation for pure virtual methods follows:
//
//=============================================================================

//==============================================================================
// Authenticator::getPasswordAuthentication
//
/**
   Called when password authorization is needed. @a Derived classes
   must provide an implementation of this pure virtual function.

   @param pAddr the IP address of the server requesting authentication
   @param port the port number of the remote server
   @param protocol the name of the protocol being used, e.g. 'HTTP'
   @param prompt a string from the server indicating why a password is required
   @param scheme the authentication scheme being used
   @param bProxy indicates if the authentication request is coming from
          the local proxy server
   @param userRet a return parameter for the required user name
   @param passwordRet a return parameter for the required password
   @returns true if @c userRet and @c passwordRet have been
            set; false otherwise
*/
//==============================================================================
bool Authenticator::getPasswordAuthentication(InetAddress* pAddr, int port,
		const String& protocol, const String& prompt, const String& scheme,
		bool bProxy, String& userRet, String& passwordRet);

#endif // QC_DOCUMENTATION_ONLY

QC_NET_NAMESPACE_END

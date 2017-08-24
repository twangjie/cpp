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
	@class qc::net::SocketImplFactory
	
	@brief Factory class for creating instances of SocketImpl.

    @sa Socket::SetSocketImplFactory()
	@sa ServerSocket::SetSocketImplFactory()
*/
//==============================================================================

#include "SocketImplFactory.h"
#include "PlainSocketImpl.h"

QC_NET_NAMESPACE_BEGIN

//==============================================================================
// SocketImplFactory::createSocketImpl
//
/**
   Creates an instance of a concrete SocketImpl class used by the
   local @QuickCPP installation.
*/
//==============================================================================
AutoPtr<SocketImpl> SocketImplFactory::createSocketImpl() const
{
	return new PlainSocketImpl;
}

QC_NET_NAMESPACE_END

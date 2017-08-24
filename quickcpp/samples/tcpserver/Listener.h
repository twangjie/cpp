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
// tcpserver sample application
//
//==============================================================================

#ifndef Listener_h
#define Listener_h

#include "QcCore/base/Thread.h"
#include "QcCore/base/Runnable.h"
#include "QcCore/base/AutoPtr.h"
#include "QcCore/net/ServerSocket.h"

using namespace qc;
using namespace qc::net;

class Listener : public
#ifdef QC_MT
 Thread
#else
 Runnable
#endif
{
public:
	Listener(ServerSocket* pSocket);
	void stop();
	void run();

private:
	AutoPtr<ServerSocket> m_rpSocket;
};

#endif //Listener_h


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

#ifndef ClientHandler_h
#define ClientHandler_h

#include "QcCore/base/Runnable.h"
#include "QcCore/base/AutoPtr.h"
#include "QcCore/net/Socket.h"

using namespace qc;
using namespace qc::net;

class Listener;

class ClientHandler : public Runnable
{
public:
	ClientHandler(Socket* pSocket, Listener* pListener);
	void run();

private:
	AutoPtr<Listener> m_rpListener;
	AutoPtr<Socket> m_rpSocket;
};

#endif //ClientHandler_h

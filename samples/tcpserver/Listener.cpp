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
// Class Listener
// --------------
//
// An instance of this class is created to whenever a new TCP socket connection
// is established with a client.  
// 
// This class handles all communication with the client over the socket.  It
// will run either synchronously or concurrently depending on the threading
// model.  In single-threaded mode the run() method is executed within the
// context of the main thread, so control is not returned to the main program
// until the client connection terminates.
//
//==============================================================================

#include "Listener.h"
#include "ClientHandler.h"

#include "QcCore/base/Thread.h"
#include "QcCore/net/ServerSocket.h"
#include "QcCore/io/Console.h"
#include "QcCore/io/IOException.h"

using namespace qc::io;

#define COUT Console::cout()
#define CERR Console::cerr()

Listener::Listener(ServerSocket* pSocket) :
	Thread(QC_T("Listener")),
	m_rpSocket(pSocket)
{
}

void Listener::stop()
{
	try
	{
		COUT << QC_T("closing listening socket") << endl;
		m_rpSocket->close();
		COUT << QC_T("closed listening socket") << endl;
		interrupt();
	}
	catch(IOException& e)
	{
		COUT << e.toString() << endl;
	}
}

void Listener::run()
{
	COUT << QC_T("The listening thread is: ") << Thread::CurrentThread()->getName() << endl;

	try
	{
		while(true)
		{
			AutoPtr<Socket> rpSocket = m_rpSocket->accept();
			AutoPtr<Thread> rpClientThread = new Thread(new ClientHandler(rpSocket.get(), this));
			rpClientThread->start();
		}
	}
	catch(Exception& e)
	{
		CERR << QC_T("accept() failed: ") << e.toString() << endl;
	}
	COUT << QC_T("Listener terminated") << endl;
}


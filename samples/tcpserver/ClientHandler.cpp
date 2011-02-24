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
// Class ClientHandler
// -------------------
//
// An instance of this class is created whenever a new TCP socket connection
// is established with a client.  
// 
// This class handles all communication with the client over the socket.  It
// will run either synchronously or concurrently depending on the threading
// model.  In single-threaded mode the run() method is executed within the
// context of the main thread, so control is not returned to the main program
// until the client connection terminates.
//
//==============================================================================

#include "ClientHandler.h"
#include "Listener.h"

#include "QcCore/base/String.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/base/System.h"
#include "QcCore/base/Thread.h"
#include "QcCore/io/BufferedReader.h"
#include "QcCore/io/BufferedInputStream.h"
#include "QcCore/io/Console.h"
#include "QcCore/io/FileOutputStream.h"
#include "QcCore/io/InterruptedIOException.h"
#include "QcCore/io/InputStreamReader.h"
#include "QcCore/io/OutputStreamWriter.h"
#include "QcCore/net/InetAddress.h"

using namespace qc;
using namespace qc::net;
using namespace qc::io;

const size_t ClientTimeout = 0; // no timeout
const bool bEcho = false;
const bool bLog = true;
const bool bWelcomeMessage = true;

#define COUT Console::cout()
#define CERR Console::cerr()

ClientHandler::ClientHandler(Socket* pSocket, Listener* pListener) :
	m_rpListener(pListener),
	m_rpSocket(pSocket)
{
}

void ClientHandler::run()
{
	COUT << QC_T("Connected to client: ") << m_rpSocket->toString() << endl;

	COUT << QC_T("The client thread is: ") << Thread::CurrentThread()->getName() << endl;

	AutoPtr<OutputStream> rpSockOut = m_rpSocket->getOutputStream();
	AutoPtr<OutputStream> rpLog = new FileOutputStream(QC_T("server.log"));
	AutoPtr<Writer> rpSockWriter = new OutputStreamWriter(m_rpSocket->getOutputStream().get());
	AutoPtr<InputStream> rpInput = new BufferedInputStream(m_rpSocket->getInputStream().get());
	
	try
	{
		if(ClientTimeout != 0)
			m_rpSocket->setSoTimeout(ClientTimeout);

		//
		// If requested, send out a welcome message to the client informing
		// them that they have connected to the QuickCPP test server
		//
		if(bWelcomeMessage)
		{
			String welcome = QC_T("Welcome to the test server using QuickCPP version ");
			welcome += System::GetVersionAsString() + QC_T(" running on ");
			welcome += InetAddress::GetLocalHost()->getHostName() + QC_T("\r\n");
			rpSockWriter->write(welcome);
			rpSockWriter->flush();
		}

		Byte buffer[1024];

		while(true)
		{
			int bytesRead = rpInput->read(buffer, sizeof(buffer));
			if(bytesRead == InputStream::EndOfFile)
			{
				break;
			}
			else
			{
				String str = StringUtils::FromLatin1((char*)buffer, bytesRead);
				COUT << str << endl;

				if(bLog)
				{
					rpLog->write(buffer, bytesRead);
					rpLog->flush();
				}

				if(bEcho)
				{
					rpSockOut->write(buffer, bytesRead);
					rpSockOut->flush();
				}

				if(StringUtils::startsWith(StringUtils::ToLower(str), QC_T("exit")))
				{
					break;
				}
			}
		}
	}
	catch(InterruptedIOException& /*e*/)
	{
		CERR << QC_T("Client idle for too long") << endl;
		if(bEcho)
		{
			try {
				static const String idle = QC_T("Too slow, sorry!\r\n");
				rpSockWriter->write(idle);
				rpSockWriter->flush();
			} catch(Exception&) {}
		}
	}
	catch(Exception& e)
	{
		CERR << QC_T("Client connection failed: ") << e.toString() << endl;
	}
	COUT << QC_T("Client connection terminated") << endl;
}

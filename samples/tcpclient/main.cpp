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
// QuickCPP Sample Application: tcpclient
//
// This simple console application will initiate a TCP connection to a specified
// host/port and send it a "Hello World!" message.  It will then (optionally)
// wait for a reply and echo this out to the console.
//
//==============================================================================

#include "QcCore/net/InetAddress.h"
#include "QcCore/net/ServerSocket.h"
#include "QcCore/net/Socket.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/base/Exception.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/base/System.h"
#include "QcCore/base/Tracer.h"
#include "QcCore/base/Thread.h"
#include "QcCore/io/Console.h"
#include "QcCore/auxil/MemCheckSystemMonitor.h"
#include "QcCore/auxil/CommandLineParser.h"
#include "QcCore/auxil/BooleanOption.h"
#include "QcCore/auxil/TraceHelper.h"

#ifndef _MSC_VER
#include <signal.h>
#endif

using namespace qc;
using namespace qc::net;
using namespace qc::io;
using namespace qc::auxil;

bool bContinue = true;
const size_t ClientTimeout = 20000; // 20 secs

#define COUT Console::cout()
#define CERR Console::cerr()

void showUsage(const String& programName)
{
	COUT << QC_T("Usage: ") << programName << QC_T(" [option]... ") << endl << endl;
	COUT << QC_T("Test server.") << endl << endl;

	COUT << QC_T("  -h, --help           display this help") << endl;
	COUT << QC_T("  -l, --linger <secs>  enable the SO_LINGER option") << endl;
	COUT << QC_T("  -p, --port           port to connect to") << endl;
	COUT << QC_T("  -p, --reply          wait for a reply (and echo it to standard out)") << endl;
	COUT << QC_T("  -s, --server         server to connect to") << endl;
	COUT << QC_T("  -t, --timeout        milliseconds to wait for connection") << endl;
}

int main(int argc, char* argv[])
{
	MemCheckSystemMonitor monitor;

	//
	// Set up a command line parser to recognise our options...
	// 
	BasicOption optHelp(QC_T("help"), 'h', BasicOption::none);
	BasicOption optLinger(QC_T("linger"), 'l', BasicOption::mandatory);
	BasicOption optPort(QC_T("port"), 'p', BasicOption::mandatory);
	BooleanOption optReply(QC_T("reply"), 'r', true);
	BasicOption optServer(QC_T("server"), 's', BasicOption::mandatory);
	BasicOption optTimeout(QC_T("timeout"), 't', BasicOption::mandatory);
	BasicOption optTrace(QC_T("trace"), 0, BasicOption::mandatory);

	CommandLineParser cmdlineParser;
	cmdlineParser.addOption(&optHelp);
	cmdlineParser.addOption(&optLinger);
	cmdlineParser.addOption(&optPort);
	cmdlineParser.addOption(&optReply);
	cmdlineParser.addOption(&optServer);
	cmdlineParser.addOption(&optTimeout);
	cmdlineParser.addOption(&optTrace);

	//
	// ...parse the cpmmand line, placing the command options into their 
	// respective objects.
	//
	try
	{
		cmdlineParser.parse(argc, argv);
	}
	catch (CommandLineException& e)
	{
		CERR << cmdlineParser.getProgramName() << QC_T(": ") << e.getMessage() << endl << endl;
		CERR << QC_T("Try ") << cmdlineParser.getProgramName() << QC_T(" --help") << endl;
		return (1);
	}

	//
	// Determine local host info
	//
	AutoPtr<InetAddress> rpLocal = InetAddress::GetLocalHost();
	COUT << QC_T("Client running on host ") << rpLocal->toString() << endl;

	if(optHelp.isPresent())
	{
		showUsage(cmdlineParser.getProgramName());
		return (0);
	}

	//
	// Determine the host and port to which we shall try and connect
	//
	int port = 0;
	if(optPort.isPresent())
	{
		port = NumUtils::ToInt(optPort.getArgument());
	}

	String host = QC_T("localhost");
	if(optServer.isPresent())
	{
		host = optServer.getArgument();
	}

	try
	{
		//
		// If we have been asked to enable tracing, create a TraceHelper (from the auxil namespace)
		// This little helper is initialized with a string from which it extracts what tracing
		// should be performed, and to where the output should be written.
		//
		if(optTrace.isPresent())
		{
			Tracer::SetTracer(new TraceHelper(cmdlineParser.getProgramName(), optTrace.getArgument()));
		}

		COUT << QC_T("Connecting to server: ") << host << QC_T(" on port: ") << port << endl;

		//
		// Here we demonstate two methods of connecting to a server.  
		//
		// The first method uses an overloaded socket constructor to specify the host and port
		// and attempts to connect immediately.
		//
		// The second method uses the default constructor, enabling us to set a timeout for the 
		// connect method.
		//
		AutoPtr<Socket> rpSocket;

		size_t timeoutMS = 0;
		if(optTimeout.isPresent())
			timeoutMS = NumUtils::ToInt(optTimeout.getArgument());

		if(timeoutMS)
		{
			// as a timeout has been specified, we must use the "construct then connect" model
			rpSocket = new Socket;

			// Note that connect() is not overloaded to take a host name and a timeout value,
			// so we must obtain an InetAddress for the host.

			AutoPtr<InetAddress> rpAddr = InetAddress::GetByName(host);
			rpSocket->connect(rpAddr.get(), port, timeoutMS);
		}
		else
		{
			rpSocket = new Socket(host, port);
		}

		//
		// If the --linger option has been specified, use its (mandatory) value
		// to set the SO_LINGER option
		//
		if(optLinger.isPresent())
		{
			int linger = NumUtils::ToInt(optLinger.getArgument());
			rpSocket->setSoLinger(true, linger);
		}

		//
		// Send a simple message to the server.
		// Note we are sending raw bytes in this case, not Unicode characters.
		//
		const char * hello = "Hello World!";
		rpSocket->getOutputStream()->write((const Byte*)hello, strlen(hello));

		//
		// If a reply is expected, then wait for it - but use the timeout value
		// to limit the length of time we'll wait.
		//
		if(optReply.getValue())
		{
			if(timeoutMS)
				rpSocket->setSoTimeout(timeoutMS);

			Byte buffer[80];
			long bytesRead;

			COUT << QC_T("Reply from server: ") ;

			while( (bytesRead = rpSocket->getInputStream()->read(buffer, sizeof(buffer))) != InputStream::EndOfFile)
			{
				COUT << StringUtils::FromLatin1((const char*)buffer, bytesRead);
			}

			COUT << endl;
		}
	}
	catch(Exception& e)
	{
		CERR << e.toString() << endl;
	}

	COUT << QC_T("Terminating client") << endl;

	return 0;
}

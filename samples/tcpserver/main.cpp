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

#include "Listener.h"

#include "QcCore/net/InetAddress.h"
#include "QcCore/net/ServerSocket.h"
#include "QcCore/net/Socket.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/base/Exception.h"
#include "QcCore/base/System.h"
#include "QcCore/base/Thread.h"
#include "QcCore/base/Tracer.h"
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

	COUT << QC_T("  -h, --help                display this help") << endl;
	COUT << QC_T("  -p, --port                port to listen on") << endl;
	COUT << QC_T("  -r, --reusaddr            set the SO_REUSADDR option") << endl;
	COUT << QC_T("  -t, --timeout             milliseconds to wait for connection") << endl;
}

int main(int argc, char* argv[])
{
	MemCheckSystemMonitor monitor;

	CommandLineParser cmdlineParser;

	BasicOption optHelp(QC_T("help"), 'h', BasicOption::none);
	BasicOption optReusaddr(QC_T("reusaddr"), 'r', BasicOption::none);
	BasicOption optPort(QC_T("port"), 'p', BasicOption::mandatory);
	BasicOption optTimeout(QC_T("timeout"), 't', BasicOption::mandatory);
	BasicOption optTrace(QC_T("trace"), 0, BasicOption::mandatory);

	cmdlineParser.addOption(&optHelp);
	cmdlineParser.addOption(&optPort);
	cmdlineParser.addOption(&optReusaddr);
	cmdlineParser.addOption(&optTimeout);
	cmdlineParser.addOption(&optTrace);

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
	// If we have been asked to enable tracing, so it
	//
	if(optTrace.isPresent())
	{
		Tracer::SetTracer(new TraceHelper(cmdlineParser.getProgramName(), optTrace.getArgument()));
	}

	//
	// Determine local host info
	//
	AutoPtr<InetAddress> rpLocal = InetAddress::GetLocalHost();
	COUT << QC_T("Server running on host ") << rpLocal->toString() << endl;

	if(optHelp.isPresent())
	{
		showUsage(cmdlineParser.getProgramName());
		return (0);
	}

	int port = 0;
	if(optPort.isPresent())
	{
		port = NumUtils::ToInt(optPort.getArgument());
	}

#if defined(QC_MT) && !defined(WIN32)
	Thread::SetInterruptSignal(SIGALRM);
#endif

	try
	{
		//
		// W. Richard Stevens' Unix Network Programming
		// suggests always setting SO_REUSADDR for TCP Servers.
		// This must be set before bind() so we must create an unbound socket
		// and then call bind() after setting the option
		//
		AutoPtr<ServerSocket> rpServerSocket = new ServerSocket();
		rpServerSocket->setReuseAddress(optReusaddr.isPresent());
		rpServerSocket->bind(port);

		COUT << QC_T("Listening on socket: ") << rpServerSocket->toString() << endl;

		if(optTimeout.isPresent())
		{
			size_t timeout = NumUtils::ToInt(optTimeout.getArgument());
			rpServerSocket->setSoTimeout(timeout);
		}

		AutoPtr<Listener> rpListener = new Listener(rpServerSocket.get());

		//
		// In multi-threaded builds the listener object is a Thread, but in single-threaded
		// builds it is a simple implemententation of Runnable.
		//
#ifdef QC_MT
		rpListener->start();
		rpListener->join();
#else
		rpListener->run();
#endif
	}
	catch(Exception& e)
	{
		CERR << e.toString() << endl;
	}

	COUT << QC_T("Terminating server") << endl;

	return 0;
}

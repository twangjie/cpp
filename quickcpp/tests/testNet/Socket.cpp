#include "QcCore/base/System.h"
#include "QcCore/io/Console.h"
#include "QcCore/base/Exception.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/util/AttributeListParser.h"

using namespace qc;
using namespace qc::io;
using namespace qc::util;

String getTestAttribute(const String& name);
void testMessage(const String& msg);
void testFailed(const String& test);
void testPassed(const String& test);
void goodCatch(const String& test, const String& eMsg);
void uncaughtException(const String& e, const String& test);


#include "QcCore/net/InetAddress.h"
#include "QcCore/net/Socket.h"
#include "QcCore/net/SocketException.h"

using namespace qc::net; 


void Socket_Tests()
{
	testMessage(QC_T("Starting tests for Socket"));

	const int wwwPort = 80;
	String server = getTestAttribute(QC_T("server"));
	if(server.empty())
		server = QC_T("www.baidu.com");

	//
	// Test socket processing by connecting to a well-knwon HTTP server
	//
	AutoPtr<InetAddress> rpAddr = InetAddress::GetByName(server);
	AutoPtr<Socket> rpSocket = new Socket;
	try
	{
		if(!rpSocket->isConnected()) {testPassed(QC_T("isConnected"));} else {testFailed(QC_T("isConnected"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("isConnected"));
	}
	try
	{
		rpSocket->connect(rpAddr.get(), wwwPort); testPassed(QC_T("connect"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("connect"));
	}
	try
	{
		rpSocket->connect(InetAddress::GetByName(server).get(), wwwPort);
		testFailed(QC_T("(re)connect"));
	}
	catch(SocketException& e)
	{
		goodCatch(QC_T("(re)connect"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("(re)connect"));
	}
	try
	{
		if(rpSocket->isConnected()) {testPassed(QC_T("isConnected1"));} else {testFailed(QC_T("isConnected1"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("isConnected1"));
	}
	try
	{
		if(rpSocket->getPort()==wwwPort) {testPassed(QC_T("getPort"));} else {testFailed(QC_T("getPort"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getPort"));
	}
	try
	{
		if(rpSocket->getInetAddress() && rpSocket->getInetAddress()->equals(*rpAddr)) {testPassed(QC_T("getInetAddress"));} else {testFailed(QC_T("getInetAddress"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getInetAddress"));
	}

	//
	// Close the socket, then check for closed exceptions
	//
	try
	{
		rpSocket->close(); testPassed(QC_T("close"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close"));
	}
	try
	{
		if(!rpSocket->isConnected()) {testPassed(QC_T("isConnected2"));} else {testFailed(QC_T("isConnected2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("isConnected2"));
	}



	testMessage(QC_T("End of tests for Socket"));
}


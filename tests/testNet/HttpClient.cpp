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


#include "QcCore/net/URL.h"
#include "QcCore/net/HttpURLConnection.h"
#include "QcCore/io/InputStream.h"

using namespace qc::io; 
using namespace qc::net; 


void HttpClient_Tests()
{
	testMessage(QC_T("Starting tests for HttpClient"));

	String server = getTestAttribute(QC_T("server"));
	if(server.empty())
		server = QC_T("www.baidu.com/");

	server.insert(0, QC_T("http://"));

	AutoPtr<InputStream> rpIS;
	AutoPtr<HttpURLConnection> rpUrlConn;
	URL url1(server); 

	// Test accessing the stream directly
	try
	{
		rpIS = url1.openStream(); testPassed(QC_T("openStream"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("openStream"));
	}
	if(rpIS)
	{
		try
		{
			/* 这条测试语句不完善，对于大多数web服务器来说是正确的，但是对于google的web服务器就是有问题的。
		       Google的web服务器会在<之前返回一段字符串，如：‘1000’
			*/
			if(rpIS && rpIS->read() == '<') {testPassed(QC_T("checkStream"));} else {testFailed(QC_T("checkStream"));}
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("checkStream"));
		}

		// Test accessing the URLConnection
		try
		{
			rpUrlConn = static_cast<HttpURLConnection*>(url1.openConnection().get()); testPassed(QC_T("openURLConnection"));
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("openURLConnection"));
		}
		try
		{
			if(rpUrlConn) {testPassed(QC_T("connection returned"));} else {testFailed(QC_T("connection returned"));}
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("connection returned"));
		}
		if(rpUrlConn)
		{
			try
			{
				rpUrlConn->connect(); testPassed(QC_T("connect"));
			}
			catch(Exception& e)
			{
				uncaughtException(e.toString(), QC_T("connect"));
			}
			long resourceSize =  rpUrlConn->getContentLength();
			try
			{
				if(resourceSize != -1) {testPassed(QC_T("checkLength"));} else {testFailed(QC_T("checkLength"));}
			}
			catch(Exception& e)
			{
				uncaughtException(e.toString(), QC_T("checkLength"));
			}
			try
			{
				if(rpUrlConn->isConnected()) {testPassed(QC_T("isConnected"));} else {testFailed(QC_T("isConnected"));}
			}
			catch(Exception& e)
			{
				uncaughtException(e.toString(), QC_T("isConnected"));
			}
			if(rpUrlConn->isConnected())
			{
				try
				{
					if(rpUrlConn->getResponseCode() == HttpURLConnection::HTTP_OK) {testPassed(QC_T("checkResponse"));} else {testFailed(QC_T("checkResponse"));}
				}
				catch(Exception& e)
				{
					uncaughtException(e.toString(), QC_T("checkResponse"));
				}
				try
				{
					if(rpUrlConn->getDate().isValid()) {testPassed(QC_T("getDate"));} else {testFailed(QC_T("getDate"));}
				}
				catch(Exception& e)
				{
					uncaughtException(e.toString(), QC_T("getDate"));
				}

				try
				{
					rpIS = rpUrlConn->getInputStream(); testPassed(QC_T("getInputStream"));
				}
				catch(Exception& e)
				{
					uncaughtException(e.toString(), QC_T("getInputStream"));
				}
				try
				{
					if(rpIS) {testPassed(QC_T("valid input stream"));} else {testFailed(QC_T("valid input stream"));}
				}
				catch(Exception& e)
				{
					uncaughtException(e.toString(), QC_T("valid input stream"));
				}
				if(rpIS)
				{
					try
					{
						if(rpIS->read() == '<') {testPassed(QC_T("checkStream2"));} else {testFailed(QC_T("checkStream2"));}
					}
					catch(Exception& e)
					{
						uncaughtException(e.toString(), QC_T("checkStream2"));
					}
					try
					{
						if(rpIS->skip(resourceSize-1) == (size_t)resourceSize-1) {testPassed(QC_T("skip"));} else {testFailed(QC_T("skip"));}
					}
					catch(Exception& e)
					{
						uncaughtException(e.toString(), QC_T("skip"));
					}
					try
					{
						if(rpIS->read() == InputStream::EndOfFile) {testPassed(QC_T("eof"));} else {testFailed(QC_T("eof"));}
					}
					catch(Exception& e)
					{
						uncaughtException(e.toString(), QC_T("eof"));
					}
				}
			}
		}
	}



	testMessage(QC_T("End of tests for HttpClient"));
}


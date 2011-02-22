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


#include "QcCore/io/BufferedReader.h"
#include "QcCore/base/NullPointerException.h"
#include "QcCore/base/IllegalArgumentException.h"
#include "QcCore/io/ByteArrayInputStream.h"
#include "QcCore/io/InputStreamReader.h"

using namespace qc::io;


void BufferedReader_Tests()
{
	testMessage(QC_T("Starting tests for BufferedReader"));

	//
	// Basic API tests
	//

	try
	{
		AutoPtr<BufferedReader> rpBufferedReader =
	              new BufferedReader(NULL);
		testFailed(QC_T("constructor 1 null ptr exception"));
	}
	catch(NullPointerException& e)
	{
		goodCatch(QC_T("constructor 1 null ptr exception"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("constructor 1 null ptr exception"));
	}

	try
	{
		AutoPtr<BufferedReader> rpBufferedReader =
	              new BufferedReader(NULL, 0);
		testFailed(QC_T("constructor 2 null ptr exception"));
	}
	catch(NullPointerException& e)
	{
		goodCatch(QC_T("constructor 2 null ptr exception"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("constructor 2 null ptr exception"));
	}

	const char data[] = "Hello\rWorld\n\r\n";

	AutoPtr<ByteArrayInputStream> rpIS1 = new ByteArrayInputStream((const Byte*)data, sizeof(data)-1);
	AutoPtr<InputStreamReader> rpISR = new InputStreamReader(rpIS1.get());
	AutoPtr<BufferedReader> rpRdr = new BufferedReader(rpISR.get());
	try
	{
		if(rpRdr->markSupported()) {testPassed(QC_T("markSupported"));} else {testFailed(QC_T("markSupported"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("markSupported"));
	}

	try
	{
		rpRdr->read(NULL,1);
		testFailed(QC_T("read throws NullPointerException"));
	}
	catch(NullPointerException& e)
	{
		goodCatch(QC_T("read throws NullPointerException"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read throws NullPointerException"));
	}
	CharType buffer[1];
	try
	{
		rpRdr->read(buffer, 0);
		testFailed(QC_T("read throws IllegalArgumentException"));
	}
	catch(IllegalArgumentException& e)
	{
		goodCatch(QC_T("read throws IllegalArgumentException"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read throws IllegalArgumentException"));
	}

	//
	// Test readline() functionality
	//
	String line;
	try
	{
		if((5 == rpRdr->readLine(line) && line == QC_T("Hello"))) {testPassed(QC_T("readline 1"));} else {testFailed(QC_T("readline 1"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("readline 1"));
	}
	try
	{
		if((5 == rpRdr->readLine(line) && line == QC_T("World"))) {testPassed(QC_T("readline 2"));} else {testFailed(QC_T("readline 2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("readline 2"));
	}
	try
	{
		if((0 == rpRdr->readLine(line) && line.empty())) {testPassed(QC_T("readline 3"));} else {testFailed(QC_T("readline 3"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("readline 3"));
	}
	try
	{
		if((BufferedReader::EndOfFile == rpRdr->readLine(line) && line.empty())) {testPassed(QC_T("readline 4"));} else {testFailed(QC_T("readline 4"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("readline 4"));
	}


	testMessage(QC_T("End of tests for BufferedReader"));
}


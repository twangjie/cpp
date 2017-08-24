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


#include "QcCore/io/BufferedInputStream.h"
#include "QcCore/io/ByteArrayInputStream.h"
#include "QcCore/base/NullPointerException.h"
#include "QcCore/base/IllegalArgumentException.h"
#include "QcCore/io/IOException.h"

#include <memory.h>

using namespace qc::io;


void BufferedInputStream_Tests()
{
	testMessage(QC_T("Starting tests for BufferedInputStream"));

	//
	// Basic API tests
	//

	try
	{
		AutoPtr<BufferedInputStream> rpBufferedInputStream =
	              new BufferedInputStream(NULL);
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
		AutoPtr<BufferedInputStream> rpBufferedInputStream =
	              new BufferedInputStream(NULL, 1);
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

	const char data[] = "Hello World\n";

	AutoPtr<ByteArrayInputStream> rpIS1 = new ByteArrayInputStream((const Byte*)data, sizeof(data));
	AutoPtr<BufferedInputStream> rpBIS = new BufferedInputStream(rpIS1.get());
	try
	{
		if(rpBIS->available() == sizeof(data)) {testPassed(QC_T("available"));} else {testFailed(QC_T("available"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("available"));
	}
	try
	{
		if(rpBIS->markSupported()) {testPassed(QC_T("markSupported"));} else {testFailed(QC_T("markSupported"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("markSupported"));
	}

	// Allocate a small byte buffer to perform read operations
	const int iBufLen = 4;
	Byte buffer[iBufLen];
	long l,j;

	try
	{
		rpBIS->read(NULL,1);
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
	try
	{
		rpBIS->read(buffer, 0);
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

	try
	{
		if((l=rpBIS->read(buffer, iBufLen))==iBufLen) {testPassed(QC_T("bytes read 1"));} else {testFailed(QC_T("bytes read 1"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("bytes read 1"));
	}
	try
	{
		if(::memcmp(buffer, data, l) == 0) {testPassed(QC_T("data read ok 1"));} else {testFailed(QC_T("data read ok 1"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("data read ok 1"));
	}

	try
	{
		if((j=rpBIS->read(buffer, iBufLen))==iBufLen) {testPassed(QC_T("bytes read 2"));} else {testFailed(QC_T("bytes read 2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("bytes read 2"));
	}
	try
	{
		if(::memcmp(buffer, data+l, j) == 0) {testPassed(QC_T("data read ok 2"));} else {testFailed(QC_T("data read ok 2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("data read ok 2"));
	}

	try
	{
		if(rpBIS->read()==*(data+l+j)) {testPassed(QC_T("byte read"));} else {testFailed(QC_T("byte read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("byte read"));
	}


	testMessage(QC_T("End of tests for BufferedInputStream"));
}


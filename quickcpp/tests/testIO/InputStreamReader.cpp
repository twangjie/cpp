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


#include "QcCore/io/InputStreamReader.h"
#include "QcCore/io/FileInputStream.h"
#include "QcCore/io/FileOutputStream.h"
#include "QcCore/io/File.h"
#include "QcCore/io/IOException.h"
#include "QcCore/io/UnsupportedEncodingException.h"
#include "QcCore/base/NullPointerException.h"

using namespace qc::io;


void InputStreamReader_Tests()
{
	testMessage(QC_T("Starting tests for InputStreamReader"));

	//
	// Basic API tests
	//

	File testFile(QC_T("test.out"));

	//
	// create a small file to work on
	//
	Byte buffer[3] = {'a', 'b', 'c'};
	AutoPtr<FileOutputStream> rpOut = new FileOutputStream(testFile);
	try
	{
		rpOut->write(buffer, sizeof(buffer)); testPassed(QC_T("write"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("write"));
	}
	try
	{
		rpOut->close(); testPassed(QC_T("close"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close"));
	}
	try
	{
		if(testFile.exists()) {testPassed(QC_T("exists"));} else {testFailed(QC_T("exists"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("exists"));
	}
	try
	{
		if(testFile.length()==sizeof(buffer)) {testPassed(QC_T("length"));} else {testFailed(QC_T("length"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("length"));
	}

	AutoPtr<FileInputStream> rpIn = new FileInputStream(testFile);

	try
	{
		InputStreamReader x(rpIn.get(), QC_T("qwertyuiop"));
		testFailed(QC_T("unsupported"));
	}
	catch(UnsupportedEncodingException& e)
	{
		goodCatch(QC_T("unsupported"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("unsupported"));
	}
	try
	{
		InputStreamReader x(0);
		testFailed(QC_T("null"));
	}
	catch(NullPointerException& e)
	{
		goodCatch(QC_T("null"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("null"));
	}
	try
	{
		InputStreamReader x(rpIn.get(), (CodeConverter*)0);
		testFailed(QC_T("null"));
	}
	catch(NullPointerException& e)
	{
		goodCatch(QC_T("null"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("null"));
	}

	AutoPtr<InputStreamReader> rpTest = new InputStreamReader(rpIn.get(), QC_T("us-ascii"));

	try
	{
		if(rpTest->markSupported()==false) {testPassed(QC_T("markSupported"));} else {testFailed(QC_T("markSupported"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("markSupported"));
	}
	try
	{
		rpTest->mark(1);
		testFailed(QC_T("mark"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("mark"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("mark"));
	}
	try
	{
		rpTest->reset();
		testFailed(QC_T("reset"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("reset"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("reset"));
	}

	CharType input[3];

	try
	{
		if(rpTest->read()=='a') {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	try
	{
		if(rpTest->read(input+1, 2)==2) {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	try
	{
		if(*(input+1)=='b') {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	try
	{
		if(rpTest->skip(1)==0) {testPassed(QC_T("skip"));} else {testFailed(QC_T("skip"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("skip"));
	}
	try
	{
		if(rpTest->read()==InputStream::EndOfFile) {testPassed(QC_T("eof"));} else {testFailed(QC_T("eof"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("eof"));
	}
	try
	{
		if(rpTest->readAtomic().isEOF()) {testPassed(QC_T("eof"));} else {testFailed(QC_T("eof"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("eof"));
	}
	try
	{
		rpTest->close(); testPassed(QC_T("close"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close"));
	}

	// delete the file, ready for next time
	try
	{
		testFile.deleteFile(); testPassed(QC_T("delete"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("delete"));
	}


	testMessage(QC_T("End of tests for InputStreamReader"));
}


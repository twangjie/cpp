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


#include "QcCore/io/OutputStreamWriter.h"
#include "QcCore/io/FileInputStream.h"
#include "QcCore/io/FileOutputStream.h"
#include "QcCore/io/File.h"
#include "QcCore/io/IOException.h"
#include "QcCore/io/UnsupportedEncodingException.h"
#include "QcCore/base/NullPointerException.h"

using namespace qc::io;


void OutputStreamWriter_Tests()
{
	testMessage(QC_T("Starting tests for OutputStreamWriter"));

	//
	// Basic API tests
	//

	File testFile(QC_T("test.out"));

	//
	// create a small output file
	//
	CharType buffer[3] = {'a', 'b', 'c'};
	AutoPtr<FileOutputStream> rpOut = new FileOutputStream(testFile);

	try
	{
		OutputStreamWriter x(rpOut.get(), QC_T("qwertyuiop"));
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
		OutputStreamWriter x(0);
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
		OutputStreamWriter x(rpOut.get(), (CodeConverter*)0);
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

	AutoPtr<OutputStreamWriter> rpTest = new OutputStreamWriter(rpOut.get(), QC_T("us-ascii"));

	try
	{
		rpTest->write(0, sizeof(buffer)/sizeof(CharType));
		testFailed(QC_T("write"));
	}
	catch(NullPointerException& e)
	{
		goodCatch(QC_T("write"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("write"));
	}
	try
	{
		rpTest->write(buffer, 0); testPassed(QC_T("write"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("write"));
	}
	try
	{
		rpTest->write(buffer, sizeof(buffer)/sizeof(CharType)); testPassed(QC_T("write"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("write"));
	}
	try
	{
		rpTest->write('d'); testPassed(QC_T("write"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("write"));
	}
	try
	{
		rpTest->close(); testPassed(QC_T("close"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close"));
	}
	try
	{
		if(testFile.length()==4) {testPassed(QC_T("length"));} else {testFailed(QC_T("length"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("length"));
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


	testMessage(QC_T("End of tests for OutputStreamWriter"));
}


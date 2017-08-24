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


#include "QcCore/io/FileOutputStream.h"
#include "QcCore/io/File.h"
#include "QcCore/io/IOException.h"
#include "QcCore/io/FileNotFoundException.h"

using namespace qc::io;


void FileOutputStream_Tests()
{
	testMessage(QC_T("Starting tests for FileOutputStream"));

	//
	// Basic API tests
	//

	File badFile1(QC_T(""));
	File badFile2(QC_T("."));
	File testFile(QC_T("test.out"));

	Byte buffer[3] = {1, 2, 3};

	try
	{
		FileOutputStream x(badFile1);
		testFailed(QC_T("badFile1"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("badFile1"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("badFile1"));
	}
	try
	{
		FileOutputStream x(badFile2);
		testFailed(QC_T("badFile2"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("badFile2"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("badFile2"));
	}

	try
	{
		if(testFile.exists()==false) {testPassed(QC_T("exists"));} else {testFailed(QC_T("exists"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("exists"));
	}
	
	AutoPtr<FileOutputStream> rpTest = new FileOutputStream(testFile);
	try
	{
		rpTest->write(buffer[0]); testPassed(QC_T("write"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("write"));
	}
	try
	{
		rpTest->write(buffer+1, 2); testPassed(QC_T("write"));
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
		if(testFile.exists()) {testPassed(QC_T("exists"));} else {testFailed(QC_T("exists"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("exists"));
	}
	try
	{
		if(testFile.length()==3) {testPassed(QC_T("length"));} else {testFailed(QC_T("length"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("length"));
	}

	// now append to the file
	rpTest = new FileOutputStream(testFile.getPath(), true);
	try
	{
		rpTest->write(4); testPassed(QC_T("write"));
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


	testMessage(QC_T("End of tests for FileOutputStream"));
}


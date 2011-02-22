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


#include "QcCore/io/FileInputStream.h"
#include "QcCore/io/FileOutputStream.h"
#include "QcCore/io/File.h"
#include "QcCore/io/IOException.h"
#include "QcCore/io/FileNotFoundException.h"

using namespace qc::io;


void FileInputStream_Tests()
{
	testMessage(QC_T("Starting tests for FileInputStream"));

	//
	// Basic API tests
	//

	File badFile1(QC_T(""));
	File badFile2(QC_T("."));
	File testFile(QC_T("test.out"));

	try
	{
		FileInputStream x(badFile1);
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
		FileInputStream x(badFile2);
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
		FileInputStream x(testFile);
		testFailed(QC_T("testFile"));
	}
	catch(FileNotFoundException& e)
	{
		goodCatch(QC_T("testFile"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("testFile"));
	}

	//
	// create a small file to work on
	//
	Byte buffer[3] = {1, 2, 3};
	AutoPtr<FileOutputStream> rpOut;
	try
	{
		rpOut = new FileOutputStream(testFile); testPassed(QC_T("new1"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("new1"));
	}
	if(rpOut)
	{
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
		rpOut->close(); testPassed(QC_T("close1"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close1"));
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
	}

	AutoPtr<FileInputStream> rpTest;
	try
	{
		rpTest = new FileInputStream(testFile); testPassed(QC_T("new2"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("new2"));
	}
	if(rpTest)
	{
		// QuickCPP API states it returns zero for available on FileInputStream
		try
	{
		if(rpTest->available()==0) {testPassed(QC_T("available"));} else {testFailed(QC_T("available"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("available"));
	}
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

		Byte input[3];

		try
	{
		if(rpTest->read()==1) {testPassed(QC_T("read1"));} else {testFailed(QC_T("read1"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read1"));
	}
		try
	{
		if(rpTest->read(input+1, 2)==2) {testPassed(QC_T("read2"));} else {testFailed(QC_T("read2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read2"));
	}
		try
	{
		if(*(input+1)==2) {testPassed(QC_T("read3"));} else {testFailed(QC_T("read3"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read3"));
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
		rpTest->close(); testPassed(QC_T("close2"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close2"));
	}
	}

	// Make the file read-only then read it again
	try
	{
		testFile.setReadOnly(true); testPassed(QC_T("setReadOnly"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setReadOnly"));
	};
	try
	{
		rpTest.release(); rpTest = new FileInputStream(testFile); testPassed(QC_T("open stream"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("open stream"));
	}
	if(rpTest)
	{
		try
	{
		if(rpTest->read()==1) {testPassed(QC_T("read4"));} else {testFailed(QC_T("read4"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read4"));
	}
		try
	{
		rpTest->close(); testPassed(QC_T("close3"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close3"));
	}
	}

	// Make the file writable then delete it
	try
	{
		testFile.setReadOnly(false); testPassed(QC_T("setReadOnly"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setReadOnly"));
	};
	try
	{
		testFile.deleteFile(); testPassed(QC_T("delete"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("delete"));
	}


	testMessage(QC_T("End of tests for FileInputStream"));
}


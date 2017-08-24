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


#include "QcCore/base/Character.h"
#include "QcCore/base/NumUtils.h"

using namespace qc; 


void NumUtils_Tests()
{
	testMessage(QC_T("Starting tests for NumUtils"));

    //
	int i1 = -12345;
	unsigned int ui1 = 12345;
	long l1 = -1234567;
	unsigned int ul1 = 1234567;
	double f1 = 123456.789000;
	
	String s1 = QC_T("-12345");
	String s2 = QC_T("12345");
	String s3 = QC_T("-1234567");
	String s4 = QC_T("1234567");
	String s5 = QC_T("123456.789000");

	try
	{
		if(NumUtils::ToString(i1)==s1) {testPassed(QC_T("ToString1"));} else {testFailed(QC_T("ToString1"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("ToString1"));
	};
	try
	{
		if(NumUtils::ToString(ui1)==s2) {testPassed(QC_T("ToString2"));} else {testFailed(QC_T("ToString2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("ToString2"));
	};
	try
	{
		if(NumUtils::ToString(l1)==s3) {testPassed(QC_T("ToString3"));} else {testFailed(QC_T("ToString3"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("ToString3"));
	};
	try
	{
		if(NumUtils::ToString(ul1)==s4) {testPassed(QC_T("ToString4"));} else {testFailed(QC_T("ToString4"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("ToString4"));
	};
	try
	{
		if(NumUtils::ToString(f1)==s5) {testPassed(QC_T("ToString5"));} else {testFailed(QC_T("ToString5"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("ToString5"));
	};
	try
	{
		if(NumUtils::ToInt(s1)==i1) {testPassed(QC_T("ToInt"));} else {testFailed(QC_T("ToInt"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("ToInt"));
	};
	try
	{
		if(NumUtils::ToLong(s3)==l1) {testPassed(QC_T("ToLong"));} else {testFailed(QC_T("ToLong"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("ToLong"));
	};
	try
	{
		if(NumUtils::ToDouble(s5)==f1) {testPassed(QC_T("ToDouble"));} else {testFailed(QC_T("ToDouble"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("ToDouble"));
	};


	testMessage(QC_T("End of tests for NumUtils"));
}


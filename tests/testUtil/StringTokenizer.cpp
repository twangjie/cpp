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
#include "QcCore/util/StringTokenizer.h"

using namespace qc::util; 


void StringTokenizer_Tests()
{
	testMessage(QC_T("Starting tests for StringTokenizer"));

    //

	String seq = QC_T("Hello \t World!\n");
	String hello  = QC_T("Hello");
	String world  = QC_T("World!");
	String ws = QC_T(" \t\n");

    Character nbsp(0xa0);
	String ws2 = ws + nbsp.toString();
	String seq2 = ws + hello + nbsp.toString() + world + ws;
	String seq3 = QC_T("  H\t \nW ");

	std::vector<String> vec1 = StringTokenizer(seq).toVector();
	try
	{
		if(vec1.size()==2) {testPassed(QC_T("tok0"));} else {testFailed(QC_T("tok0"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("tok0"));
	};
	try
	{
		if(vec1[0]==hello) {testPassed(QC_T("tok1"));} else {testFailed(QC_T("tok1"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("tok1"));
	};
	try
	{
		if(vec1[1]==world) {testPassed(QC_T("tok2"));} else {testFailed(QC_T("tok2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("tok2"));
	};

	std::vector<String> vec2 = StringTokenizer(seq2).toVector();
	try
	{
		if(vec2.size()==1) {testPassed(QC_T("tok3"));} else {testFailed(QC_T("tok3"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("tok3"));
	};

	std::vector<String> vec3 = StringTokenizer(seq2, ws2, true).toVector();
	try
	{
		if(vec3.size()==5) {testPassed(QC_T("tok4"));} else {testFailed(QC_T("tok4"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("tok4"));
	};
	try
	{
		if(vec3[2]==nbsp.toString()) {testPassed(QC_T("tok5"));} else {testFailed(QC_T("tok5"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("tok5"));
	};

	std::vector<String> vec4 = StringTokenizer(seq3, ws, true, false).toVector();
	try
	{
		if(vec4.size()==seq3.size()) {testPassed(QC_T("tok6"));} else {testFailed(QC_T("tok6"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("tok6"));
	};
	try
	{
		if(vec4[0]==QC_T(" ")) {testPassed(QC_T("tok7"));} else {testFailed(QC_T("tok7"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("tok7"));
	};
	try
	{
		if(vec4[2]==QC_T("H")) {testPassed(QC_T("tok8"));} else {testFailed(QC_T("tok8"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("tok8"));
	};


	testMessage(QC_T("End of tests for StringTokenizer"));
}


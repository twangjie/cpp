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
#include "QcCore/base/String.h"
#include "QcCore/base/StringIterator.h"

#include <algorithm>


void StringIterator_Tests()
{
	testMessage(QC_T("Starting tests for StringIterator"));

    //

	String seq = QC_T("Hello \t World!\n");
	String hello  = QC_T("Hello");
	String world  = QC_T("World!");
	String ws = QC_T(" \t\n");

    Character nbsp(0xa0);
	String ws2 = ws + nbsp.toString();
	String seq2 = ws + hello + nbsp.toString() + world + ws;

	StringIterator begin(seq.data());
	StringIterator end(seq.data()+seq.size());
	StringIterator firstDelim(ws.data());
	StringIterator endDelim(ws.data()+ws.size());

	//
	// Test various construction methods
	//
	StringIterator space;
	StringIterator space2 = space;
	StringIterator space3(space2);
	try
	{
		if(space == space2 && space2 == space3) {testPassed(QC_T("equality"));} else {testFailed(QC_T("equality"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("equality"));
	};

	space3 = std::find_first_of(begin, end, firstDelim, endDelim);
	try
	{
		if(space3==(seq.data()+5)) {testPassed(QC_T("find_first_of"));} else {testFailed(QC_T("find_first_of"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("find_first_of"));
	};

	StringIterator iW = std::find(begin, end, QC_T('W'));
	try
	{
		if(*iW == QC_T('W')) {testPassed(QC_T("find"));} else {testFailed(QC_T("find"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("find"));
	};


	testMessage(QC_T("End of tests for StringIterator"));
}


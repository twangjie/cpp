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


#include "QcXml/sax/XMLReaderFactory.h"
#include "QcXml/sax/XMLReader.h"
#include "QcXml/sax/AttributesImpl.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/base/IllegalArgumentException.h"

#include <stdio.h>

using namespace qc; 
using namespace qc::sax; 
using namespace qc::io; 


void AttributesImpl_Tests()
{
	testMessage(QC_T("Starting tests for AttributesImpl"));

	const size_t iNumAttributes = 10;
	String value;

	AttributesImpl impl1;
	size_t i;
	for(i = 0; i < iNumAttributes; i++)
	{
		String qnameStr = String(QC_T("qc:attribute")) + NumUtils::ToString(i);
		try
		{
			impl1.addAttribute(QC_T("http://google.com/ns"), qnameStr, QC_T("CDATA"), QC_T("hello")); testPassed(QC_T("addAttribute"));
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("addAttribute"));
		}
		try
		{
			if(impl1.getLength() == i+1) {testPassed(QC_T("getLength"));} else {testFailed(QC_T("getLength"));}
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("getLength"));
		}
	}

	AttributesImpl impl2(impl1);
	try
	{
		if(impl2.getLength() == iNumAttributes) {testPassed(QC_T("getLength2"));} else {testFailed(QC_T("getLength2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getLength2"));
	}

	// test that setAttributes works
	AttributesImpl impl3;
	try
	{
		impl3.setAttributes(impl2); testPassed(QC_T("setAttributes"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setAttributes"));
	}
	try
	{
		if(impl2.getLength() == iNumAttributes) {testPassed(QC_T("getLength3"));} else {testFailed(QC_T("getLength3"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getLength3"));
	}

	// test that clear works
	try
	{
		impl3.clear(); testPassed(QC_T("clear"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("clear"));
	}
	try
	{
		if(impl3.getLength() == 0) {testPassed(QC_T("getLength4"));} else {testFailed(QC_T("getLength4"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getLength4"));
	}

	try
	{
		impl3.setAttributes(impl2); testPassed(QC_T("setAttributes2"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setAttributes2"));
	}
	try
	{
		if(impl3.getLength() == impl2.getLength()) {testPassed(QC_T("getLength5"));} else {testFailed(QC_T("getLength5"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getLength5"));
	}

	for(i = 1; i<=iNumAttributes; i++)
	{
		try
		{
			impl3.removeAttribute(iNumAttributes-i); testPassed(QC_T("removeAttribute"));
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("removeAttribute"));
		}
		try
		{
			if(impl3.getLength() == iNumAttributes-i) {testPassed(QC_T("getLength6"));} else {testFailed(QC_T("getLength6"));}
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("getLength6"));
		}
		try
		{
			impl3.removeAttribute(iNumAttributes-i);
			testFailed(QC_T("removeAttribute throw"));
		}
		catch(qc::IllegalArgumentException& e)
		{
			goodCatch(QC_T("removeAttribute throw"), e.toString());
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("removeAttribute throw"));
		}
	}

	try
	{
		impl3.setAttributes(impl2); testPassed(QC_T("setAttributes3"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setAttributes3"));
	}
	try
	{
		if(impl3.getLength() == iNumAttributes) {testPassed(QC_T("getLength7"));} else {testFailed(QC_T("getLength7"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getLength7"));
	}
	try
	{
		if(impl3.getLocalName(1) == QC_T("attribute1")) {testPassed(QC_T("getLocalName"));} else {testFailed(QC_T("getLocalName"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getLocalName"));
	}
	// test that an invalid index returns an empty string
	try
	{
		if(impl3.getLocalName(iNumAttributes) == QC_T("")) {testPassed(QC_T("getLocalName2"));} else {testFailed(QC_T("getLocalName2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getLocalName2"));
	}

	// test that setAttribute throws when you supply an
	// index off the end of the array
	try
	{
		impl3.setAttribute(iNumAttributes, QC_T("newuri"), QC_T("qc:newqname"), QC_T("newtype"), QC_T("goodbye"));
		testFailed(QC_T("setAttribute throw"));
	}
	catch(qc::IllegalArgumentException& e)
	{
		goodCatch(QC_T("setAttribute throw"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setAttribute throw"));
	}
	// set the last element to have new values
	try
	{
		impl3.setAttribute(0, QC_T("newuri"), QC_T("qc:newqname"), QC_T("newtype"), QC_T("goodbye")); testPassed(QC_T("setAttribute2"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setAttribute2"));
	}
	try
	{
		if(impl3.getValue(0) == QC_T("goodbye")) {testPassed(QC_T("getValue"));} else {testFailed(QC_T("getValue"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getValue"));
	}

	// test that setQName throws when you supply an index off the end of the array
	try
	{
		impl3.setQName(iNumAttributes, QC_T("qc:new"));
		testFailed(QC_T("setQName throw"));
	}
	catch(qc::IllegalArgumentException& e)
	{
		goodCatch(QC_T("setQName throw"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setQName throw"));
	}

	// test that setQName works
	try
	{
		impl3.setQName(0, QC_T("qc:new")); testPassed(QC_T("setQName"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setQName"));
	}
	try
	{
		if(impl3.getQName(0) == QC_T("qc:new")) {testPassed(QC_T("getQName"));} else {testFailed(QC_T("getQName"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getQName"));
	}

	// test that getIndex (qname) works
	for (i = 0; i < iNumAttributes; i++)
	{
		String qnameStr = String(QC_T("qc:attribute")) + NumUtils::ToString(i);
		try
		{
			if(impl1.getIndex(qnameStr) == (int)i) {testPassed(QC_T("getIndex qname"));} else {testFailed(QC_T("getIndex qname"));}
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("getIndex qname"));
		}
	}

	// test that getIndex (qname) returns -1 if it cant find the attribute
	try
	{
		if(impl1.getIndex(QC_T("nonexistent")) == -1) {testPassed(QC_T("getIndex qname returns -1"));} else {testFailed(QC_T("getIndex qname returns -1"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getIndex qname returns -1"));
	}
	try
	{
		if(impl3.getIndex(QC_T("nosuchuri"), QC_T("nosuchname")) == -1) {testPassed(QC_T("getIndex"));} else {testFailed(QC_T("getIndex"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getIndex"));
	}

	// test that setType works
	try
	{
		impl3.setType(0, QC_T("ID")); testPassed(QC_T("setType"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setType"));
	}
	try
	{
		if(impl3.getType(0) == QC_T("ID")) {testPassed(QC_T("getType"));} else {testFailed(QC_T("getType"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getType"));
	}
	// check setType throws
	try
	{
		impl3.setType(iNumAttributes, QC_T("ID"));
		testFailed(QC_T("setType throw"));
	}
	catch(qc::IllegalArgumentException& e)
	{
		goodCatch(QC_T("setType throw"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setType throw"));
	}

	// test that setURI throws
	try
	{
		impl3.setURI(iNumAttributes, QC_T("http://google.com/ns"));
		testFailed(QC_T("setURI throw"));
	}
	catch(qc::IllegalArgumentException& e)
	{
		goodCatch(QC_T("setURI throw"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setURI throw"));
	}

	// test that setValue works
	try
	{
		impl3.setAttributes(impl2); testPassed(QC_T("setAttributes"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setAttributes"));
	}
	try
	{
		impl3.setValue(0, QC_T("newvalue")); testPassed(QC_T("setValue"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setValue"));
	}
	try
	{
		if(impl3.getValue(0) == QC_T("newvalue")) {testPassed(QC_T("getValue"));} else {testFailed(QC_T("getValue"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getValue"));
	}

	// test that setValue throws
	try
	{
		impl3.setValue(iNumAttributes, QC_T("newvalue"));
		testFailed(QC_T("setValue throw"));
	}
	catch(qc::IllegalArgumentException& e)
	{
		goodCatch(QC_T("setValue throw"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setValue throw"));
	}


	testMessage(QC_T("End of tests for AttributesImpl"));
}

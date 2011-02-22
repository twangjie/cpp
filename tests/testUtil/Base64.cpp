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
#include "QcCore/base/ArrayAutoPtr.h"
#include "QcCore/util/Base64.h"
#include <string.h>

using namespace qc::util; 

void testBase64(const char* pPlain, const char* pBase64)
{
	//
	// First convert the plain test to base-64
	//
	const size_t encSize = Base64::GetEncodedLength((const Byte*)pPlain, (const Byte*)pPlain+strlen(pPlain));
	ArrayAutoPtr<Byte> apBuffer(new Byte[encSize]);
	Byte* pNext;
	try
	{
		if(Base64::Encode((const Byte*)pPlain, (const Byte*)pPlain+strlen(pPlain), apBuffer.get(), apBuffer.get()+encSize, pNext) == Base64::ok) {testPassed(QC_T("Encode"));} else {testFailed(QC_T("Encode"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("Encode"));
	}
	size_t actualLength = pNext - apBuffer.get();
	try
	{
		if(encSize == actualLength) {testPassed(QC_T("length test"));} else {testFailed(QC_T("length test"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("length test"));
	}
	try
	{
		if(strncmp((const char*)apBuffer.get(), pBase64, encSize) == 0) {testPassed(QC_T("Base64 match"));} else {testFailed(QC_T("Base64 match"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("Base64 match"));
	}
	//
	// And then convert back again
	//
	const size_t maxDecodedSize = Base64::GetMaxDecodedLength((const Byte*)pBase64, (const Byte*)pBase64+strlen(pBase64));
	try
	{
		if(maxDecodedSize < encSize) {testPassed(QC_T("decoded length"));} else {testFailed(QC_T("decoded length"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("decoded length"));
	}

	try
	{
		if(Base64::Decode((const Byte*)pBase64, (const Byte*)pBase64+strlen(pBase64), apBuffer.get(), apBuffer.get()+encSize, pNext) == Base64::ok) {testPassed(QC_T("Decode"));} else {testFailed(QC_T("Decode"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("Decode"));
	}
	actualLength = pNext - apBuffer.get();
	try
	{
		if(maxDecodedSize >= actualLength) {testPassed(QC_T("dec length test"));} else {testFailed(QC_T("dec length test"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("dec length test"));
	}
	try
	{
		if(strncmp((const char*)apBuffer.get(), pPlain, actualLength) == 0) {testPassed(QC_T("plain match"));} else {testFailed(QC_T("plain match"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("plain match"));
	}
}


void Base64_Tests()
{
	testMessage(QC_T("Starting tests for Base64"));

    //
	const char* plain_1 = "Hello World!\r\n";
	const char* b64_1   = "SGVsbG8gV29ybGQhDQo=";
	const char* plain_2 = "qwertyuiop1234567890bvcxz\r\n";
	const char* b64_2   = "cXdlcnR5dWlvcDEyMzQ1Njc4OTBidmN4eg0K";
	const char* plain_3 = "Q";
	const char* b64_3   = "UQ==";
	const char* plain_4 = "1234567890.<ZXC:@\r\n";
	const char* b64_4   = "MTIzNDU2Nzg5MC48WlhDOkANCg==";
	const char* plain_5 = "blat!\r\n";
	const char* b64_5   = "YmxhdCENCg==";

	testBase64(plain_1, b64_1);
	testBase64(plain_2, b64_2);
	testBase64(plain_3, b64_3);
	testBase64(plain_4, b64_4);
	testBase64(plain_5, b64_5);


	testMessage(QC_T("End of tests for Base64"));
}


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
#include "QcCore/base/StringUtils.h"

using namespace qc; 


void StringUtils_Tests()
{
	testMessage(QC_T("Starting tests for StringUtils"));

    //
	String s1 = QC_T("abcdef");
	String s2 = QC_T("ABCDEF");
	const char * pS1 = "abcdef";
	const char * pS2 = "ABCDEF";

	try
	{
		if(StringUtils::CompareNoCase(s1, s2) == 0) {testPassed(QC_T("cmpncase1"));} else {testFailed(QC_T("cmpncase1"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("cmpncase1"));
	}
	try
	{
		if(StringUtils::CompareNoCase(pS1, pS2) == 0) {testPassed(QC_T("cmpncase2"));} else {testFailed(QC_T("cmpncase2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("cmpncase2"));
	}
	try
	{
		if(s2 == StringUtils::ToUpper(s1)) {testPassed(QC_T("toUpper"));} else {testFailed(QC_T("toUpper"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("toUpper"));
	}
	try
	{
		if(s1 == StringUtils::ToLower(s2)) {testPassed(QC_T("toLower"));} else {testFailed(QC_T("toLower"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("toLower"));
	}

	Character pound(163UL);
	ByteString strPound = StringUtils::ToLatin1(pound.toString());
	try
	{
		if(strPound[0] == char(163U)) {testPassed(QC_T("ToLatin1"));} else {testFailed(QC_T("ToLatin1"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("ToLatin1"));
	};

	const char pLatin1[] = "?23.00 please!";
	String sLatin1 = pound.toString() + QC_T("123.00 please!");
	try
	{
		if(sLatin1 == StringUtils::FromLatin1(pLatin1)) {testPassed(QC_T("fromLatin1"));} else {testFailed(QC_T("fromLatin1"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("fromLatin1"));
	}
	try
	{
		if(sLatin1 == StringUtils::FromLatin1(pLatin1, sizeof(pLatin1)-1)) {testPassed(QC_T("fromLatin1-2"));} else {testFailed(QC_T("fromLatin1-2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("fromLatin1-2"));
	}

#if !defined(QC_UCS2)
	Character hiChar(0x10f000UL);
#if defined(QC_UCS4)
	bool bMultiExpected = false;
#else
	bool bMultiExpected = true;
#endif
	try
	{
		if(StringUtils::ContainsMultiCharSequence(hiChar.toString()) == bMultiExpected) {testPassed(QC_T("ContainsMultiCharSequence"));} else {testFailed(QC_T("ContainsMultiCharSequence"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("ContainsMultiCharSequence"));
	}
#endif

	String repl1 = QC_T(" ppplease  pppick  uppp  the  ppphone ");
	String repl2 = QC_T(" please  pick  up  the  phone ");
	String repl3 = QC_T("please  pick  up  the  phone");
	String repl4 = QC_T("please pick up the phone");

	String repl = repl1;
	try
	{
		StringUtils::ReplaceAll(repl, QC_T("ppp"), QC_T("p")); testPassed(QC_T("ReplaceAll"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("ReplaceAll"));
	}
	try
	{
		if(repl == repl2) {testPassed(QC_T("ReplaceAll"));} else {testFailed(QC_T("ReplaceAll"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("ReplaceAll"));
	}
	try
	{
		if(StringUtils::StripWhiteSpace(repl2, StringUtils::both) == repl3) {testPassed(QC_T("StripWhiteSpace"));} else {testFailed(QC_T("StripWhiteSpace"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("StripWhiteSpace"));
	}
	try
	{
		if(StringUtils::StripWhiteSpace(repl3, StringUtils::both) == repl3) {testPassed(QC_T("StripWhiteSpace1"));} else {testFailed(QC_T("StripWhiteSpace1"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("StripWhiteSpace1"));
	}
	try
	{
		if(StringUtils::StripWhiteSpace(QC_T("   "), StringUtils::both).empty()) {testPassed(QC_T("StripWhiteSpace2"));} else {testFailed(QC_T("StripWhiteSpace2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("StripWhiteSpace2"));
	}
	try
	{
		if(StringUtils::StripWhiteSpace(QC_T("i\r\n"), StringUtils::both) == String(QC_T("i"))) {testPassed(QC_T("StripWhiteSpace3"));} else {testFailed(QC_T("StripWhiteSpace3"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("StripWhiteSpace3"));
	}
	try
	{
		if(StringUtils::StripWhiteSpace(QC_T("  i\r\n"), StringUtils::both) == String(QC_T("i"))) {testPassed(QC_T("StripWhiteSpace4"));} else {testFailed(QC_T("StripWhiteSpace4"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("StripWhiteSpace4"));
	}
	try
	{
		if(StringUtils::StripWhiteSpace(QC_T(" i"), StringUtils::both) == String(QC_T("i"))) {testPassed(QC_T("StripWhiteSpace5"));} else {testFailed(QC_T("StripWhiteSpace5"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("StripWhiteSpace5"));
	}
	try
	{
		if(StringUtils::StripWhiteSpace(QC_T("i"), StringUtils::both) == String(QC_T("i"))) {testPassed(QC_T("StripWhiteSpace6"));} else {testFailed(QC_T("StripWhiteSpace6"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("StripWhiteSpace6"));
	}
	try
	{
		if(StringUtils::NormalizeWhiteSpace(repl3) == repl4) {testPassed(QC_T("NormalizeWS"));} else {testFailed(QC_T("NormalizeWS"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("NormalizeWS"));
	}
	try
	{
		if(StringUtils::IsHexString("xyz") == false) {testPassed(QC_T("IsHexString1"));} else {testFailed(QC_T("IsHexString1"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("IsHexString1"));
	}
	try
	{
		if(StringUtils::IsHexString("fab") == true) {testPassed(QC_T("IsHexString1"));} else {testFailed(QC_T("IsHexString1"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("IsHexString1"));
	}


	testMessage(QC_T("End of tests for StringUtils"));
}


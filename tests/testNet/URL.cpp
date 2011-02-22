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


#include "QcCore/base/String.h"
#include "QcCore/base/StringUtils.h"
#include "QcCore/net/URL.h"
#include "QcCore/net/MalformedURLException.h"

using namespace qc::net; 

void testURL(const String& sURL,
	const String& sContext,
	const String& protocol,
	const String& host,
	const String& file,
	const String& ref,
	const String& query,
	const String& userInfo,
	int port)
{
	URL url;
	if(!sContext.empty())
	{
		URL urlContext(sContext);
		url = URL(urlContext, sURL);
	}
	else
	{
		url = URL(sURL);
	}

	URL url2(url.toExternalForm());
	try
	{
		if(url == url2) {testPassed(QC_T("roundTrip"));} else {testFailed(QC_T("roundTrip"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("roundTrip"));
	}

	try
	{
		if(url.getProtocol() == protocol) {testPassed(QC_T("getProtocol"));} else {testFailed(QC_T("getProtocol"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getProtocol"));
	}
	try
	{
		if(url.getHost() == host) {testPassed(QC_T("gethost"));} else {testFailed(QC_T("gethost"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("gethost"));
	}
	try
	{
		if(url.getFile() == file) {testPassed(QC_T("getFile"));} else {testFailed(QC_T("getFile"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getFile"));
	}
	//testMessage(url.getFile());
	try
	{
		if(url.getRef() == ref) {testPassed(QC_T("getRef"));} else {testFailed(QC_T("getRef"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getRef"));
	}
	try
	{
		if(url.getPort() == port) {testPassed(QC_T("getPort"));} else {testFailed(QC_T("getPort"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getPort"));
	}
	try
	{
		if(url.getQuery() == query) {testPassed(QC_T("getQuery"));} else {testFailed(QC_T("getQuery"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getQuery"));
	}
	try
	{
		if(url.getUserInfo() == userInfo) {testPassed(QC_T("getUserInfo"));} else {testFailed(QC_T("getUserInfo"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getUserInfo"));
	}
}


void URL_Tests()
{
	testMessage(QC_T("Starting tests for URL"));

	String s1 = QC_T("http://a.b.c/d/e#f");
	String sNull;

	try
	{
		testURL(s1, sNull, QC_T("http"), QC_T("a.b.c"), QC_T("/d/e"), QC_T("f"), sNull, sNull, -1); testPassed(QC_T("s1"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("s1"));
	}
	String s1a = QC_T("file#ref");
	try
	{
		testURL(s1a, s1, QC_T("http"), QC_T("a.b.c"), QC_T("/d/file"), QC_T("ref"), sNull, sNull, -1); testPassed(QC_T("s1a"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("s1a"));
	}

	String s2 = QC_T("http://www.baidu.com:8080/test.cgi?query+string#ref");
	try
	{
		testURL(s2, sNull, QC_T("http"), QC_T("www.baidu.com"), QC_T("/test.cgi?query+string"), QC_T("ref"), QC_T("query+string"), sNull, 8080); testPassed(QC_T("s2"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("s2"));
	}
	String s2a = QC_T("test2.cgi");
	try
	{
		testURL(s2a, s2, QC_T("http"), QC_T("www.baidu.com"), QC_T("/test2.cgi"), sNull, sNull, sNull, 8080); testPassed(QC_T("s2a"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("s2a"));
	}
	String s2b = QC_T("test2b.html#xxx");
	try
	{
		testURL(s2b, s2, QC_T("http"), QC_T("www.baidu.com"), QC_T("/test2b.html"), QC_T("xxx"), sNull, sNull, 8080); testPassed(QC_T("s2b"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("s2b"));
	}

	String s3 = QC_T("ftp://user:password@ftp.baidu.com/data/test.txt");
	try
	{
		testURL(s3, sNull, QC_T("ftp"), QC_T("ftp.baidu.com"), QC_T("/data/test.txt"), sNull, sNull, QC_T("user:password"), -1); testPassed(QC_T("s3"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("s3"));
	}
	String s3a = QC_T("test2.txt");
	try
	{
		testURL(s3a, s3, QC_T("ftp"), QC_T("ftp.baidu.com"), QC_T("/data/test2.txt"), sNull, sNull, QC_T("user:password"), -1); testPassed(QC_T("s3a"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("s3a"));
	}
	String s3b = QC_T("ftp://ftp.baidu.com:1234/data/test2.txt");
	try
	{
		testURL(s3b, s3, QC_T("ftp"), QC_T("ftp.baidu.com"), QC_T("/data/test2.txt"), sNull, sNull, sNull, 1234); testPassed(QC_T("s3b"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("s3b"));
	}

	String s4 = QC_T("XXXX://www.baidu.com/");
	try
	{
		URL url3(s4);
		testFailed(QC_T("malformed url"));
	}
	catch(MalformedURLException& e)
	{
		goodCatch(QC_T("malformed url"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("malformed url"));
	}

	URL comp1(QC_T("http://www.baidu.com/index.html"));
	URL comp2(QC_T("HTTP://WWW.BAIDU.COM:80/index.html"));
	URL comp2a(QC_T("HTTP://WWW.BAIDU.COM:8080/index.html"));
	URL comp2b(QC_T("HTTP://WWW.BAIDU.COM/"));
	URL comp3(QC_T("HTTP://BAIDU.COM/index.html"));
	try
	{
		if(comp1 == comp2) {testPassed(QC_T("equals"));} else {testFailed(QC_T("equals"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("equals"));
	}
	try
	{
		if(comp2 != comp2a) {testPassed(QC_T("equals"));} else {testFailed(QC_T("equals"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("equals"));
	}
	try
	{
		if(comp2 != comp2b) {testPassed(QC_T("equals"));} else {testFailed(QC_T("equals"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("equals"));
	}

	try
	{
		/* 主机名不同 */
		if(comp1 != comp3) {testPassed(QC_T("equals"));} else {testFailed(QC_T("equals"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("equals"));
	}

#ifdef WIN32
	String prefix;
#else
	String prefix = QC_T("/");
#endif

	String s5 = QC_T("file:///c:\\temp\\x.txt");
	try
	{
		testURL(s5, sNull, QC_T("file"), sNull, prefix + QC_T("c:/temp/x.txt"), sNull, sNull, sNull, -1); testPassed(QC_T("s5"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("s5"));
	}
	String s5a = QC_T("test2.txt");
	try
	{
		testURL(s5a, s5, QC_T("file"), sNull, prefix + QC_T("c:/temp/test2.txt"), sNull, sNull, sNull, -1); testPassed(QC_T("s5a"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("s5a"));
	}

	String s6 = QC_T("file://host/share/rootdir/filename");
	try
	{
		testURL(s6, sNull, QC_T("file"), QC_T("host"), QC_T("/share/rootdir/filename"), sNull, sNull, sNull, -1); testPassed(QC_T("s6"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("s6"));
	}
	String s6a = QC_T("test2.txt");
	try
	{
		testURL(s6a, s6, QC_T("file"), QC_T("host"), QC_T("/share/rootdir/test2.txt"), sNull, sNull, sNull, -1); testPassed(QC_T("s6a"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("s6a"));
	}

	String s7 = QC_T("http://www.baidu.com:80/dir1/dir2/search?xml");
	String s7a = QC_T("../../images/image1.gif");
	String s7b = QC_T("../../../images/image1.gif");
	try
	{
		testURL(s7a, s7, QC_T("http"), QC_T("www.baidu.com"), QC_T("/images/image1.gif"), sNull, sNull, sNull, 80); testPassed(QC_T("s7a"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("s7a"));
	}
	try
	{
		testURL(s7b, s7, QC_T("http"), QC_T("www.baidu.com"), QC_T("/../images/image1.gif"), sNull, sNull, sNull, 80); testPassed(QC_T("s7b"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("s7b"));
	}

	String s8 = QC_T("file:tests/test.xml");
	String s8a = QC_T("../../test2.xml");
	try
	{
		testURL(s8a, s8, QC_T("file"), sNull, QC_T("../test2.xml"), sNull, sNull, sNull, -1); testPassed(QC_T("s8a"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("s8a"));
	}


	testMessage(QC_T("End of tests for URL"));
}


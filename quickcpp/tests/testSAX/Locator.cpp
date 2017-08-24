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
#include "QcXml/sax/Locator.h"
#include "QcXml/sax/DefaultHandler.h"
#include "QcCore/net/UnknownHostException.h"

using namespace qc::sax; 
using namespace qc::net; 

//String s1 = QC_T("http://a.b.c/x.xml");
String s1 = QC_T("http://www.w3schools.com/xml/note.xml");

class MyContentHandler : public DefaultHandler
{
	virtual void setDocumentLocator(Locator* pLocator)
	{
		m_rpLocator = pLocator;
	}
	virtual void startDocument()
	{
		try
		{
			if(m_rpLocator->getLineNumber() == -1) {testPassed(QC_T("test Line"));} else {testFailed(QC_T("test Line"));}
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("test Line"));
		}
		try
		{
			if(m_rpLocator->getColumnNumber() == -1) {testPassed(QC_T("test Column"));} else {testFailed(QC_T("test Column"));}
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("test Column"));
		}
		try
		{
			if(m_rpLocator->getSystemId() == s1) {testPassed(QC_T("test systemId"));} else {testFailed(QC_T("test systemId"));}
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("test systemId"));
		}
	}
	AutoPtr<Locator> m_rpLocator;
};


void Locator_Tests()
{
	testMessage(QC_T("Starting tests for Locator"));


	AutoPtr<XMLReader> rpReader = XMLReaderFactory::CreateXMLReader();
	try
	{
		if(rpReader) {testPassed(QC_T("rpReader"));} else {testFailed(QC_T("rpReader"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("rpReader"));
	}
	AutoPtr<MyContentHandler> rpMyHandler = new MyContentHandler;

	rpReader->setContentHandler(rpMyHandler.get());
	try
	{
		if(rpReader->getContentHandler() == rpMyHandler) {testPassed(QC_T("check content handler"));} else {testFailed(QC_T("check content handler"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("check content handler"));
	}

	try
	{
		rpReader->parse(s1);
	}
	catch(UnknownHostException& e)
	{
		goodCatch(QC_T("unknown host"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("unknown host"));
	}

	testMessage(QC_T("End of tests for Locator"));
}


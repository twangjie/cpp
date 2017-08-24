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
#include "QcXml/sax/SAXFeatures.h"
#include "QcXml/sax/DefaultHandler.h"
#include "QcXml/sax/SAXNotSupportedException.h"
#include "QcXml/sax/SAXParseException.h"
#include "QcCore/net/UnknownHostException.h"
#include "QcCore/io/StringReader.h"

using namespace qc::sax; 
using namespace qc::net; 
using namespace qc::io; 

size_t NamespaceAttributes = 0;

class MyXMLReaderHandler : public DefaultHandler
{
public:

	virtual void setDocumentLocator(Locator* pLocator)
	{
		m_rpLocator = pLocator;
	}
	virtual void startDocument()
	{
	}
	virtual void startElement(const String& ns, const String& local, const String& qname, const Attributes& atts)
	{
		try
		{
			if(ns == QC_T("http://www.google.com/ns")) {testPassed(QC_T("check ns uri"));} else {testFailed(QC_T("check ns uri"));}
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("check ns uri"));
		}
		try
		{
			if(local == QC_T("test")) {testPassed(QC_T("check local"));} else {testFailed(QC_T("check local"));}
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("check local"));
		}
		try
		{
			if(qname == QC_T("qc:test")) {testPassed(QC_T("check qname"));} else {testFailed(QC_T("check qname"));}
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("check qname"));
		}
		try
		{
			if(atts.getLength() == (1 + NamespaceAttributes)) {testPassed(QC_T("check atts"));} else {testFailed(QC_T("check atts"));}
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("check atts"));
		}
		try
		{
			if(atts.getType(0) == QC_T("CDATA")) {testPassed(QC_T("check att type"));} else {testFailed(QC_T("check att type"));}
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("check att type"));
		}
	}
	virtual void error(const SAXParseException& e)
	{
		throw e;
	}	

	AutoPtr<Locator> m_rpLocator;
	bool m_bIsStandalone;
};


void XMLReader_Tests()
{
	testMessage(QC_T("Starting tests for XMLReader"));

	const String dtd   = QC_T("<!DOCTYPE test [")
		QC_T("<!ELEMENT test (#PCDATA)*>")
		QC_T("<!ATTLIST test attr1 CDATA #REQUIRED attr2 (yes|no) 'yes'>")
		QC_T("]>");
	const String doc   = QC_T("<qc:test xmlns:qc='http://www.google.com/ns' attr1='attr1'/>");
	const String valid = dtd + doc;

	AutoPtr<XMLReader> rpReader = XMLReaderFactory::CreateXMLReader();
	AutoPtr<MyXMLReaderHandler> rpHandler = new MyXMLReaderHandler();

	//
	// Set the XMLreader into a validating parser
	//
	try
	{
		rpReader->setFeature(SAXFeatures::validation, true); testPassed(QC_T("set validation"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("set validation"));
	}

	//
	// Register a content handler, which tests correct namespace processing
	//
	rpReader->setContentHandler(rpHandler.get());
	{
		AutoPtr<StringReader> rpSR = new StringReader(dtd);
		AutoPtr<InputSource> rpIS = new InputSource(rpSR.get());
		try
		{
			rpReader->parse(rpIS.get());
			testFailed(QC_T("parse notwf"));
		}
		catch(SAXParseException& e)
		{
			goodCatch(QC_T("parse notwf"), e.toString());
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("parse notwf"));
		}
	}
	{
		AutoPtr<StringReader> rpSR = new StringReader(doc);
		AutoPtr<InputSource> rpIS = new InputSource(rpSR.get());
		try
		{
			rpReader->parse(rpIS.get()); testPassed(QC_T("parse invalid"));
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("parse invalid"));
		}
	}
	{
		AutoPtr<StringReader> rpSR = new StringReader(valid);
		AutoPtr<InputSource> rpIS = new InputSource(rpSR.get());
		try
		{
			rpReader->parse(rpIS.get()); testPassed(QC_T("parse valid"));
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("parse valid"));
		}
	}

	//
	// Now let's try that again, this time with namespace prefix reporting switched on
	//
	try
	{
		rpReader->setFeature(SAXFeatures::namespace_prefixes, true); testPassed(QC_T("switch on prefix reporting"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("switch on prefix reporting"));
	}
	NamespaceAttributes = 1;
	{
		AutoPtr<StringReader> rpSR = new StringReader(valid);
		AutoPtr<InputSource> rpIS = new InputSource(rpSR.get());
		try
		{
			rpReader->parse(rpIS.get()); testPassed(QC_T("parse ns-prefix"));
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("parse ns-prefix"));
		}
	}

	//
	// Register an error handler, which throws a SAXParseException on validity errors
	//
	rpReader->setErrorHandler(rpHandler.get());
	{
		AutoPtr<StringReader> rpSR = new StringReader(doc);
		AutoPtr<InputSource> rpIS = new InputSource(rpSR.get());
		try
		{
			rpReader->parse(rpIS.get());
			testFailed(QC_T("parse invalid 2"));
		}
		catch(SAXParseException& e)
		{
			goodCatch(QC_T("parse invalid 2"), e.toString());
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("parse invalid 2"));
		}
	}


	testMessage(QC_T("End of tests for XMLReader"));
}


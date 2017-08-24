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
#include "QcCore/net/UnknownHostException.h"
#include "QcCore/io/StringReader.h"

using namespace qc::sax; 
using namespace qc::net; 
using namespace qc::io; 

class MyFeatureContentHandler : public DefaultHandler
{
public:
	MyFeatureContentHandler(XMLReader& reader)
		: m_reader(reader)
	{}

	virtual void setDocumentLocator(Locator* pLocator)
	{
		m_rpLocator = pLocator;
	}
	virtual void startDocument()
	{
		m_bIsStandalone = false;
		try
		{
			m_bIsStandalone = m_reader.getFeature(SAXFeatures::is_standalone);
			testFailed(QC_T("get is-standalone"));
		}
		catch(SAXNotSupportedException& e)
		{
			goodCatch(QC_T("get is-standalone"), e.toString());
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("get is-standalone"));
		}
	}
	virtual void startElement(const String&, const String&, const String&, const Attributes&)
	{
		try
		{
			m_bIsStandalone = m_reader.getFeature(SAXFeatures::is_standalone); testPassed(QC_T("get is-standalone"));
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("get is-standalone"));
		}

		//
		// try and set some read-only features
		//
		try
		{
			m_reader.setFeature(SAXFeatures::validation, true);
			testFailed(QC_T("read-only 1"));
		}
		catch(SAXNotSupportedException& e)
		{
			goodCatch(QC_T("read-only 1"), e.toString());
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("read-only 1"));
		}
		try
		{
			m_reader.setFeature(SAXFeatures::namespaces, true);
			testFailed(QC_T("read-only 2"));
		}
		catch(SAXNotSupportedException& e)
		{
			goodCatch(QC_T("read-only 2"), e.toString());
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("read-only 2"));
		}
		try
		{
			m_reader.setFeature(SAXFeatures::external_parameter_entities, true);
			testFailed(QC_T("read-only 3"));
		}
		catch(SAXNotSupportedException& e)
		{
			goodCatch(QC_T("read-only 3"), e.toString());
		}
		catch(Exception& e)
		{
			uncaughtException(e.toString(), QC_T("read-only 3"));
		}
	}

	AutoPtr<Locator> m_rpLocator;
	XMLReader& m_reader;
	bool m_bIsStandalone;
};


void Features_Tests()
{
	testMessage(QC_T("Starting tests for Features"));

	const String Standalone = QC_T("<?xml version='1.0' standalone='yes'?><test/>");

	AutoPtr<XMLReader> rpReader = XMLReaderFactory::CreateXMLReader();
	try
	{
		if(rpReader) {testPassed(QC_T("rpReader"));} else {testFailed(QC_T("rpReader"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("rpReader"));
	}
	AutoPtr<MyFeatureContentHandler> rpMyHandler = new MyFeatureContentHandler(*rpReader);

	rpReader->setContentHandler(rpMyHandler.get());

	//
	// test the default value of features
	//
	try
	{
		if(rpReader->getFeature(SAXFeatures::validation) == false) {testPassed(QC_T("default 1"));} else {testFailed(QC_T("default 1"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("default 1"));
	}
	try
	{
		if(rpReader->getFeature(SAXFeatures::external_general_entities) == true) {testPassed(QC_T("default 2"));} else {testFailed(QC_T("default 2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("default 2"));
	}
	try
	{
		if(rpReader->getFeature(SAXFeatures::external_parameter_entities) == true) {testPassed(QC_T("default 3"));} else {testFailed(QC_T("default 3"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("default 3"));
	}
	try
	{
		if(rpReader->getFeature(SAXFeatures::lexical_handler_parameter_entities) == false) {testPassed(QC_T("default 4"));} else {testFailed(QC_T("default 4"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("default 4"));
	}
	try
	{
		if(rpReader->getFeature(SAXFeatures::namespaces) == true) {testPassed(QC_T("default 5"));} else {testFailed(QC_T("default 5"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("default 5"));
	}
	try
	{
		if(rpReader->getFeature(SAXFeatures::namespace_prefixes) == false) {testPassed(QC_T("default 6"));} else {testFailed(QC_T("default 6"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("default 6"));
	}
	try
	{
		if(rpReader->getFeature(SAXFeatures::resolve_dtd_uris) == true) {testPassed(QC_T("default 7"));} else {testFailed(QC_T("default 7"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("default 7"));
	}

	AutoPtr<StringReader> rpSR = new StringReader(Standalone);
	AutoPtr<InputSource> rpIS = new InputSource(rpSR.get());
	try
	{
		rpReader->parse(rpIS.get()); testPassed(QC_T("parse standalone"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("parse standalone"));
	}
	try
	{
		if(rpMyHandler->m_bIsStandalone) {testPassed(QC_T("is-standalone"));} else {testFailed(QC_T("is-standalone"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("is-standalone"));
	}


	testMessage(QC_T("End of tests for Features"));
}


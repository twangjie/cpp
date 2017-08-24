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


void File_Tests();
void FileInputStream_Tests();
void FileOutputStream_Tests();
void InputStreamReader_Tests();
void OutputStreamWriter_Tests();
void Stream_Tests();
void BufferedInputStream_Tests();
void BufferedReader_Tests();


#include "QcCore/base/System.h"
#include "QcCore/auxil/MemCheckSystemMonitor.h"
#include "QcCore/auxil/CommandLineParser.h"
#include "QcCore/auxil/BasicOption.h"
#include "QcCore/auxil/TraceHelper.h"

using namespace qc;
using namespace qc::auxil;

int fail(0), pass(0);
bool bShowTests(false), bShowPasses(false), bShowFailures(true);
AttributeListParser TestAttributes;

String getTestAttribute(const String& name)
{
	return TestAttributes.getAttributeValue(name);
}

void testMessage(const String& msg)
{
	if (bShowTests) Console::Out()->println(msg);
}

void testFailed(const String& test)
{
	fail++;
	if(bShowFailures) Console::Out()->println(String(QC_T("Test failed for ")) + test);
}

void uncaughtException(const String& e, const String& test)
{
	fail++;
	if(bShowFailures) Console::Out()->println(String(QC_T("Unexpected exception: ")) + e + String(QC_T(" for ")) + test);
}

void testPassed(const String& test)
{
	pass++;
	if(bShowPasses) Console::Out()->println(String(QC_T("Test passed for ")) + test);
}

void goodCatch(const String& test, const String& eMsg)
{
	if(bShowPasses) Console::Out()->println(String(QC_T("Caught expected exception: ")) + eMsg);
	testPassed(test);
}

int main(int argc, char* argv[])
{
	MemCheckSystemMonitor _monitor;

	CommandLineParser cmdlineParser;

	BasicOption optVerbose(QC_T("verbose"), 'v', BasicOption::none);
	BasicOption optTrace(QC_T("trace"), 't', BasicOption::mandatory);
	BasicOption optParam(QC_T("param"), 'p', BasicOption::mandatory);

	cmdlineParser.addOption(&optVerbose);
	cmdlineParser.addOption(&optTrace);
	cmdlineParser.addOption(&optParam);

	try
	{
		cmdlineParser.parse(argc, argv);
	}
	catch (CommandLineException& e)
	{
		Console::Err()->println(cmdlineParser.getProgramName() + String(QC_T(": ")) + e.getMessage());
		Console::Err()->println(String(QC_T("Try ")) + cmdlineParser.getProgramName() + QC_T(" --help"));
		return (1);
	}

	//
	// Use a TraceHelper to set up a standard tracing output
	//
	if(optTrace.isPresent())
	{
		Tracer::SetTracer(new TraceHelper(cmdlineParser.getProgramName(), optTrace.getArgument()));
	}

	bShowPasses = optVerbose.isPresent();
	bShowTests = optVerbose.isPresent();
	if(optParam.isPresent())
	{
		TestAttributes.parseString(optParam.getArgument());
	}

	try
	{
		File_Tests();
		FileInputStream_Tests();
		FileOutputStream_Tests();
		InputStreamReader_Tests();
		OutputStreamWriter_Tests();
		Stream_Tests();
		BufferedInputStream_Tests();
		BufferedReader_Tests();
	}
	catch(Exception& e)
	{
		Console::cout() << cmdlineParser.getProgramName() << QC_T(": unhandled exception: ") << e.toString() << endl;
	}
	catch(...)
	{
		Console::cout() << cmdlineParser.getProgramName() << QC_T(": unhandled system exception") << endl;
	}
	Console::Out()->print(cmdlineParser.getProgramName() + QC_T(": tests passed: "));
	Console::Out()->print(pass);
	Console::Out()->print(QC_T(", tests failed: "));
	Console::Out()->println(fail);

	return (fail != 0);
}



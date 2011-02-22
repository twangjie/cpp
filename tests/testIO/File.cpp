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
#include "QcCore/base/IllegalArgumentException.h"
#include "QcCore/io/File.h"
#include "QcCore/io/FileOutputStream.h"
#include "QcCore/io/FileInputStream.h"
#include "QcCore/io/OutputStreamWriter.h"
#include "QcCore/io/InputStreamReader.h"
#include "QcCore/io/FileNotFoundException.h"
#include "QcCore/io/ExistingFileException.h"
#include "QcCore/io/IOException.h"

using namespace qc::io;

void doFileTests(const File& f, bool bDir)
{
	try
	{
		if(f.exists()==true) {testPassed(QC_T("exists1"));} else {testFailed(QC_T("exists1"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("exists1"));
	}
	try
	{
		if(f.isAbsolute()==false) {testPassed(QC_T("isAbsolute"));} else {testFailed(QC_T("isAbsolute"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("isAbsolute"));
	}
	try
	{
		if(f.isFile()==!bDir) {testPassed(QC_T("isFile"));} else {testFailed(QC_T("isFile"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("isFile"));
	}
	try
	{
		if(f.isDirectory()==bDir) {testPassed(QC_T("isDirectory"));} else {testFailed(QC_T("isDirectory"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("isDirectory"));
	}
	try
	{
		if(f.canRead()==true) {testPassed(QC_T("canRead"));} else {testFailed(QC_T("canRead"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("canRead"));
	}
	try
	{
		if(f.canWrite()==true) {testPassed(QC_T("canWrite"));} else {testFailed(QC_T("canWrite"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("canWrite"));
	}
	try
	{
		if(f==f.getPath()) {testPassed(QC_T("operator=="));} else {testFailed(QC_T("operator=="));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("operator=="));
	}

	File canon(f.getCanonicalPath());
	File abs(f.getAbsolutePath());

	try
	{
		if(canon.isAbsolute()==true) {testPassed(QC_T("getCanonicalPath"));} else {testFailed(QC_T("getCanonicalPath"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getCanonicalPath"));
	}
	try
	{
		if(abs.isAbsolute()==true) {testPassed(QC_T("getAbsolutePath"));} else {testFailed(QC_T("getAbsolutePath"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getAbsolutePath"));
	}
	try
	{
		if(canon.getPath() == abs.getPath()) {testPassed(QC_T("getAbsolutePath"));} else {testFailed(QC_T("getAbsolutePath"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getAbsolutePath"));
	}

	File copy(f);
	try
	{
		if(copy == f) {testPassed(QC_T("equality"));} else {testFailed(QC_T("equality"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("equality"));
	};
}


void File_Tests()
{
	testMessage(QC_T("Starting tests for File"));

	//
	// invalid filename
	//
	File badFile(QC_T(",./<>?#;-=s/\\\t\n~{}][===============================================================================================================================================================================================================================================================================================================================================>"));
	try
	{
		if(badFile.exists()==false) {testPassed(QC_T("exists_bad"));} else {testFailed(QC_T("exists_bad"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("exists_bad"));
	};
	try
	{
		if(badFile.isFile()==false) {testPassed(QC_T("isFile_bad"));} else {testFailed(QC_T("isFile_bad"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("isFile_bad"));
	};
	try
	{
		if(badFile.isDirectory()==false) {testPassed(QC_T("isDirectory_bad"));} else {testFailed(QC_T("isDirectory_bad"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("isDirectory_bad"));
	};
	try
	{
		if(badFile.canRead()==false) {testPassed(QC_T("canRead_bad"));} else {testFailed(QC_T("canRead_bad"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("canRead_bad"));
	};
	try
	{
		if(badFile.canWrite()==false) {testPassed(QC_T("canWrite_bad"));} else {testFailed(QC_T("canWrite_bad"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("canWrite_bad"));
	};

	File badFile2(QC_T(""));
	try
	{
		if(badFile2.exists()==false) {testPassed(QC_T("exists_bad2"));} else {testFailed(QC_T("exists_bad2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("exists_bad2"));
	};
	try
	{
		if(badFile2.canRead()==false) {testPassed(QC_T("canRead_bad2"));} else {testFailed(QC_T("canRead_bad2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("canRead_bad2"));
	};
	try
	{
		if(badFile2.canWrite()==false) {testPassed(QC_T("canWrite_bad2"));} else {testFailed(QC_T("canWrite_bad2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("canWrite_bad2"));
	};
	try
	{
		badFile2.setReadOnly(true);
		testFailed(QC_T("setReadOnly_bad2"));
	}
	catch(FileNotFoundException& e)
	{
		goodCatch(QC_T("setReadOnly_bad2"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setReadOnly_bad2"));
	};
	try
	{
		badFile2.renameTo(badFile);
		testFailed(QC_T("rename_bad2"));
	}
	catch(FileNotFoundException& e)
	{
		goodCatch(QC_T("rename_bad2"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("rename_bad2"));
	};

	//
	// create a file
	//
	File file1(QC_T("test_1"));
	File file1a(QC_T("./test_1"));
	File file1b(QC_T("./test_1b"));

	try
	{
		if(file1.getPath()==QC_T("test_1")) {testPassed(QC_T("getPath"));} else {testFailed(QC_T("getPath"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getPath"));
	}
	try
	{
		if(file1.getParent().empty()) {testPassed(QC_T("getParent"));} else {testFailed(QC_T("getParent"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getParent"));
	}
	try
	{
		if(file1==file1a) {testPassed(QC_T("operator=="));} else {testFailed(QC_T("operator=="));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("operator=="));
	}
	try
	{
		if(file1!=file1b) {testPassed(QC_T("operator!="));} else {testFailed(QC_T("operator!="));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("operator!="));
	}
	try
	{
		if(file1.getParentFile().getPath().empty()) {testPassed(QC_T("getParentFile"));} else {testFailed(QC_T("getParentFile"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("getParentFile"));
	}
	try
	{
		if(file1.exists()==false) {testPassed(QC_T("exists2"));} else {testFailed(QC_T("exists2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("exists2"));
	}
	try
	{
		if(file1.isFile()==false) {testPassed(QC_T("isFile"));} else {testFailed(QC_T("isFile"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("isFile"));
	};
	try
	{
		if(file1.isDirectory()==false) {testPassed(QC_T("isDirectory"));} else {testFailed(QC_T("isDirectory"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("isDirectory"));
	};
	try
	{
		if(file1.listDirectory().size()==0) {testPassed(QC_T("listDirectory"));} else {testFailed(QC_T("listDirectory"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("listDirectory"));
	}
	try
	{
		file1.length();
		testFailed(QC_T("length"));
	}
	catch(FileNotFoundException& e)
	{
		goodCatch(QC_T("length"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("length"));
	}
	try
	{
		file1.lastModified();
		testFailed(QC_T("lastModified"));
	}
	catch(FileNotFoundException& e)
	{
		goodCatch(QC_T("lastModified"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("lastModified"));
	}
	try
	{
		file1.createNewFile(); testPassed(QC_T("createNewFile"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("createNewFile"));
	}
	try
	{
		if(file1.exists()==true) {testPassed(QC_T("exists3"));} else {testFailed(QC_T("exists3"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("exists3"));
	}
	try
	{
		if(file1.listDirectory().size()==0) {testPassed(QC_T("listDirectory2"));} else {testFailed(QC_T("listDirectory2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("listDirectory2"));
	}
	try
	{
		file1.createNewFile();
		testFailed(QC_T("createNewFile2"));
	}
	catch(ExistingFileException& e)
	{
		goodCatch(QC_T("createNewFile2"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("createNewFile2"));
	}

	doFileTests(file1, false);

	// get the time from 5 minutes ago to use as a comparison
	DateTime now = DateTime::GetSystemTime();
	now.adjust(0, 0, -5, 0, 0);

	try
	{
		if(file1.lastModified()>=now) {testPassed(QC_T("lastModified"));} else {testFailed(QC_T("lastModified"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("lastModified"));
	}

	DateTime then(QC_T("01 Dec 2000 12:30:00 +0100"));
	try
	{
		if(then.isValid()) {testPassed(QC_T("DateTime"));} else {testFailed(QC_T("DateTime"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("DateTime"));
	};
	try
	{
		file1.setLastModified(then); testPassed(QC_T("setLastModified"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setLastModified"));
	};
	try
	{
		if(file1.lastModified()==then) {testPassed(QC_T("lastModified"));} else {testFailed(QC_T("lastModified"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("lastModified"));
	};
	try
	{
		file1.setLastModified(DateTime());
		testFailed(QC_T("setLastModified2"));
	}
	catch(IllegalArgumentException& e)
	{
		goodCatch(QC_T("setLastModified2"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setLastModified2"));
	};

	//
	// alter read-only status and test its effects
	//
	try
	{
		file1.setReadOnly(true); testPassed(QC_T("setReadOnly"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setReadOnly"));
	};
	try
	{
		if(file1.canWrite()==false) {testPassed(QC_T("canWrite"));} else {testFailed(QC_T("canWrite"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("canWrite"));
	};
	try
	{
		file1.setReadOnly(false); testPassed(QC_T("setReadOnly"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("setReadOnly"));
	};
	try
	{
		if(file1.canWrite()==true) {testPassed(QC_T("canWrite"));} else {testFailed(QC_T("canWrite"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("canWrite"));
	};

	//
	// Delete the files created earlier
	//
	try
	{
		file1.deleteFile(); testPassed(QC_T("deleteFile1"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("deleteFile1"));
	}
	try
	{
		file1.deleteFile();
		testFailed(QC_T("deleteFile2"));
	}
	catch(FileNotFoundException& e)
	{
		goodCatch(QC_T("deleteFile2"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("deleteFile2"));
	}

	//
	// Test directory existence/creation
	//
	File dir1(QC_T("sub1/sub2/sub3"));
	File dir4(QC_T("sub1/sub2/sub4"));
	File dir2(QC_T("sub1/sub2"));
	File dir3(QC_T("sub1"));
	try
	{
		if(dir1.exists() == false) {testPassed(QC_T("exists4"));} else {testFailed(QC_T("exists4"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("exists4"));
	}
	try
	{
		dir1.mkdir();
		testFailed(QC_T("mkdir"));
	}
	catch(FileNotFoundException& e)
	{
		goodCatch(QC_T("mkdir"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("mkdir"));
	}
	try
	{
		dir1.mkdirs(); testPassed(QC_T("mkdirs"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("mkdirs"));
	}
	try
	{
		dir1.mkdir();
		testFailed(QC_T("mkdir"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("mkdir"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("mkdir"));
	}
	try
	{
		if(dir2.exists()) {testPassed(QC_T("exists5"));} else {testFailed(QC_T("exists5"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("exists5"));
	}
	try
	{
		if(dir3.exists()) {testPassed(QC_T("exists6"));} else {testFailed(QC_T("exists6"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("exists6"));
	}
	try
	{
		if((dir3.listDirectory().size()==1) && (*dir3.listDirectory().begin()==QC_T("sub2"))) {testPassed(QC_T("listDirectory3"));} else {testFailed(QC_T("listDirectory3"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("listDirectory3"));
	}

	doFileTests(dir1, true);
	try
	{
		dir1.renameTo(dir4.getPath()); testPassed(QC_T("renameTo"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("renameTo"));
	}
	try
	{
		dir4.deleteFile(); testPassed(QC_T("deleteFile3"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("deleteFile3"));
	}
	try
	{
		if(dir4.exists()==false) {testPassed(QC_T("exists7"));} else {testFailed(QC_T("exists7"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("exists7"));
	}
	try
	{
		dir2.deleteFile(); testPassed(QC_T("deleteFile4"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("deleteFile4"));
	}
	try
	{
		if(dir2.exists()==false) {testPassed(QC_T("exists8"));} else {testFailed(QC_T("exists8"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("exists8"));
	}
	try
	{
		dir3.deleteFile(); testPassed(QC_T("deleteFile5"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("deleteFile5"));
	}
	try
	{
		if(dir3.exists()==false) {testPassed(QC_T("exists9"));} else {testFailed(QC_T("exists9"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("exists9"));
	}


	testMessage(QC_T("End of tests for File"));
}


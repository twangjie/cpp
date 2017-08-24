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
#include "QcCore/io/AtomicReadException.h"
#include "QcCore/io/BufferedInputStream.h"
#include "QcCore/io/BufferedOutputStream.h"
#include "QcCore/io/BufferedReader.h"
#include "QcCore/io/BufferedWriter.h"
#include "QcCore/io/Reader.h"
#include "QcCore/io/Writer.h"
#include "QcCore/io/File.h"
#include "QcCore/io/IOException.h"
#include "QcCore/io/FileOutputStream.h"
#include "QcCore/io/FileInputStream.h"
#include "QcCore/io/OutputStreamWriter.h"
#include "QcCore/io/InputStreamReader.h"
#include "QcCore/io/StringReader.h"
#include "QcCore/io/StringWriter.h"
#include "QcCore/io/FilterReader.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/base/IllegalCharacterException.h"
#include "QcCore/io/CharacterCodingException.h"
#include "QcCore/io/MalformedInputException.h"

using namespace qc::io;

//
// write a range of characters to a Writer
//
void writeCharRange(Writer* pWriter, unsigned long firstChar, unsigned long lastChar)
{
	AutoPtr<BufferedWriter> rpWriter = new BufferedWriter(pWriter);
	for (unsigned long i=firstChar; i<=lastChar; i++)
	{
		if(i<0xD800UL || i>0xDFFFUL)
		{
			Character ch(i);
			rpWriter->write(ch.data(), ch.length());
		}
	}
}

//
// Helper function to generate a file with a name, encoding and filled
// with characters from firstChar  - lastChar
//
void genFile(const File& file, const String& enc, unsigned long firstChar, unsigned long lastChar)
{
	AutoPtr<OutputStream> rpOS = new FileOutputStream(file);
	rpOS = new BufferedOutputStream(rpOS.get());
	AutoPtr<Writer> rpWriter = new OutputStreamWriter(rpOS.get(), enc, true);
	rpWriter = new BufferedWriter(rpWriter.get());
	try
	{
		writeCharRange(rpWriter.get(), firstChar, lastChar); testPassed(QC_T("write range"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("write range"));
	}
}

//
// Helper function to check that all methods return the correct value
// when  a character stream is at EOF
//
void testEOF(Reader* pReader)
{
	// Ensure all read methods agree we have reached the end of file
	CharType buffer[6];
	try
	{
		if(pReader->readAtomic(buffer, 6)==Reader::EndOfFile) {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	try
	{
		if(pReader->read(buffer, 6)==Reader::EndOfFile) {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	try
	{
		if(pReader->read()==Reader::EndOfFile) {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	try
	{
		if(pReader->skip(1)==0) {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	try
	{
		if(pReader->skipAtomic(1)==0) {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
}

qc::Character qc::Character::EndOfFileCharacter = qc::Character();
//
// Helper function to check the contents returned from a Reader
//
bool testReaderContents(Reader* pReader, unsigned long firstChar, unsigned long lastChar)
{
	bool bRet = true;
	unsigned long i = firstChar;
	while(true)
	{
		Character ch = pReader->readAtomic();
		if(ch == Character::EndOfFileCharacter)
		{
			testEOF(pReader);
			break;
		}
		if(ch.toUnicode() != i || i > lastChar)
		{
			String msg = QC_T("mismatch, got=");
			msg += NumUtils::ToString(ch.toUnicode());
			msg += QC_T(" expected=");
			msg += NumUtils::ToString(i);
			testMessage(msg);
			bRet = false;
			break;
		}
		i++;
		// skip the surrogate pair range
		if(i==0xD800) i=0xE000;
	}
	bRet = bRet && (i==lastChar+1);
	if(!bRet)
	{
		String msg = QC_T("testReaderContents failed, i=");
		msg += NumUtils::ToString(i);
		testMessage(msg);
	}
	return bRet;
}

//
// Helper function to read a file with a name, encoding and filled
// with characters from firstChar - lastChar
//
void testFileContents(const File& file, const String& enc, unsigned long firstChar, unsigned long lastChar)
{
	AutoPtr<InputStream> rpIS = new FileInputStream(file);
	AutoPtr<Reader> rpReader = new InputStreamReader(rpIS.get(), enc, true);
		// test again using buffering
	rpIS = new FileInputStream(file);
	rpReader = new BufferedReader(new InputStreamReader(rpIS.get(), enc, true));
	try
	{
		if(testReaderContents(rpReader.get(), firstChar, lastChar)) {testPassed(QC_T("testReaderContents"));} else {testFailed(QC_T("testReaderContents"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("testReaderContents"));
	}
}

//
// test that opererations are correctly disallowed on closed readers
//
void testClosedOps(Reader* pReader)
{
	// It should be okay to close a closed stream
	try
	{
		pReader->close(); testPassed(QC_T("closed1"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("closed1"));
	}
	try
	{
		pReader->close(); testPassed(QC_T("closed2"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("closed2"));
	}
	// Once closed, the following methods are not allowed: read, mark, reset
	CharType buffer[10];
	size_t bufLen=10;
	try
	{
		pReader->readAtomic();
		testFailed(QC_T("closed3"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("closed3"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("closed3"));
	}
	try
	{
		pReader->readAtomic(buffer, bufLen);
		testFailed(QC_T("closed4"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("closed4"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("closed4"));
	}
	try
	{
		pReader->read();
		testFailed(QC_T("closed5"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("closed5"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("closed5"));
	}
	try
	{
		pReader->read(buffer, bufLen);
		testFailed(QC_T("closed6"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("closed6"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("closed6"));
	}
	try
	{
		pReader->mark(4);
		testFailed(QC_T("closed7"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("closed7"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("closed7"));
	}
	try
	{
		pReader->reset();
		testFailed(QC_T("closed8"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("closed8"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("closed8"));
	}
	try
	{
		pReader->skip(1);
		testFailed(QC_T("closed9"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("closed9"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("closed9"));
	}
	try
	{
		pReader->skipAtomic(1);
		testFailed(QC_T("closedA"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("closedA"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("closedA"));
	}
}

//
// Perform a bunch of atomic operations on a Reader that
// is positioned on a high unicode character
//
void atomicReaderTests(Reader* pReader)
{
#ifndef QC_UCS4
	CharType buffer[6];
	// test rc=0 for readAtomic with too small buffer
	try
	{
		if(pReader->readAtomic(buffer, 1)==0) {testPassed(QC_T("readAtomic"));} else {testFailed(QC_T("readAtomic"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("readAtomic"));
	}
	try
	{
		if(pReader->skipAtomic(1)==1) {testPassed(QC_T("skipAtomic"));} else {testFailed(QC_T("skipAtomic"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("skipAtomic"));
	}
	try
	{
		if(pReader->readAtomic().toUnicode()==0x10001UL) {testPassed(QC_T("readAtomic2"));} else {testFailed(QC_T("readAtomic2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("readAtomic2"));
	}
	// skip to an invalid multi-char seq boundary
	try
	{
		if(pReader->skip(1)==1) {testPassed(QC_T("skip"));} else {testFailed(QC_T("skip"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("skip"));
	}
		try
	{
		pReader->skipAtomic(1);
		testFailed(QC_T("skipAtomic"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("skipAtomic"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("skipAtomic"));
	}
#endif //QC_UCS4
}

//
// Perform a bunch of atomic operations on a file with the given encoding
//
void atomicFileTests(const String& enc)
{
	File atomic(QC_T("atomic_file.txt"));
	genFile(atomic, enc, 0x10000, 0x10FFFF);
	try
	{
		if(atomic.isFile()) {testPassed(QC_T("atomic isFile"));} else {testFailed(QC_T("atomic isFile"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("atomic isFile"));
	}

	AutoPtr<InputStream> rpIS = new FileInputStream(atomic);
	AutoPtr<Reader> rpReader = new InputStreamReader(rpIS.get(), enc, true);
	atomicReaderTests(rpReader.get());
	try
	{
		rpReader->close(); testPassed(QC_T("close"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close"));
	}
	testClosedOps(rpReader.get());

	rpIS = new FileInputStream(atomic);
	rpReader = new BufferedReader(new InputStreamReader(rpIS.get(), enc, true));
	atomicReaderTests(rpReader.get());
	try
	{
		rpReader->close(); testPassed(QC_T("close"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close"));
	}
	testClosedOps(rpReader.get());

	try
	{
		atomic.deleteFile(); testPassed(QC_T("delete atomic"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("delete atomic"));
	}
	try
	{
		if(atomic.isFile()==false) {testPassed(QC_T("atomic isFile"));} else {testFailed(QC_T("atomic isFile"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("atomic isFile"));
	}
	try
	{
		if(atomic.exists()==false) {testPassed(QC_T("atomic exists"));} else {testFailed(QC_T("atomic exists"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("atomic exists"));
	}
}

//
// test a variety of Readers which have a controlled
// stream of "Hello World!"
//
void markMyWorld(Reader* pReader)
{
	try
	{
		if(pReader->markSupported()==true) {testPassed(QC_T("markSupported"));} else {testFailed(QC_T("markSupported"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("markSupported"));
	}
	try
	{
		pReader->mark(4); testPassed(QC_T("mark"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("mark"));
	}
	try
	{
		if(pReader->read()=='H') {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	try
	{
		if(pReader->read()=='e') {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	try
	{
		if(pReader->readAtomic().toUnicode()=='l') {testPassed(QC_T("readAtomic"));} else {testFailed(QC_T("readAtomic"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("readAtomic"));
	}
	try
	{
		pReader->reset(); testPassed(QC_T("reset7"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("reset7"));
	}
	try
	{
		if(pReader->read()=='H') {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	try
	{
		if(pReader->skip(11)==11) {testPassed(QC_T("skip"));} else {testFailed(QC_T("skip"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("skip"));
	}
	testEOF(pReader);
}

//
// test that opererations are correctly disallowed on closed writers
//
void testClosedOps(Writer* pWriter)
{
	// It should be okay to close a closed stream
	try
	{
		pWriter->close(); testPassed(QC_T("close"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close"));
	}
	try
	{
		pWriter->close(); testPassed(QC_T("close"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close"));
	}
	CharType buffer[10];
	size_t bufLen=10;
	try
	{
		pWriter->write(buffer, bufLen);
		testFailed(QC_T("write"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("write"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("write"));
	}
	try
	{
		pWriter->write(QC_T("hello"));
		testFailed(QC_T("write"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("write"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("write"));
	}
	try
	{
		pWriter->write('H');
		testFailed(QC_T("write"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("write"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("write"));
	}
	try
	{
		pWriter->flush();
		testFailed(QC_T("flush"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("flush"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("flush"));
	}
}


void Stream_Tests()
{
	testMessage(QC_T("Starting tests for Stream"));

	//
	// create three files using the common unicode encodings
	// utf-8, utf-16 and iso-8859-1
	//
	File utf8(QC_T("test_utf8"));
	File utf16(QC_T("test_utf16"));
	File iso88591(QC_T("test_iso8859_1"));

	genFile(utf8, QC_T("utf-8"), 1, 0x10FFFFUL);
	testFileContents(utf8, QC_T("utf-8"), 1, 0x10FFFFUL);

	genFile(utf16, QC_T("utf-16"), 1, 0x10FFFFUL);
	testFileContents(utf16, QC_T("utf-16"), 1, 0x10FFFFUL);

	genFile(iso88591, QC_T("iso-8859-1"), 1, 0x00FEUL);
	testFileContents(iso88591, QC_T("iso-8859-1"), 1, 0x00FEUL);

	//
	// test ability to write illegal surrogate value
	// with strict turned on and then off
	//
	File badutf16(QC_T("badutf16.txt"));
	AutoPtr<OutputStreamWriter> badWriter = new OutputStreamWriter(new FileOutputStream(badutf16), QC_T("utf-16"), true);
#ifdef QC_UTF16
	try
	{
		Character bad(0xD800);
		testFailed(QC_T("illegal surrogate"));
	}
	catch(IllegalCharacterException& e)
	{
		goodCatch(QC_T("illegal surrogate"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("illegal surrogate"));
	}
#else
	Character bad(0xD800);
	try
	{
		badWriter->write(bad.data(), bad.length());
		testFailed(QC_T("surrogate"));
	}
	catch(CharacterCodingException& e)
	{
		goodCatch(QC_T("surrogate"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("surrogate"));
	}
	try
	{
		badWriter->getEncoder()->setUnmappableCharAction(CodeConverter::replace); testPassed(QC_T("surrogate2"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("surrogate2"));
	}
	try
	{
		; testPassed(QC_T("surrogate3 badWriter->write(bad.data(), bad.length())"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("surrogate3 badWriter->write(bad.data(), bad.length())"));
	}
#endif
	try
	{
		badWriter->close(); testPassed(QC_T("close"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close"));
	}
	try
	{
		badutf16.deleteFile(); testPassed(QC_T("deleteFile"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("deleteFile"));
	}

	//
	// test ability to handle illegal utf-8 sequences
	//
	File badutf8(QC_T("badutf8.txt"));
	AutoPtr<OutputStream> rpBadUTF8OStream  = new FileOutputStream(badutf8);
	Byte bad_text_1[] = {0x80}; // illegal start char
	Byte bad_text_2[] = {0xf0, 0x80, 0x80, 0x80}; // illegal null char
	Byte bad_text_3[] = {0xf0, 0x81, 0x81}; // premature eof

	try
	{
		rpBadUTF8OStream->write(bad_text_1, sizeof(bad_text_1)); testPassed(QC_T("write 1"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("write 1"));
	}
	try
	{
		rpBadUTF8OStream->write(bad_text_2, sizeof(bad_text_2)); testPassed(QC_T("write 2"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("write 2"));
	}
	try
	{
		rpBadUTF8OStream->write(bad_text_3, sizeof(bad_text_3)); testPassed(QC_T("write 3"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("write 3"));
	}
	try
	{
		rpBadUTF8OStream->close(); testPassed(QC_T("close"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close"));
	}
	//
	// Now, let's see what happens when we read it!
	// We need strict checking switched on to start with
	//
	AutoPtr<InputStreamReader> rpBadUtf8Reader = new InputStreamReader(new FileInputStream(badutf8), QC_T("utf-8"), true);
	
	// If the exception is correctly thrown, then the Reader's position should not have changed
	// which means there are three tokens left to read
	try
	{
		rpBadUtf8Reader->readAtomic();
		testFailed(QC_T("illegal start"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("illegal start"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("illegal start"));
	}
	try
	{
		rpBadUtf8Reader->getDecoder()->setInvalidCharAction(CodeConverter::replace); testPassed(QC_T("change converter"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("change converter"));
	}
	try
	{
		if(rpBadUtf8Reader->readAtomic().toUnicode() == 0xFFFD) {testPassed(QC_T("test repl1"));} else {testFailed(QC_T("test repl1"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("test repl1"));
	}
	try
	{
		if(rpBadUtf8Reader->readAtomic().toUnicode() == 0xFFFD) {testPassed(QC_T("test repl2"));} else {testFailed(QC_T("test repl2"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("test repl2"));
	}
	try
	{
		if(rpBadUtf8Reader->readAtomic().toUnicode() == 0xFFFD) {testPassed(QC_T("test repl3"));} else {testFailed(QC_T("test repl3"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("test repl3"));
	}
	try
	{
		if(rpBadUtf8Reader->readAtomic() == Character::EndOfFileCharacter) {testPassed(QC_T("test eof"));} else {testFailed(QC_T("test eof"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("test eof"));
	}

	try
	{
		rpBadUtf8Reader->close(); testPassed(QC_T("close"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close"));
	}
	testClosedOps(rpBadUtf8Reader.get());
	try
	{
		badutf8.deleteFile(); testPassed(QC_T("deleteFile"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("deleteFile"));
	}

	//
	// Test mark/reset operations
	//

	AutoPtr<FileInputStream> rpIS1 = new FileInputStream(utf8);
	try
	{
		if(rpIS1->markSupported()==false) {testPassed(QC_T("markSupported"));} else {testFailed(QC_T("markSupported"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("markSupported"));
	}
	try
	{
		rpIS1->mark(1);
		testFailed(QC_T("mark"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("mark"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("mark"));
	}
	try
	{
		rpIS1->reset();
		testFailed(QC_T("reset"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("reset"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("reset"));
	}

	AutoPtr<BufferedInputStream> rpBufIS = new BufferedInputStream(rpIS1.get(), 200);
	try
	{
		if(rpBufIS->markSupported()==true) {testPassed(QC_T("markSupported"));} else {testFailed(QC_T("markSupported"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("markSupported"));
	}
	try
	{
		rpBufIS->mark(100); testPassed(QC_T("mark"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("mark"));
	}
	try
	{
		if(rpBufIS->read()==1) {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	try
	{
		if(rpBufIS->read()==2) {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	//
	// Reset the position and then test the first two bytes again
	//
	try
	{
		rpBufIS->reset(); testPassed(QC_T("reset2"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("reset2"));
	}
	try
	{
		if(rpBufIS->read()==1) {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	try
	{
		if(rpBufIS->read()==2) {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	//
	// Mark again, this time from a different position, and with a buffer squeezing readLimit
	//
	try
	{
		rpBufIS->mark(200); testPassed(QC_T("mark"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("mark"));
	}
	try
	{
		if(rpBufIS->read()==3) {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	try
	{
		rpBufIS->reset(); testPassed(QC_T("reset3"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("reset3"));
	}
	try
	{
		if(rpBufIS->read()==3) {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	//
	// Mark again, this time from a different position, and with a buffer blowing readLimit
	//
	try
	{
		rpBufIS->mark(1000); testPassed(QC_T("mark"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("mark"));
	}
	try
	{
		if(rpBufIS->read()==4) {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	try
	{
		rpBufIS->reset(); testPassed(QC_T("reset4"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("reset4"));
	}
	int startChar=0;
	try
	{
		if((startChar = rpBufIS->read())==4) {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	try
	{
		rpBufIS->reset(); testPassed(QC_T("reset5"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("reset5"));
	}

	// 
	// By default, Readers do not support mark/reset
	// 
	AutoPtr<InputStreamReader> rpRdr = new InputStreamReader(rpBufIS.get(), QC_T("utf-8"));
	try
	{
		if(rpRdr->markSupported()==false) {testPassed(QC_T("markSupported"));} else {testFailed(QC_T("markSupported"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("markSupported"));
	}
	try
	{
		rpRdr->mark(1);
		testFailed(QC_T("mark"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("mark"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("mark"));
	}
	try
	{
		rpRdr->reset();
		testFailed(QC_T("reset4"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("reset4"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("reset4"));
	}

	//
	// but BufferedReaders do support mark.reset
	// use a small buffer size so that reset() fails when mark exceeded
	//
	AutoPtr<BufferedReader> rpBufRdr = new BufferedReader(rpRdr.get(), 100);
	try
	{
		if(rpBufRdr->markSupported()==true) {testPassed(QC_T("markSupported"));} else {testFailed(QC_T("markSupported"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("markSupported"));
	}
	try
	{
		rpBufRdr->mark(4); testPassed(QC_T("mark"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("mark"));
	}
	try
	{
		if(rpBufRdr->read()==startChar) {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	try
	{
		rpBufRdr->reset(); testPassed(QC_T("reset5"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("reset5"));
	}
	//
	// Bust the buffer size limit
	//
	try
	{
		rpBufRdr->mark(200); testPassed(QC_T("reset5"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("reset5"));
	}
	try
	{
		if(rpBufRdr->read()==startChar) {testPassed(QC_T("read"));} else {testFailed(QC_T("read"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	try
	{
		if(rpBufRdr->readAtomic().toUnicode()==(UCS4Char)startChar+1) {testPassed(QC_T("readAtomic"));} else {testFailed(QC_T("readAtomic"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("readAtomic"));
	}
	try
	{
		if(rpBufRdr->skipAtomic(100)==100) {testPassed(QC_T("skipAtomic"));} else {testFailed(QC_T("skipAtomic"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("skipAtomic"));
	}
	try
	{
		if(rpBufRdr->readAtomic().toUnicode()==(UCS4Char)startChar+102) {testPassed(QC_T("readAtomic"));} else {testFailed(QC_T("readAtomic"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("readAtomic"));
	}
	//
	// Reset the position and then test that the 128th character is okay
	//
	try
	{
		rpBufRdr->reset(); testPassed(QC_T("reset5"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("reset5"));
	}
	const size_t charsToSkip = (127-startChar+1);
	try
	{
		if(rpBufRdr->skip(charsToSkip)==charsToSkip) {testPassed(QC_T("skip"));} else {testFailed(QC_T("skip"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("skip"));
	}
	try
	{
		if(rpBufRdr->readAtomic().toUnicode()==128) {testPassed(QC_T("readAtomic"));} else {testFailed(QC_T("readAtomic"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("readAtomic"));
	}

#ifdef QC_UTF8
	// throw the reader into an invalid character position
	try
	{
		rpBufRdr->read(); testPassed(QC_T("read"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("read"));
	}
	// We are in an invalid position, so readAtomic should throw an exception
	try
	{
		rpBufRdr->readAtomic();
		testFailed(QC_T("readAtomic"));
	}
	catch(AtomicReadException& e)
	{
		goodCatch(QC_T("readAtomic"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("readAtomic"));
	}
#endif //QC_UTF8

	// Ensure we exceed the readLimit
	try
	{
		if(rpBufRdr->skip(200) == 200) {testPassed(QC_T("skip"));} else {testFailed(QC_T("skip"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("skip"));
	}
	// We have exceeded the mark position, so expect an IOException
	try
	{
		rpBufRdr->reset();
		testFailed(QC_T("reset6"));
	}
	catch(IOException& e)
	{
		goodCatch(QC_T("reset6"), e.toString());
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("reset6"));
	}
	//
	// we may not be able to delete the file while it's still open, so
	// explicity close it
	//
	try
	{
		rpBufRdr->close(); testPassed(QC_T("close"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close"));
	}
	testClosedOps(rpBufRdr.get());

	//
	// Delete the data files created earlier
	//
	try
	{
		utf8.deleteFile(); testPassed(QC_T("deleteFile"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("deleteFile"));
	}
	try
	{
		utf16.deleteFile(); testPassed(QC_T("deleteFile"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("deleteFile"));
	}
	try
	{
		iso88591.deleteFile(); testPassed(QC_T("deleteFile"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("deleteFile"));
	}

	//
	// Test atomic operations
	//
	atomicFileTests(QC_T("utf-8"));
	atomicFileTests(QC_T("utf-16"));

	AutoPtr<StringWriter> rpStrWrt = new StringWriter;
	writeCharRange(rpStrWrt.get(), 0x10000, 0x10100);

	String s = rpStrWrt->toString();
	AutoPtr<Reader> rpReader = new StringReader(s);
	try
	{
		if(testReaderContents(rpReader.get(), 0x10000, 0x10100)) {testPassed(QC_T("contents"));} else {testFailed(QC_T("contents"));}
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("contents"));
	}
	rpReader = new StringReader(s);
	atomicReaderTests(rpReader.get());
	try
	{
		rpReader->close(); testPassed(QC_T("close"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close"));
	}
	testClosedOps(rpReader.get());

	//
	// StringReaders do support mark/reset without the need for a BufferedReader
	//
	String hello = QC_T("Hello World!");
	AutoPtr<Reader> rpStrRdr = new StringReader(hello);
	markMyWorld(rpStrRdr.get());
	try
	{
		rpStrRdr->close(); testPassed(QC_T("close"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close"));
	}
	testClosedOps(rpStrRdr.get());

	//
	// FilterReaders should support the same ops as the Reader they are imbued with
	//
	AutoPtr<Reader> rpFilterRdr = new FilterReader(new StringReader(hello));
	markMyWorld(rpFilterRdr.get());
	try
	{
		rpFilterRdr->close(); testPassed(QC_T("close"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close"));
	}
	testClosedOps(rpFilterRdr.get());

	//
	// create a file containing Hello World
	//
	File fHello(QC_T("hello.txt"));
	AutoPtr<FileOutputStream> rpOS = new FileOutputStream(fHello);
	AutoPtr<OutputStreamWriter> rpWriter = new OutputStreamWriter(rpOS.get(), QC_T("UTF-16"));
	try
	{
		rpWriter->write(hello); testPassed(QC_T("write"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("write"));
	};
	try
	{
		rpWriter->close(); testPassed(QC_T("close"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close"));
	};
	testClosedOps(rpWriter.get());

	AutoPtr<FileInputStream> rpISHello = new FileInputStream(fHello);
	AutoPtr<BufferedReader> rpHelloRdr = new BufferedReader(new InputStreamReader(rpISHello.get(), QC_T("UTF-16")));
	markMyWorld(rpHelloRdr.get());
	try
	{
		rpHelloRdr->close(); testPassed(QC_T("close"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("close"));
	}
	testClosedOps(rpHelloRdr.get());
	try
	{
		fHello.deleteFile(); testPassed(QC_T("deleteFile"));
	}
	catch(Exception& e)
	{
		uncaughtException(e.toString(), QC_T("deleteFile"));
	}
	

	testMessage(QC_T("End of tests for Stream"));
}


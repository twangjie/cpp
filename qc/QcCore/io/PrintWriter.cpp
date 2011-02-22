/*
 * This file is part of QuickCPP.
 * (c) Copyright 2011 Jie Wang(twj31470952@gmail.com)
 *
 * QuickCPP is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * QuickCPP is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with QuickCPP.  If not, see <http://www.gnu.org/licenses/>.
 */
 
//==============================================================================
//
// $Revision$
// $Date$
//
//==============================================================================
//
// Class: PrintWriter
//
/**
	@class qc::io::PrintWriter
	
	@brief A Writer with the capability to do basic output formatting
	for a variety of built-in types, Characters and Strings.

    Unlike its namesake in the Java API, this class <em>does</em> throw
	exceptions whenever an I/O error occurs.
*/
//==============================================================================

#include "PrintWriter.h"
#include "IOException.h"
#include "OutputStream.h"
#include "OutputStreamWriter.h"

#include "QcCore/base/NullPointerException.h"
#include "QcCore/base/NumUtils.h"
#include "QcCore/base/System.h"

QC_IO_NAMESPACE_BEGIN

//==============================================================================
// PrintWriter::PrintWriter
/**
	Creates a PrintWriter for an existing byte output stream.  The system's
	default encoding will be used to translate from Unicode characters
	into bytes.
	
    @param pOut the underlying OutputStream
	@param bAutoFlush when @c true, the stream will be flushed after each println()
	       call
    @throws NullPointerException if @c pOut is null
*/
//==============================================================================
PrintWriter::PrintWriter(OutputStream* pOut, bool bAutoFlush) :
	Writer(new OutputStreamWriter(pOut)),
	m_rpWriter((Writer*)(SynchronizedObject*)getLock().get()), // the lock is the Writer just created
	m_bAutoFlush(bAutoFlush)
{
}

//==============================================================================
// PrintWriter::PrintWriter
/**
	Creates a PrintWriter using an existing Writer as the character sink.
	The encoding of the existing Writer will be used to translate from Unicode
	characters into bytes.
	
    @param pWriter the existing Writer to use
	@param bAutoFlush when @c true, the stream will be flushed after each println()
	       call
    @throws NullPointerException if @c pWriter is null

    @mt
    The existing Writer is used as the lock object for synchronized methods.
*/
//==============================================================================
PrintWriter::PrintWriter(Writer* pWriter, bool bAutoFlush) :
	Writer(pWriter ? (SynchronizedObject*)pWriter->getLock().get() : (SynchronizedObject*)0),
	m_rpWriter(pWriter),
	m_bAutoFlush(bAutoFlush)
{
	QC_DBG_ASSERT(this != pWriter);
	if(!pWriter) throw NullPointerException();
}

//==============================================================================
// PrintWriter::~PrintWriter
/**
	Flushes characters to the underlying output stream before destroying
	this PrintWriter.
*/
//==============================================================================
PrintWriter::~PrintWriter()
{
	if(m_rpWriter)
	{
		try
		{
			flush();
		}
		catch(Exception& /*e*/)
		{
		}
	}
}

//==============================================================================
// PrintWriter::close
//
//==============================================================================
void PrintWriter::close()
{
	if(m_rpWriter)
	{
		m_rpWriter->close();
		m_rpWriter.release();
	}
}

//==============================================================================
// PrintWriter::print
//
/**
	Prints the single Unicode character @c c.

    The character is translated into bytes using the encoding of the underlying
	Writer.
	@synchronized
*/
//==============================================================================
void PrintWriter::print(Character c)
{
	write(c.data(), c.length());
}

//==============================================================================
// PrintWriter::print
//
/**
	Prints a null-terminated array of ::CharType characters.

    The character array is translated into bytes using the encoding of the
	underlying Writer.
	@synchronized
*/
//==============================================================================
void PrintWriter::print(const CharType* pStr)
{
	write(pStr);
}

//==============================================================================
// PrintWriter::print
//
/**
	Prints a double-precision floating-point number.

    The number is first converted into a String using
	NumUtils::ToString() before being converted into bytes using the encoding
	of the underlying Writer.

    @sa NumUtils::ToString()
	@synchronized
*/
//==============================================================================
void PrintWriter::print(double d)
{
	write(NumUtils::ToString(d));
}

//==============================================================================
// PrintWriter::print
//
/**
	Prints a floating-point number.

    The number is first converted into a String using
	NumUtils::ToString() before being converted into bytes using the encoding
	of the underlying Writer.

    @sa NumUtils::ToString()
	@synchronized
*/
//==============================================================================
void PrintWriter::print(float f)
{
	write(NumUtils::ToString(f));
}

//==============================================================================
// PrintWriter::print
//
/**
	Prints a long integer.

    The long integer is first converted into a String using
	NumUtils::ToString() before being converted into bytes using the encoding
	of the underlying Writer.

    @sa NumUtils::ToString()
	@synchronized
*/
//==============================================================================
void PrintWriter::print(long l)
{
	write(NumUtils::ToString(l));
}

//==============================================================================
// PrintWriter::print
//
/**
	Prints an unsigned long integer.

    The unsigned integer is first converted into a String using
	NumUtils::ToString() before being converted into bytes using the encoding
	of the underlying Writer.

    @sa NumUtils::ToString()
	@synchronized
*/
//==============================================================================
void PrintWriter::print(unsigned long l)
{
	write(NumUtils::ToString(l));
}

//==============================================================================
// PrintWriter::print
//
/**
	Prints an integer.

    The integer is first converted into a String using
	NumUtils::ToString() before being converted into bytes using the encoding
	of the underlying Writer.

    @sa NumUtils::ToString()
	@synchronized
*/
//==============================================================================
void PrintWriter::print(int i)
{
	write(NumUtils::ToString(i));
}

//==============================================================================
// PrintWriter::print
//
/**
	Prints an unsigned integer.

    The unsigned integer is first converted into a String using
	NumUtils::ToString() before being converted into bytes using the encoding
	of the underlying Writer.

    @sa NumUtils::ToString()
	@synchronized
*/
//==============================================================================
void PrintWriter::print(unsigned int i)
{
	write(NumUtils::ToString(i));
}

//==============================================================================
// PrintWriter::print
//
/**
	Prints a String containing a sequence of ::CharType characters.

    The characters are converted into bytes using the encoding
	of the underlying Writer.
	@synchronized
*/
//==============================================================================
void PrintWriter::print(const String& s)
{
	write(s);
}

//==============================================================================
// PrintWriter::println
//
/**
	Prints a line separator string as returned from System::GetLineEnding().

	@sa System::GetLineEnding()
	@synchronized
*/
//==============================================================================
void PrintWriter::println()
{
	write(System::GetLineEnding());
	if(m_bAutoFlush)
	{
		flush();
	}
}

//==============================================================================
// PrintWriter::println
//
/**
   Prints a character and then terminates the line.
   @synchronized
*/
//==============================================================================
void PrintWriter::println(Character c)
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	print(c);
	println();
}

// 
//==============================================================================
// PrintWriter::println
//
/**
   Prints a null-terminated array of ::CharType characters and then terminates
   the line. 
   @synchronized
*/
//==============================================================================
void PrintWriter::println(const CharType* pStr)
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	print(pStr);
	println();
}

//==============================================================================
// PrintWriter::println
//
/**
   Prints a double-precision floating-point number and then terminates the line. 
   @synchronized
*/
//==============================================================================
void PrintWriter::println(double x)
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	print(x);
	println();
}

//==============================================================================
// PrintWriter::println
//
/**
   Prints a floating-point number and then terminates the line. 
   @synchronized
*/
//==============================================================================
void PrintWriter::println(float x)
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	print(x);
	println();
}

//==============================================================================
// PrintWriter::println
//
/**
   Prints a long integer and then terminates the line. 
   @synchronized
*/
//==============================================================================
void PrintWriter::println(long x)
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	print(x);
	println();
}

//==============================================================================
// PrintWriter::println
//
/**
   Prints an unsigned long integer and then terminates the line. 
   @synchronized
*/
//==============================================================================
void PrintWriter::println(unsigned long x)
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	print(x);
	println();
}

//==============================================================================
// PrintWriter::println
//
/**
   Prints an integer and then terminates the line. 
   @synchronized
*/
//==============================================================================
void PrintWriter::println(int x)
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	print(x);
	println();
}

//==============================================================================
// PrintWriter::println
//
/**
   Prints an unsigned integer and then terminates the line. 
   @synchronized
*/
//==============================================================================
void PrintWriter::println(unsigned int x)
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	print(x);
	println();
}

//==============================================================================
// PrintWriter::println
//
/**
   Prints a String and then terminates the line. 
   @synchronized
*/
//==============================================================================
void PrintWriter::println(const String& x)
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	print(x);
	println();
}

//==============================================================================
// PrintWriter::write
//
//==============================================================================
void PrintWriter::write(const CharType* pStr, size_t len)
{
	if(!m_rpWriter) throw IOException(QC_T("stream is closed"));

	m_rpWriter->write(pStr, len);
	m_rpWriter->flushBuffers();
}

//==============================================================================
// PrintWriter::flush
//
//==============================================================================
void PrintWriter::flush()
{
	if(!m_rpWriter) throw IOException(QC_T("stream is closed"));
	m_rpWriter->flush();
}

//==============================================================================
// PrintWriter::flushBuffers
//
//==============================================================================
void PrintWriter::flushBuffers()
{
	if(!m_rpWriter) throw IOException(QC_T("stream is closed"));
	m_rpWriter->flushBuffers();
}

#if defined(QC_HAVE_BUILTIN_BOOL)

	
//==============================================================================
// PrintWriter::print
//
/**
   Prints a boolean value as @c 'true' or @c 'false'.
   @synchronized
*/
//==============================================================================
void PrintWriter::print(bool b)
{
	write(b ? QC_T("true") : QC_T("false"));
}

//==============================================================================
// PrintWriter::println
//
/**
   Prints a boolean value and then terminates the line. 
   @synchronized
*/
//==============================================================================
void PrintWriter::println(bool x)
{
	QC_SYNCHRONIZED_PTR(m_rpLock)

	print(x);
	println();
}

#endif //QC_HAVE_BUILTIN_BOOL

//==============================================================================
// PrintWriter::operator
//
/**
   Output operator that takes a function pointer as its parameter.  
   This is required to enable the IO manipulator mechanism to work correctly.
*/
//==============================================================================
PrintWriter& PrintWriter::operator<<(PrintWriter& (*func)(PrintWriter&))
{
	return ((*func)(*this));
}

/** Output operator that prints a Unicode character. */
PrintWriter& PrintWriter::operator<<(Character c) {print(c); return *this;}

/** Output operator that prints a null-terminated string. */
PrintWriter& PrintWriter::operator<<(const CharType* pStr) {print(pStr); return *this;}

/** Output operator that prints a double-precision floating point value. */
PrintWriter& PrintWriter::operator<<(double d) {print(d); return *this;}

/** Output operator that prints a floating-point value. */
PrintWriter& PrintWriter::operator<<(float f) {print(f); return *this;}

/** Output operator that prints an unsigned long integer value; */
PrintWriter& PrintWriter::operator<<(unsigned long l) {print(l); return *this;}

/** Output operator that prints a long integer value. */
PrintWriter& PrintWriter::operator<<(long l) {print(l); return *this;}

/** Output operator that prints an unsigned integer value. */
PrintWriter& PrintWriter::operator<<(unsigned int i) {print(i); return *this;}

/** Output operator that prints an integer value. */
PrintWriter& PrintWriter::operator<<(int i) {print(i); return *this;}

/** Output operator that prints a String value. */
PrintWriter& PrintWriter::operator<<(const String& s) {print(s); return *this;}

#if defined QC_HAVE_BUILTIN_BOOL
    /** Output operator that prints a boolean value. */
	PrintWriter& PrintWriter::operator<<(bool x) {print(x); return *this;}
#endif //QC_HAVE_BUILTIN_BOOL

//==============================================================================
// endl
//
/**
   PrintWriter manipulator that enables @c endl to be used
   as an insertion operand.  For example
   @code
   Console::cout() << QC_T("Hello World!") << endl;
   @endcode
*/
//==============================================================================
PrintWriter& endl(PrintWriter& w)
{
	w.println();
	return w;
}

//==============================================================================
// flush
//
/**
   PrintWriter manipulator that enables @c flush to be used
   as an insertion operand.
*/
//==============================================================================
PrintWriter& flush(PrintWriter& w)
{
	w.flush();
	return w;
}

QC_IO_NAMESPACE_END

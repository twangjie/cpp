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

#ifndef QC_IO_PrintWriter_h
#define QC_IO_PrintWriter_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "Writer.h"

QC_IO_NAMESPACE_BEGIN

class OutputStream;

class QC_IO_PKG PrintWriter : public Writer
{
public:

	PrintWriter(OutputStream* pOut, bool bAutoFlush=false);
	PrintWriter(Writer* pWriter, bool bAutoFlush=false);

	virtual ~PrintWriter();

	virtual void close();
	virtual void flush();
	virtual void flushBuffers();

#if defined QC_HAVE_BUILTIN_BOOL
	void print(bool b);
#endif //QC_HAVE_BUILTIN_BOOL

	void print(Character c);
	void print(const CharType* pStr);
	void print(double d);
	void print(float f);
	void print(unsigned long l);
	void print(long l);
	void print(unsigned int i);
	void print(int i);
	void print(const String& s);

	void println();

#if defined QC_HAVE_BUILTIN_BOOL
	void println(bool x);
#endif //QC_HAVE_BUILTIN_BOOL

	void println(Character c);
	void println(const CharType* pStr);
	void println(double d);
	void println(float f);
	void println(int i);
	void println(unsigned int i);
	void println(long l);
	void println(unsigned long l);
	void println(const String& s);

#ifdef QC_USING_DECL_BROKEN
	virtual void write(CharType c)           {Writer::write(c);}
	virtual void write(const Character& ch)  {Writer::write(ch);}
	virtual void write(const String& str)    {Writer::write(str);}
#else
	using Writer::write; 	// unhide inherited write methods
#endif

	virtual void write(const CharType* pStr, size_t len);

	//
	// Standard IO Operators
	// 
	PrintWriter& operator<<(PrintWriter& (*func)(PrintWriter&));
	PrintWriter& operator<<(Character c);
	PrintWriter& operator<<(const CharType* pString);
	PrintWriter& operator<<(double d);
	PrintWriter& operator<<(float f);
	PrintWriter& operator<<(unsigned long l);
	PrintWriter& operator<<(long l);
	PrintWriter& operator<<(unsigned int i);
	PrintWriter& operator<<(int i);
	PrintWriter& operator<<(const String& s);
	#if defined QC_HAVE_BUILTIN_BOOL
		PrintWriter& operator<<(bool x);
	#endif //QC_HAVE_BUILTIN_BOOL

private: // Private helper methods
	void init();

private:
	AutoPtr<Writer> m_rpWriter;
	bool m_bAutoFlush;
};

// Standard Manipulators
QC_IO_PKG PrintWriter& endl(PrintWriter& w);
QC_IO_PKG PrintWriter& flush(PrintWriter& w);

QC_IO_NAMESPACE_END

#endif //QC_IO_PrintWriter_h

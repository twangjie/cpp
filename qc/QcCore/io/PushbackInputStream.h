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

/**
 * A <code>PushbackInputStream</code> adds
 * functionality to another input stream, namely
 * the  ability to "push back" or "unread"
 * one byte. This is useful in situations where
 * it is  convenient for a fragment of code
 * to read an indefinite number of data bytes
 * that  are delimited by a particular byte
 * value; after reading the terminating byte,
 * the  code fragment can "unread" it, so that
 * the next read operation on the input stream
 * will reread the byte that was pushed back.
 * For example, bytes representing the  characters
 * constituting an identifier might be terminated
 * by a byte representing an  operator character;
 * a method whose job is to read just an identifier
 * can read until it  sees the operator and
 * then push the operator back to be re-read.
 *
 * @author  Íõ½Ü
 * @date    2010-12-15
 */

#ifndef QC_IO_PushbackInputStream_h
#define QC_IO_PushbackInputStream_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "QcCore/io/InputStream.h"
#include "QcCore/io/FilterInputStream.h"
#include "QcCore/base/SynchronizedObject.h"

QC_IO_NAMESPACE_BEGIN

class QC_IO_PKG PushbackInputStream : public FilterInputStream, public SynchronizedObject
{
public:
	PushbackInputStream(InputStream *pInputStream);
	PushbackInputStream(InputStream *pInputStream, int size);
	~PushbackInputStream();

	virtual int read();
	virtual long read(Byte* pBuffer, size_t bufLen);
	void unread(int b);
	void unread(Byte *b, int len);
	virtual size_t available();
	virtual size_t skip(long n);
	virtual bool markSupported();
	virtual void reset();
	virtual void close();

protected:
	int pos;
	Byte *buf;
	int buf_size;

private:
	void init(int size);
	void ensureOpen();
};

QC_IO_NAMESPACE_END

#endif //QC_IO_PushbackInputStream_h


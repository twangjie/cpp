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

// Class PushbackInputStream
/** 
* @class qc::io::PushbackInputStream
*
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
#include "PushbackInputStream.h"
#include "QcCore/io/IOException.h"
#include "QcCore/base/SystemUtils.h"

QC_IO_NAMESPACE_BEGIN

PushbackInputStream::PushbackInputStream(InputStream* pInputStream) : FilterInputStream(pInputStream)
{
	init(1);
}

PushbackInputStream::PushbackInputStream(InputStream *pInputStream, int size) : FilterInputStream(pInputStream)
{
	init(size);
}

PushbackInputStream::~PushbackInputStream()
{
	if(buf)
	{
		delete[] this->buf;
		buf_size = 0;
	}
}

void PushbackInputStream::init(int size)
{
	if (size <= 0)
	{
		//throw IllegalArgumentException("size <= 0");
	}
	this->buf = new Byte[size];
	memset((void *)this->buf, 0x00, size);
	this->buf_size = size;
	this->pos = size;
}

/**
* Check to make sure that this stream has not been closed
*/
void PushbackInputStream::ensureOpen()
{
	if (this->getInputStream().isNull())
		throw IOException(QC_T("Stream closed"));
}

/**
* Reads the next byte of data from this input stream. The value byte is
* returned as an <code>int</code> in the range <code>0</code> to
* <code>255</code>. If no byte is available because the end of the stream
* has been reached, the value <code>-1</code> is returned. This method
* blocks until input data is available, the end of the stream is detected,
* or an exception is thrown.
* 
* <p>
* This method returns the most recently pushed-back byte, if there is one,
* and otherwise calls the <code>read</code> method of its underlying input
* stream and returns whatever value that method returns.
* 
* @return the next byte of data, or <code>-1</code> if the end of the
*         stream has been reached.
* @exception IOException
*                if this input stream has been closed by invoking its
*                {@link #close()} method, or an I/O error occurs.
* @see java.io.InputStream#read()
*/
int PushbackInputStream::read()
{
	ensureOpen();
	if (pos < buf_size)
	{
		return buf[pos++] & 0xff;
	}

	return this->getInputStream()->read();
}

/**
* Reads up to <code>len</code> bytes of data from this input stream into an
* array of bytes. This method first reads any pushed-back bytes; after
* that, if fewer than <code>len</code> bytes have been read then it reads
* from the underlying input stream. If <code>len</code> is not zero, the
* method blocks until at least 1 byte of input is available; otherwise, no
* bytes are read and <code>0</code> is returned.
* 
* @param b
*            the buffer into which the data is read.
* @param off
*            the start offset in the destination array <code>b</code>
* @param len
*            the maximum number of bytes read.
* @return the total number of bytes read into the buffer, or
*         <code>-1</code> if there is no more data because the end of the
*         stream has been reached.
* @exception NullPointerException
*                If <code>b</code> is <code>NULL</code>.
* @exception IndexOutOfBoundsException
*                If <code>off</code> is negative, <code>len</code> is
*                negative, or <code>len</code> is greater than
*                <code>b.length - off</code>
* @exception IOException
*                if this input stream has been closed by invoking its
*                {@link #close()} method, or an I/O error occurs.
* @see java.io.InputStream#read(byte[], int, int)
*/
//int PushbackInputStream::read(Byte* b, int off, int len)
long PushbackInputStream::read(Byte* b, size_t len)
{
	ensureOpen();
	//if (b == NULL)
	//{
	//	throw NullPointerException();
	//}
	//else if (off < 0 || len < 0 || len > b.length - off)
	//{
	//	//throw IndexOutOfBoundsException();
	//}
	//else if (len == 0)
	//{
	//	return 0;
	//}

	SystemUtils::TestBufferIsValid(b, len);

	int avail = len - pos;
	if (avail > 0)
	{
		if (len < avail)
		{
			avail = len;
		}
		//System.arraycopy(buf, pos, b, off, avail);
		::memcpy(b, buf + pos, avail);

		pos += avail;
		b += avail;
		len -= avail;
	}
	if (len > 0)
	{
		//len = super.read(b, len);
		len = this->getInputStream()->read(b, len);
		if (len == -1)
		{
			return avail == 0 ? -1 : avail;
		}
		return avail + len;
	}
	return avail;
}

/**
* Pushes back a byte by copying it to the front of the pushback buffer.
* After this method returns, the next byte to be read will have the value
* <code>(byte)b</code>.
* 
* @param b
*            the <code>int</code> value whose low-order byte is to be
*            pushed back.
* @exception IOException
*                If there is not enough room in the pushback buffer for the
*                byte, or this input stream has been closed by invoking its
*                {@link #close()} method.
*/
void PushbackInputStream::unread(int b)
{
	ensureOpen();
	if (pos == 0)
	{
		throw IOException(QC_T("Push back buffer is full"));
	}
	buf[--pos] = (Byte) b;
}

/**
* Pushes back a portion of an array of bytes by copying it to the front of
* the pushback buffer. After this method returns, the next byte to be read
* will have the value <code>b[off]</code>, the byte after that will have
* the value <code>b[off+1]</code>, and so forth.
* 
* @param b
*            the byte array to push back.
* @param off
*            the start offset of the data.
* @param len
*            the number of bytes to push back.
* @exception IOException
*                If there is not enough room in the pushback buffer for the
*                specified number of bytes, or this input stream has been
*                closed by invoking its {@link #close()} method.
* @since JDK1.1
*/
void PushbackInputStream::unread(Byte *b, int len)
{
	ensureOpen();
	if (len > pos)
	{
		throw new IOException(QC_T("Push back buffer is full"));
	}
	pos -= len;
	//System.arraycopy(b, off, buf, pos, len);
	::memcpy(buf + pos, b, len);
}

/**
* Returns an estimate of the number of bytes that can be read (or skipped
* over) from this input stream without blocking by the next invocation of a
* method for this input stream. The next invocation might be the same
* thread or another thread. A single read or skip of this many bytes will
* not block, but may read or skip fewer bytes.
* 
* <p>
* The method returns the sum of the number of bytes that have been pushed
* back and the value returned by
* {@link java.io.FilterInputStream#available available}.
* 
* @return the number of bytes that can be read (or skipped over) from the
*         input stream without blocking.
* @exception IOException
*                if this input stream has been closed by invoking its
*                {@link #close()} method, or an I/O error occurs.
* @see java.io.FilterInputStream#in
* @see java.io.InputStream#available()
*/
size_t PushbackInputStream::available()
{
	ensureOpen();
	return (buf_size - pos) + this->getInputStream()->available();
}

/**
* Skips over and discards <code>n</code> bytes of data from this input
* stream. The <code>skip</code> method may, for a variety of reasons, end
* up skipping over some smaller number of bytes, possibly zero. If
* <code>n</code> is negative, no bytes are skipped.
* 
* <p>
* The <code>skip</code> method of <code>PushbackInputStream</code> first
* skips over the bytes in the pushback buffer, if any. It then calls the
* <code>skip</code> method of the underlying input stream if more bytes
* need to be skipped. The actual number of bytes skipped is returned.
* 
* @param n
*            {@inheritDoc}
* @return {@inheritDoc}
* @exception IOException
*                if the stream does not support seek, or the stream has
*                been closed by invoking its {@link #close()} method, or an
*                I/O error occurs.
* @see java.io.FilterInputStream#in
* @see java.io.InputStream#skip(long n)
* @since 1.2
*/
size_t PushbackInputStream::skip(long n)
{
	ensureOpen();
	if (n <= 0)
	{
		return 0;
	}

	long pskip = buf_size - pos;
	if (pskip > 0)
	{
		if (n < pskip)
		{
			pskip = n;
		}
		pos += pskip;
		n -= pskip;
	}
	if (n > 0)
	{
		pskip += this->getInputStream()->skip(n);
	}
	return pskip;
}

/**
* Tests if this input stream supports the <code>mark</code> and
* <code>reset</code> methods, which it does not.
* 
* @return <code>false</code>, since this class does not support the
*         <code>mark</code> and <code>reset</code> methods.
* @see java.io.InputStream#mark(int)
* @see java.io.InputStream#reset()
*/
bool PushbackInputStream::markSupported()
{
	return false;
}

/**
* Repositions this stream to the position at the time the <code>mark</code>
* method was last called on this input stream.
* 
* <p>
* The method <code>reset</code> for class <code>PushbackInputStream</code>
* does nothing except throw an <code>IOException</code>.
* 
* @exception IOException
*                if this method is invoked.
* @see java.io.InputStream#mark(int)
* @see java.io.IOException
*/
void PushbackInputStream::reset()
{
	QC_SYNCHRONIZED;
	throw IOException(QC_T("mark/reset not supported"));
}

/**
* Closes this input stream and releases any system resources associated
* with the stream. Once the stream has been closed, further read(),
* unread(), available(), reset(), or skip() invocations will throw an
* IOException. Closing a previously closed stream has no effect.
* 
* @exception IOException
*                if an I/O error occurs.
*/
void PushbackInputStream::close()
{
	QC_SYNCHRONIZED;
	InputStream *in = this->getInputStream().get();
	if (in == NULL)
		return;
	in->close();
	in = NULL;

	delete [] buf;
	buf_size = 0;
	buf = NULL;
}

QC_IO_NAMESPACE_END

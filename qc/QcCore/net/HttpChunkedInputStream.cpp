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

#include "HttpChunkedInputStream.h"

#include "QcCore/base/SystemUtils.h"
#include "QcCore/base/UnicodeCharacterType.h"
#include "QcCore/io/IOException.h"

#include <ctype.h>

QC_NET_NAMESPACE_BEGIN

using io::IOException;

HttpChunkedInputStream::HttpChunkedInputStream(InputStream* pInputStream) :
	FilterInputStream(pInputStream),
	m_chunkSize(0),
	m_chunkRead(0),
	m_eof(false)
{
}

//==============================================================================
// HttpChunkedInputStream::read
//
//==============================================================================
long HttpChunkedInputStream::read(Byte* pBuffer, size_t bufLen)
{
	SystemUtils::TestBufferIsValid(pBuffer, bufLen);

	if(m_chunkRead == m_chunkSize && !m_eof)
	{
		readChunkHeader();
	}

	if(m_eof)
	{
		return EndOfFile;
	}
	else
	{
		QC_DBG_ASSERT(m_chunkRead < m_chunkSize);
		size_t chunkLeft = m_chunkSize - m_chunkRead;
		size_t maxBytes = (chunkLeft < bufLen) ? chunkLeft : bufLen;
		long numBytesRead = FilterInputStream::read(pBuffer, maxBytes);
		QC_DBG_ASSERT(numBytesRead > 0);
		m_chunkRead += numBytesRead;
		return numBytesRead;
	}
}

//==============================================================================
// HttpChunkedInputStream::readChunkHeader
//
// When m_chunkRead == m_chunkSize we expect to read a chunk header
// which comprises a hexadecimal size plus CRLF.
//
// The first header we read will start with the chunk size, but subsequent
// ones will have a leading CRLF.
//==============================================================================
void HttpChunkedInputStream::readChunkHeader()
{
	//
	// First extract the hex chunk size
	// (perhaps with leading white space)
	//
	bool bSkipWhitespace = true;
	ByteString strChunkSize;
	int x;

	while(true)
	{
		x = FilterInputStream::read();
		if(x == EndOfFile)
		{
			throw IOException(QC_T("HTTP Chunked encoding exception"));
		}
		if(isxdigit(x))
		{
			strChunkSize += char(x);
			bSkipWhitespace = false;
		}
		else if(UnicodeCharacterType::IsSpace(x) && bSkipWhitespace)
		{
			// carry on camping;
		}
		else
		{
			break;
		}
	}

	//
	// Now discard the rest of the line
	//
	while(x != EndOfFile && x != '\n')
	{
		x = FilterInputStream::read();
	}

	//
	// Decode the hexadecimal chunk size into an understandable number
	//
	m_chunkSize = strtol(strChunkSize.c_str(), 0, 16);

	//
	// The last chunk is followed by zero or more trailers
	// (in our case it should always be zero because we don't give permission
	// for trailers to be present), followed by a blank line.
	//
	if(m_chunkSize == 0)
	{
		m_eof = true;
		x = 0;
		size_t lineSize=0;
		while(x != EndOfFile && !(x == '\n' && lineSize == 0))
		{
			x = FilterInputStream::read();
			if(x != '\r' && x != '\n')
			{
				lineSize++;
			}
			else if(x == '\n')
			{
				lineSize=0;
			}
		}
	}

	m_chunkRead = 0;
}

QC_NET_NAMESPACE_END

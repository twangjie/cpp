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
// Class: HttpChunkedInputStream
// 
// Overview
// --------
// This class is an QC Technology extension.
//
// A HttpChunkedInputStream is used to decode chunked results from a
// HTTP 1.1 GET request.
//
// See RFC 2616 (3.6.1) For a description of Chunked Encoding
// In a nut-shell, the result of an HTTP request is split into chunks, each
// chunk is prefixed by a chunk size field.  The last chunk has a size of "0".
//
//=============================================================================

#ifndef QC_NET_HttpChunkedInputStream_h
#define QC_NET_HttpChunkedInputStream_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "QcCore/io/FilterInputStream.h"

QC_NET_NAMESPACE_BEGIN

using io::FilterInputStream;

class QC_NET_PKG HttpChunkedInputStream : public FilterInputStream
{
public:
	HttpChunkedInputStream(InputStream* pInputStream);

#ifdef QC_USING_DECL_BROKEN
	virtual int read() {return FilterInputStream::read();}
#else
	using FilterInputStream::read; 	// unhide inherited read methods
#endif

	virtual long read(Byte* pBuffer, size_t bufLen);

protected:
	void readChunkHeader();

private:
	size_t m_chunkSize;
	size_t m_chunkRead;
	bool m_eof;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_HttpChunkedInputStream_h


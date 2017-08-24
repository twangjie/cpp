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

#include "NvtAsciiInputStream.h"
#include "ProtocolException.h"

#include "QcCore/base/SystemUtils.h"

QC_NET_NAMESPACE_BEGIN

#if defined(WIN32)
	const char* EOLBytes = "\r\n";
	const size_t EOLBytesLength = 2;
#else
	const char* EOLBytes = "\n";
	const size_t EOLBytesLength = 1;
#endif

NvtAsciiInputStream::NvtAsciiInputStream(InputStream* pInputStream) :
	FilterInputStream(pInputStream)
{
}

//==============================================================================
// NvtAsciiInputStream::read
//
// Translate <CRLF> to local line-feed convention.
//==============================================================================
long NvtAsciiInputStream::read(Byte* pBuffer, size_t bufLen)
{
	long bytesRead = FilterInputStream::read(pBuffer, bufLen);

#if defined(WIN32)

	return bytesRead;

#else

	// In the UNIX domain, the local convention is <LF>
	//
	// translate all <CRLF> pairs to standalone <LF>

	if(bytesRead == EndOfFile)
	{
		return EndOfFile;
	}

	Byte* pNext = pBuffer;
	Byte* pEnd = pBuffer + bytesRead;
	const Byte CR = 13;
	const Byte LF = 10;

	while(pNext < pEnd)
	{
		const Byte b = *pNext++;
		if(b == CR)
		{
			//
			// Reset the <CR> to <LF> and check to see if we have enough
			// buffer space to zap the next byte
			//
			*--pNext = LF;
			if(pNext >= pEnd)
			{
				int b = FilterInputStream::read(); // eat the next byte - it must be <null> of <LF>
				if(b != 0 && b != LF)
				{
					throw ProtocolException(QC_T("invalid NVT-ASCII byte sequence"));
				}
			}
			else
			{
				//
				// We can now deal with the second half of the <CRLF> pair
				// which simply needs to be discarded
				//
				::memmove(pNext, pNext+1, pEnd-pNext-1);
				--bytesRead;
				--pEnd;
			}
		}
	}

	return bytesRead;

#endif // WIN32
}

QC_NET_NAMESPACE_END

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

#include "NvtAsciiOutputStream.h"

#include "QcCore/base/NullPointerException.h"

QC_NET_NAMESPACE_BEGIN

NvtAsciiOutputStream::NvtAsciiOutputStream(OutputStream* pOutputStream) :
	FilterOutputStream(pOutputStream),
	m_bCRSeen(false)
{
}

//==============================================================================
// NvtAsciiOutputStream::write
//
//==============================================================================
void NvtAsciiOutputStream::write(const Byte* pBuffer, size_t bufLen)
{
	if(!pBuffer) throw NullPointerException();

	//
	// Search for <CR> bytes not followed by a <LF> and replace by <CRLF>
	//
	const Byte* pEnd = pBuffer+bufLen;
	const Byte* pNext = pBuffer;
	const Byte* pLast = pBuffer;
	const Byte CR = 13;

	while(pNext < pEnd)
	{
		const Byte b = *pNext;
		if(b == CR)
		{
			m_bCRSeen = true;
		}
		else if(b == 10)
		{
			if(!m_bCRSeen)
			{
				// flush out what's gone so far
				FilterOutputStream::write(pLast, pNext-pLast);
				pLast = pNext;
				// write an additional <CR>
				FilterOutputStream::write(&CR, 1);
			}
			m_bCRSeen = false;
		}
		++pNext;
	}
	QC_DBG_ASSERT(pEnd == pNext);
	FilterOutputStream::write(pLast, pNext-pLast);
}

QC_NET_NAMESPACE_END

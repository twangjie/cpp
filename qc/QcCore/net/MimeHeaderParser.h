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

#ifndef QC_NET_MimeHeaderParser_h
#define QC_NET_MimeHeaderParser_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

#include "MimeHeaderSequence.h"

QC_NET_NAMESPACE_BEGIN

class QC_NET_PKG MimeHeaderParser
{
public:

	static AutoPtr<MimeHeaderSequence> ParseHeaders(InputStream* pInputStream);
	static long ReadLineLatin1(InputStream* pInputStream, String& retLine);
};

QC_NET_NAMESPACE_END

#endif //QC_NET_MimeHeaderParser_h

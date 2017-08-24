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
// Class: NetUtils
//
// This is a module containing portable implementations of 
// networking utility functions.
//
//==============================================================================

#ifndef QC_NET_NetUtils_h
#define QC_NET_NetUtils_h

#ifndef QC_NET_DEFS_h
#include "defs.h"
#endif //QC_NET_DEFS_h

QC_NET_NAMESPACE_BEGIN

class SocketDescriptor;

class QC_NET_PKG NetUtils
{
public:
	enum Direction {Read, Write, Both};

	static String GetLocalHostName();
	static int GetLastSocketError();
	static String GetSocketErrorString(int errorNum=0);
	static void InitializeSocketLibrary();
	static long SocketIOControl(SocketDescriptor* pSocketDescriptor, long cmd, long arg);
	static void SetBlockingSocket(SocketDescriptor* pSocketDescriptor, bool bBlocking);
	static void ShutdownSocket(SocketDescriptor* pSocketDescriptor, Direction dir);
	static bool SelectSocket(SocketDescriptor* pSocketDescriptor, size_t timeoutMS,
	                         bool bRead, bool bWrite);

private:
	NetUtils(); // not implemented

private:
	static bool s_bInitialized;
};

QC_NET_NAMESPACE_END

#endif //QC_NET_NetUtils_h


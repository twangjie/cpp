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
// Class: MemCheckSystemMonitor
// 
//==============================================================================

#ifndef QC_AUXIL_MemCheckSystemMonitor_h
#define QC_AUXIL_MemCheckSystemMonitor_h

#ifndef QC_AUXIL_DEFS_h
#include "defs.h"
#endif //QC_AUXIL_DEFS_h

#include "QcCore/base/SystemMonitor.h"

QC_AUXIL_NAMESPACE_BEGIN

class QC_AUXIL_PKG MemCheckSystemMonitor : public SystemMonitor
{
public:
	MemCheckSystemMonitor();
};

QC_AUXIL_NAMESPACE_END

#endif //QC_AUXIL_MemCheckSystemMonitor_h

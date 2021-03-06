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

#ifndef QC_AUXIL_messages_h
#define QC_AUXIL_messages_h

#include "defs.h"

QC_AUXIL_NAMESPACE_BEGIN

enum MessageCodes {

EAUXIL_OPTIONARGINVALID     = 100, // '{0}' is not a valid argument for the {1} option
EAUXIL_OPTIONARGMISSING     = 101, // the {0} option requires an argument
EAUXIL_OPTIONNAMEAMBIGUOUS  = 102, // the {0} option is ambiguous
EAUXIL_OPTIONNAMEUNKNOWN    = 103, // unrecognized option: {0}
EAUXIL_OPTIONNAMEBOOL       = 104, // illegal boolean option name: {0}
EAUXIL_OPTIONDUPLICATE      = 105, // duplicate option: {0}
EAUXIL_MAX

};

QC_AUXIL_NAMESPACE_END

#endif //QC_AUXIL_messages_h


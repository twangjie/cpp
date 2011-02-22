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

#include "MessageFormatter.h"

#include "QcCore/base/NumUtils.h"

#include <vector>
#include <stdarg.h>

QC_UTIL_NAMESPACE_BEGIN

static const String sUnknown = QC_T("?");

String MessageFormatter::Format(const String& spec, const String& arg1)
{
	return FormatImpl(spec, 1, &arg1);
}

String MessageFormatter::Format(const String& spec, const String& arg1,
									  const String& arg2)
{
	return FormatImpl(spec, 2, &arg1, &arg2);
}

String MessageFormatter::Format(const String& spec, const String& arg1,
									  const String& arg2,
									  const String& arg3)
{
	return FormatImpl(spec, 3, &arg1, &arg2, &arg3);
}

String MessageFormatter::Format(const String& spec, const String& arg1,
									  const String& arg2,
									  const String& arg3,
									  const String& arg4)
{
	return FormatImpl(spec, 4, &arg1, &arg2, &arg3, &arg4);
}

String MessageFormatter::Format(const String& spec, const String& arg1,
									  const String& arg2,
									  const String& arg3,
									  const String& arg4,
									  const String& arg5)
{
	return FormatImpl(spec, 5, &arg1, &arg2, &arg3, &arg4, &arg5);
}

String MessageFormatter::Format(const String& spec, const String& arg1,
									  const String& arg2,
									  const String& arg3,
									  const String& arg4,
									  const String& arg5,
									  const String& arg6)
{
	return FormatImpl(spec, 6, &arg1, &arg2, &arg3, &arg4, &arg5, &arg6);
}

//==============================================================================
// MessageFormatter::FormatImpl
//
// Do the actual work of parsing the specification string and replacing all
// occurences of {n} with the appropriate parameter value.
//
//==============================================================================
String MessageFormatter::FormatImpl(const String& spec, size_t count, ...)
{
	va_list marker;
	va_start(marker, count);     /* Initialize variable arguments. */

	std::vector<const String*> argVec;

	for(size_t i=0; i<count; ++i)
	{
		argVec.push_back(va_arg(marker, const String*));
	}
	
	va_end(marker);              /* Reset variable arguments.      */

	size_t lastpos=0;
	String ret;
	while(true)
	{
		size_t beginPos=spec.find('{', lastpos);
		if(beginPos == String::npos)
		{
			ret += spec.substr(lastpos);
			break;
		}
		else
		{
			size_t endPos=spec.find('}', beginPos);
			if(endPos == String::npos)
			{
				ret += spec.substr(lastpos);
				break;
			}
			else
			{
				ret += spec.substr(lastpos, beginPos-lastpos);
				lastpos = endPos+1;
				String argNum = spec.substr(beginPos+1, endPos-beginPos);
				size_t nArg = NumUtils::ToInt(argNum);
				if(nArg < argVec.size())
				{
					ret += *(argVec[nArg]);
				}
				else
				{
					ret += sUnknown;
				}
			}
		}
	}

	return ret;
}

QC_UTIL_NAMESPACE_END

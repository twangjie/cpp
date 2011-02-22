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
// Class: CodeConverterFactory
// 
//==============================================================================

#ifndef QC_CVT_CodeConverterFactory_h
#define QC_CVT_CodeConverterFactory_h

#ifndef QC_CVT_DEFS_h
#include "defs.h"
#endif //QC_CVT_DEFS_h

QC_CVT_NAMESPACE_BEGIN

class CodeConverter;

class QC_CVT_PKG CodeConverterFactory : public virtual ManagedObject
{
public:

	static CodeConverterFactory& GetInstance();
	static void SetInstance(CodeConverterFactory* pFactory);

	AutoPtr<CodeConverter> getConverter(const String& encoding) const;
	AutoPtr<CodeConverter> getDefaultConverter() const;

private:
	static CodeConverterFactory* QC_MT_VOLATILE s_pInstance;
};

QC_CVT_NAMESPACE_END

#endif //QC_CVT_CodeConverterFactory_h

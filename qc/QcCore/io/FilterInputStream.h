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
// Class: FilterInputStream
// 
//=============================================================================

#ifndef QC_IO_FilterInputStream_h
#define QC_IO_FilterInputStream_h

#ifndef QC_IO_DEFS_h
#include "defs.h"
#endif //QC_IO_DEFS_h

#include "InputStream.h"

QC_IO_NAMESPACE_BEGIN

class QC_IO_PKG FilterInputStream : public InputStream
{
protected:
	FilterInputStream(InputStream* pInputStream);

public:
	virtual void mark(size_t readLimit);
	virtual bool markSupported() const;
	virtual void reset();
	virtual size_t available();
	virtual void close();
	virtual int read();
	virtual long read(Byte* pBuffer, size_t bufLen);
	virtual size_t skip(size_t n);

protected:
	AutoPtr<InputStream> getInputStream() const;

private:
	AutoPtr<InputStream> m_rpInputStream;
};

QC_IO_NAMESPACE_END

#endif //QC_IO_FilterInputStream_h


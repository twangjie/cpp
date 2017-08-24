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
// Class: UTF8codecvt
// 
// Overview
// --------
// Provides a version of a std::codecvt that will encode/decode UTF-8
// byte streams.
//
//=============================================================================

#ifndef OT_CVT_UTF8codecvt_h
#define OT_CVT_UTF8codecvt_h

#ifndef OT_CVT_defs_h
#include "defs.h"
#endif //OT_CVT_defs_h

#include <locale>
#include "UTF8Converter.h"

OT_CVT_NAMESPACE_BEGIN

class UTF8codecvt : public std::codecvt<CharType, char, mbstate_t>
{
public:
	typedef std::codecvt<CharType, char, mbstate_t> codecvt_base;
    typedef CharType internT;
    typedef Byte externT;
    typedef mbstate_t stateT;

    explicit UTF8codecvt(size_t refs = 0)
        : codecvt_base(refs) {}

protected:
    virtual result do_in(stateT& state,
	                     const externT *from, const externT *from_end,
	                     const externT *& from_next,
	                     internT *to, internT *to_limit,
	                     internT *& to_next) const;

	virtual result do_out(stateT& state,
	                     const internT *from, const internT *from_end,
	                     const internT *& from_next,
	                     externT *to, externT *to_limit,
	                     externT *& to_next) const;

    virtual result do_unshift(stateT& state,
	                     externT *to, externT *to_limit, externT *& to_next) const;

    virtual int do_length(stateT& state, const externT *from,
	                     const externT *from_end, size_t max) const;

	virtual bool do_always_noconv() const;

	virtual int do_max_length() const;

	virtual int do_encoding() const;

private:
	UTF8Converter m_cvt;
};

OT_CVT_NAMESPACE_END

#endif //OT_CVT_UTF8codecvt_h

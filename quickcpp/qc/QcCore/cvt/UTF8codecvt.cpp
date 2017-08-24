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

#include "UTF8codecvt.h"

OT_CVT_NAMESPACE_BEGIN

//==============================================================================
// UTF8codecvt::do_in
//
// Decode a sequence of external characters into a sequence of internal
// characters.  As this is a job required elsewhere (by programs not using
// IOStreams) we delegate to another class that knows how to do it.
//==============================================================================
UTF8codecvt::result UTF8codecvt::do_in(stateT& state,
	const externT *from, const externT *from_end, const externT *& from_next,
	internT *to, internT *to_limit, internT *& to_next) const
{
	switch(const_cast<UTF8Converter&>(m_cvt).decode(from, from_end, from_next,
	                  to, to_limit, to_next))
	{
	case UTF8Converter::outputExhausted:
	case UTF8Converter::inputExhausted:
		return partial;
	case UTF8Converter::ok:
		return ok;
	default:
		return error;
	}
}

//==============================================================================
// UTF8codecvt::do_out
//
// Encode a sequence of internal characters into a sequence of external
// characters.  As this is a job required elsewhere (by programs not using
// IOStreams) we delegate to another class that knows how to do it.
//==============================================================================
UTF8codecvt::result UTF8codecvt::do_out(stateT& state,
		const internT *from, const internT *from_end, const internT *& from_next,
		externT *to, externT *to_limit, externT *& to_next) const
{
	switch(const_cast<UTF8Converter&>(m_cvt).encode(from, from_end, from_next,
	                             to, to_limit, to_next))
	{
	case UTF8Converter::outputExhausted:
	case UTF8Converter::inputExhausted:
		return partial;
	case UTF8Converter::ok:
		return ok;
	default:
		return error;
	}
}

//==============================================================================
// UTF8codecvt::do_unshift
//
//==============================================================================
UTF8codecvt::result UTF8codecvt::do_unshift(stateT& state,
	externT *to, externT *to_limit, externT *& to_next) const
{
	return (ok);
}

//==============================================================================
// UTF8codecvt::do_length
//
//==============================================================================
int UTF8codecvt::do_length(stateT& state, const externT *from,
	const externT *from_end, size_t max) const 
{
	size_t len = m_cvt.getDecodedLength(from, from_end);
	return len > max ? max : len;
}

//==============================================================================
// UTF8codecvt::do_always_noconv
//
// Return true if this is a non-converting codecvt.
//==============================================================================
bool UTF8codecvt::do_always_noconv() const
{
	return (false); // false = we do convert!
}

//==============================================================================
// UTF8codecvt::do_max_length
//
// Return the maximum number of external characters required to encode a 
// single internal character.
//==============================================================================
int UTF8codecvt::do_max_length() const 
{
	return m_cvt.getMaxEncodedLength();
}

//==============================================================================
// UTF8codecvt::do_encoding
//
// Return -1 if the encoding sequence is state dependent.  Otherwise return the
// constant number of external characters that are needed to produce one
// internal character, or 0 if this number is not constant
//==============================================================================
int UTF8codecvt::do_encoding() const 
{
	return (0); // variable number of bytes reqd to encode a single character
}

OT_CVT_NAMESPACE_END


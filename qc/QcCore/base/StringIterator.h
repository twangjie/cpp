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
// Class StringIterator
//
//==============================================================================

#ifndef QC_BASE_StringIterator_h
#define QC_BASE_StringIterator_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include "Character.h"
#include "SystemCodeConverter.h"

#include <utility>

QC_BASE_NAMESPACE_BEGIN

class QC_BASE_PKG StringIterator 

#ifdef QC_USE_STD_ITERATOR_BASE
	: public std::iterator<std::bidirectional_iterator_tag, Character>
#endif //QC_USE_STD_ITERATOR_BASE
{
public:

#ifndef QC_USE_STD_ITERATOR_BASE
	typedef std::bidirectional_iterator_tag  iterator_category;
	typedef Character                        value_type;
	typedef ptrdiff_t                        difference_type;
	typedef Character&                       reference;
	typedef CharType*                        pointer;
#endif //QC_USE_STD_ITERATOR_BASE

	StringIterator();
	StringIterator(const CharType* ptr);
	StringIterator& operator=(const CharType* ptr);

	bool operator==(const StringIterator& rhs) const;
	bool operator!=(const StringIterator& rhs) const;

	StringIterator& operator++();
	StringIterator operator++(int);
	StringIterator& operator--();
	StringIterator operator--(int);

	Character operator*() const;
	const CharType* data() const;

private:
	const CharType* m_ptr;
};

//==============================================================================
// StringIterator::StringIterator
//
/**
Default constructor.
*/
//==============================================================================
inline
	StringIterator::StringIterator() :
m_ptr(0)
{
}

//==============================================================================
// StringIterator::StringIterator
//
/**
Creates a StringIterator positioned at the first character of 
the sequence of ::CharType characters starting at @c ptr.
*/
//==============================================================================
inline
	StringIterator::StringIterator(const CharType* ptr) :
m_ptr(ptr)
{
}

//==============================================================================
// StringIterator::operator=
//
/**
Assignment operator.

Sets the position of this StringIterator to be the first character of 
the sequence of ::CharType characters starting at @c ptr.
*/
//==============================================================================
inline
	StringIterator& StringIterator::operator=(const CharType* ptr)
{
	m_ptr = ptr;
	return *this;
}

//==============================================================================
// StringIterator::operator==
//
/**
Equality operator.
@returns true if this StringIterator is pointing at the same location as
@c rhs; false otherwise.
*/
//==============================================================================
inline
	bool StringIterator::operator==(const StringIterator& rhs) const
{
	return (m_ptr == rhs.m_ptr);
}

//==============================================================================
// StringIterator::operator!=
//
/**
Inequality operator.
@returns false if this StringIterator is pointing at the same location as
@c rhs; true otherwise.
*/
//==============================================================================
inline
	bool StringIterator::operator!=(const StringIterator& rhs) const
{
	return (m_ptr != rhs.m_ptr);
}

//==============================================================================
// StringIterator::operator++()
//
/**
Prefix increment operator.

Positions the iterator at the next Unicode character in the sequence and
returns a reference to self.
*/
//==============================================================================
inline
	StringIterator& StringIterator::operator++()
{
	if(m_ptr) m_ptr+=SystemCodeConverter::GetCharSequenceLength(*m_ptr);
	return *this;
}

//==============================================================================
// StringIterator::operator++(int)
//
/**
Postfix increment operator.

Positions the iterator at the next Unicode character in the sequence and returns
a copy of self positioned at the original location.
*/
//==============================================================================
inline
	StringIterator StringIterator::operator++(int)
{
	const CharType* pOld = m_ptr;
	if(m_ptr) m_ptr+=SystemCodeConverter::GetCharSequenceLength(*m_ptr);
	return pOld;
}

//==============================================================================
// StringIterator::operator--()
//
/**
Prefix decrement operator.

Positions the iterator at the previous Unicode character in the sequence and
returns a reference to self.
*/
//==============================================================================
inline
	StringIterator& StringIterator::operator--()
{
	while(m_ptr && SystemCodeConverter::IsSequenceStartChar(*--m_ptr)) {}
	return *this;
}

//==============================================================================
// StringIterator::operator--(int)
//
/**
Postfix decrement operator.

Positions the iterator at the previous Unicode character in the sequence and returns
a copy of self positioned at the original location.
*/
//==============================================================================
inline
	StringIterator StringIterator::operator--(int)
{
	const CharType* pOld = m_ptr;
	while(m_ptr && SystemCodeConverter::IsSequenceStartChar(*--m_ptr)) {}
	return pOld;
}

//==============================================================================
// StringIterator::operator*
//
/**
Dereference operator.

@returns the Unicode character represented by the ::CharType sequence starting
at the current location of the iterator.
*/
//==============================================================================
inline
	Character StringIterator::operator*() const
{
	// Note: As we do not know the size of the charcater array,
	// We have to invent a length for the Character constructor.

	return Character(m_ptr, 6 /* any number >= utf-8 max len */);
}


//==============================================================================
// StringIterator::data()
//
/**
Returns a pointer to the current position within the ::CharType sequence.

@returns a pointer to the current data position of this iterator.
*/
//==============================================================================
inline
	const CharType* StringIterator::data() const
{
	return m_ptr;
}

QC_BASE_NAMESPACE_END

#endif //QC_BASE_StringIterator_h

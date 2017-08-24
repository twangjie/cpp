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

#ifndef QC_BASE_AutoBuffer_h
#define QC_BASE_AutoBuffer_h

#ifndef QC_BASE_DEFS_h
#include "defs.h"
#endif //QC_BASE_DEFS_h

#include "debug.h"

#include <memory.h>

QC_BASE_NAMESPACE_BEGIN

	template<typename T>
class AutoBuffer
{
public:
	typedef T DataType;

	AutoBuffer();	
	AutoBuffer(size_t initialSize);
	~AutoBuffer();

	void append(T data);
	void append(const T* pData, size_t length);
	size_t capacity() const;
	void clear();
	const T* data() const;
	void destroy();
	size_t size() const;

private:
	void init(size_t bufferSize);

private:
	T*  m_pBuffer;
	size_t m_used;
	size_t m_size;
};

//==============================================================================
// AutoBuffer<T>::AutoBuffer()
//
/**
Default constructor which creates an internal buffer with a default initial
size.
*/
//==============================================================================
template<typename T>
inline
	AutoBuffer<T>::AutoBuffer()
{
	const int defaultSize=240;
	init(defaultSize);
}

//==============================================================================
// AutoBuffer<T>::AutoBuffer(size_t initialSize)
//
/**
Default constructor which creates an internal buffer with an initial
size of @c initialSize.

@param initialSize the initial size of the internal buffer.
*/
//==============================================================================
template<typename T>
inline
	AutoBuffer<T>::AutoBuffer(size_t initialSize)
{
	init(initialSize);
}

//==============================================================================
// AutoBuffer<T>::~AutoBuffer()
//
/**
Destructor.  The internal buffer is freed.
*/
//==============================================================================
template<typename T>
inline
	AutoBuffer<T>::~AutoBuffer()
{
	delete [] m_pBuffer;
}

//==============================================================================
// AutoBuffer<T>::clear()
//
/**
Clears the internal buffer for re-use - the memory is not freed.
*/
//==============================================================================
template<typename T>
inline
	void AutoBuffer<T>::clear()
{
	m_used = 0;
}	

//==============================================================================
// AutoBuffer<T>::destroy()
//
/**
Frees the internal buffer.  Subsequent append() operations will cause a new
buffer to be allocated.
*/
//==============================================================================
template<typename T>
inline
	void AutoBuffer<T>::destroy()
{
	delete [] m_pBuffer;
	m_pBuffer = m_used = m_size = 0;
}

//==============================================================================
// AutoBuffer<T>::size()
//
/**
Returns the number of buffer elements currently in use.  This is not the
same as the capacity of the buffer.

@sa capacity()
*/
//==============================================================================
template<typename T>
inline
	size_t AutoBuffer<T>::size() const
{
	return m_used;
}

//==============================================================================
// AutoBuffer<T>::capacity()
//
/**
Returns the allocated size of the internal buffer.  This is not necessarily
the number of buffer elements that are currently being used.

@sa size()
*/
//==============================================================================
template<typename T>
inline
	size_t AutoBuffer<T>::capacity() const
{
	return m_size;
}

//==============================================================================
// AutoBuffer<T>::data()
//
/**
Returns a const pointer to the start of the internal buffer.
*/
//==============================================================================
template<typename T>
inline
	const T* AutoBuffer<T>::data() const
{
	return m_pBuffer;
}

//==============================================================================
// AutoBuffer<T>::append()
//
/**
Appends a single element of type T to the internal buffer.  The buffer is
re-sized if necessary.

@param data the element to append to the internal buffer
*/
//==============================================================================
template<typename T>
inline
	void AutoBuffer<T>::append(T data)
{
	append(&data, 1);
}

//==============================================================================
// AutoBuffer<T>::append()
//
/**
Appends an array of elements of type T to the internal buffer.  The buffer is
re-sized if necessary.

@param pData a pointer to the first element in the array
@param length the number of elements to append
*/
//==============================================================================
template<typename T>
inline
	void AutoBuffer<T>::append(const T* pData, size_t length)
{
	if(length > 0)
	{
		size_t freeSize = (m_size - m_used);
		if(freeSize < length)
		{
			//
			// Decide how big to make the new buffer.
			// We need a compromise between continually re-sizing the buffer,
			// causing memory fragmentation and memory starvation.
			//
			enum {MinSize = sizeof(DataType) <= 32 ? 31 : 7};
			size_t growth = (m_used + length) / 2;
			m_size = m_used + length + growth;
			if(m_size < MinSize) m_size = MinSize;

			DataType* pNewBuffer = new DataType[m_size];
			if(m_pBuffer)
			{
				if(m_used)
				{
					::memcpy((void*)pNewBuffer, (void*)m_pBuffer, m_used*sizeof(DataType));
				}
				delete [] m_pBuffer;
			}
			m_pBuffer = pNewBuffer;
		}

		QC_DBG_ASSERT(m_pBuffer!=0);
		QC_DBG_ASSERT(length <= (m_size - m_used));

		::memcpy(m_pBuffer+m_used, pData, length*sizeof(DataType));
		m_used += length;
	}
}

//==============================================================================
// AutoBuffer<T>::init
//
// Private initialization functions
//==============================================================================
template<typename T>
inline
	void AutoBuffer<T>::init(size_t bufferSize)
{
	m_pBuffer=0;
	m_used=0;
	m_size=bufferSize;

	if(m_size)
	{
		m_pBuffer = new DataType[m_size];
	}
}

QC_BASE_NAMESPACE_END

#endif //QC_BASE_AutoBuffer_h

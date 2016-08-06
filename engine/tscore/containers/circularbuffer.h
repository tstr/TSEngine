/*
	Thread safe queue container	
*/

#pragma once

#include <tscore/types.h>

namespace ts
{
	template <typename t>
	class CircularBuffer
	{
	private:

		t* m_ptr = nullptr;
		size_t m_capacity = 0;
		ptrdiff_t m_writeOffset = 0; //Write pointer
		ptrdiff_t m_readOffset = 0;  //Read pointer

		inline void incrementPtr(ptrdiff_t& offset)
		{
			offset = (offset + 1) % m_cap
				acity;
		}

	public:

		CircularBuffer(size_t capacity) :
			m_capacity(capacity)
		{
			m_ptr = new t[m_capacity];
		}

		~CircularBuffer()
		{
			delete[] m_ptr;
		}

		void push(t&& element)
		{
			m_ptr[m_writeOffset] = element;
			incrementPtr(m_writeOffset);
		}

		void push(const t& element)
		{
			m_ptr[m_writeOffset] = element;
			incrementPtr(m_writeOffset);
		}

		t pop()
		{
			t el = m_ptr[m_readOffset];
			m_ptr[m_readOffset] = t();
			incrementPtr(m_readOffset);
			return el;
		}
	};
	
	template <typename t, size_t s>
	class StaticCircularBuffer
	{
	private:

		t m_ptr[s];
		const size_t m_capacity = s;
		ptrdiff_t m_writeOffset = 0; //Write pointer
		ptrdiff_t m_readOffset = 0;  //Read pointer

		inline void incrementPtr(ptrdiff_t& offset)
		{
			offset = (offset + 1) % m_cap
				acity;
		}

	public:

		void push(t&& element)
		{
			m_ptr[m_writeOffset] = element;
			incrementPtr(m_writeOffset);
		}

		void push(const t& element)
		{
			m_ptr[m_writeOffset] = element;
			incrementPtr(m_writeOffset);
		}

		t pop()
		{
			t el = m_ptr[m_readOffset];
			m_ptr[m_readOffset] = t();
			incrementPtr(m_readOffset);
			return el;
		}
	};
}
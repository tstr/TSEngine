/*
	Stack container
*/

#pragma once

#include <tscore/types.h>

namespace ts
{
	class Stack
	{
	private:

		byte* m_ptr = nullptr;
		size_t m_capacity = 0;
		ptrdiff_t m_writeOffset = 0; //Write pointer
		ptrdiff_t m_readOffset = 0;  //Read pointer

		inline void incrementPtr(ptrdiff_t& offset, ptrdiff_t step)
		{
			offset = (offset + step) % m_capacity;
		}

	public:

		Stack(size_t reserve)
		{
			m_ptr = new byte[reserve];
		}

		~Stack()
		{
			delete[] m_ptr;
		}

		void write(void* block, size_t blocksize)
		{
			incrementPtr(m_writeOffset, 1);
		}

		void read(void** block, size_t& blocksize)
		{

		}
	};
}
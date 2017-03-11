/*
	Linear allocator class
*/

#pragma once

#include <tscore/abi.h>
#include <tscore/types.h>
#include <atomic>

namespace ts
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/*
		Linear allocator class:
		
		- Implemented as a thread safe stack
		  where individual elements can be added,
		  but individual elements cannot be removed.
	*/
	class LinearAllocator
	{
	private:
		
		//Range
		void* m_start;
		void* m_end;

		//True if class owns memory
		bool m_owns = false;
		
		//Stack pointer
		std::atomic<void*> m_ptr;
		
		inline void* alignPtr(void* ptr, size_t alignment)
		{
			size_t p = (size_t)ptr;
			
			if (p % alignment)
				p += alignment - (p % alignment);
			
			return (void*)p;
		}
		
	public:

		/*
			Construct an empty allocator
		*/
		LinearAllocator() :
			m_start(nullptr),
			m_end(nullptr),
			m_ptr(nullptr),
			m_owns(false)
		{}
		
		/*
			Preallocate a chunk of memory of given capacity
		*/
		LinearAllocator(size_t capacity) :
			m_start(nullptr),
			m_end(nullptr),
			m_owns(true)
		{
			m_start = (void*)new byte[capacity];
			m_end = (void*)((size_t)m_start + capacity);
			m_ptr = m_start;
		}
		
		/*
			Use a given range of preallocated memory as a pool
		*/
		LinearAllocator(void* begin, void* end)
		{
			m_start = begin;
			m_end = end;
			m_ptr = m_start;
			m_owns = false;
		}

		~LinearAllocator()
		{
			if (m_owns)
			{
				delete[] (byte*)m_start;
			}
		}
		
		//Disable copy construction
		LinearAllocator(const LinearAllocator&) = delete;
		
		//Allocate a chunk of memory from the stack
		void* alloc(size_t size, size_t alignment = 16)
		{
			size += alignment - 1;
			
			void* mem = m_ptr.fetch_add(size);
			
			if (((size_t)mem + size) > (size_t)m_end)
			{
				return nullptr;
			}
			else
			{
				return alignPtr(mem, alignment);
			}
		}
		
		//Allocate memory for a given type
		template<typename T>
		T* alloc(size_t count = 1, size_t alignment = alignof(T))
		{
			return new(this->alloc(sizeof(T) * count, alignment)) T[count];
		}

		//Reset stack pointer to start
		void reset()
		{
			m_ptr.store(m_start);
		}

		//Resizes capacity
		void resize(size_t capacity)
		{
			reset();

			if (m_owns)
			{
				delete[] (byte*)m_start;
			}

			m_start = (void*)new byte[capacity];
			m_end = (void*)((size_t)m_start + capacity);
			m_ptr = m_start;

			m_owns = true;
		}

		void resize(void* start, void* end)
		{
			reset();

			if (m_owns)
			{
				delete[](byte*)m_start;
			}

			m_start = start;
			m_end = end;
			m_ptr = m_start;

			m_owns = false;
		}

		//Get pointers

		const void* getStart() const
		{
			return m_start;
		}

		const void* getTop() const
		{
			return m_ptr;
		}

		const void* getEnd() const
		{
			return m_end;
		}

		void* getStart()
		{
			return m_start;
		}

		void* getTop()
		{
			return m_ptr;
		}

		void* getEnd()
		{
			return m_end;
		}
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<typename type_t>
	inline type_t* construct(type_t* ptr, const type_t& other)
	{
		return new(ptr) type_t(other);
	}

	template<typename type_t, typename ... args_t>
	inline type_t* construct(type_t* ptr, args_t&&... args)
	{
		return new(ptr) type_t(std::forward<args_t>(args)...);
	}

	template<typename type_t>
	inline void destroy(type_t* ptr)
	{
		if (ptr != nullptr)
		{
			ptr->~type_t();
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

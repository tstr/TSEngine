/*
	Memory header - provides helper classes for memory management
*/

#pragma once

#include <tscore/types.h>
#include <memory>
#include <vector>

#define ALIGN(x) __declspec(align(x))

namespace ts
{
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <int X = 32>
	class Aligned
	{
	public:
			
		void* operator new(std::size_t n){ return _aligned_malloc(n, X); }
		void operator delete(void * p) throw() { _aligned_free(p); }
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
	class IAllocator
	{
	public:

		//Memory block type
		typedef void* block;

		virtual block allocate(size_t sz) = 0;
		virtual void free(block mem) = 0;
	};

	/////////////////////////////////////////////////////////////////////////////

	class DefaultAllocator : public IAllocator
	{
	public:

		block allocate(size_t sz) override { return malloc(sz); }
		void free(block mem) override { free(mem); }
	};
	*/

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	template<typename t, typename d = std::default_delete<t>>
	using UPtr = std::unique_ptr<t, d>;

	template<typename t>
	using SPtr = std::shared_ptr<t>;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////

	class MemoryView
	{
	public:

		byte* begin() { return m_start; }
		const byte* begin() const { return m_start; }
		byte* end() { return m_end; }
		const byte* end() const { return m_end; }

		byte* pointer() { return m_start; }
		const byte* pointer() const { return m_start; }

		size_t size() const { return m_end - m_start; }

	protected:

		byte* m_start;
		byte* m_end;
	};

	class MemoryBuffer : public MemoryView
	{
	public:

		MemoryBuffer() {}

		MemoryBuffer(size_t reserve)
		{
			m_start = new byte[reserve];
			m_end = m_start + reserve;
		}

		MemoryBuffer(const void* data, size_t size) : 
			MemoryBuffer((const byte*)data, size)
		{}

		MemoryBuffer(const byte* data, size_t size)
		{
			m_start = new byte[size];
			m_end = m_start + size;

			memcpy_s(m_start, this->size(), data, size);
		}

		explicit MemoryBuffer(const MemoryBuffer& copy)
		{
			if (this->size() < copy.size())
			{
				reset();
				m_start = new byte[copy.size()];
				m_end = m_start + copy.size();
			}

			memcpy_s(this->begin(), this->size(), copy.begin(), copy.size());
		}

		MemoryBuffer(MemoryBuffer&& other)
		{
			std::swap(m_start, other.m_start);
			std::swap(m_end, other.m_end);
		}

		MemoryBuffer& operator=(MemoryBuffer&& other)
		{
			std::swap(m_start, other.m_start);
			std::swap(m_end, other.m_end);
			return *this;
		}

		MemoryBuffer& operator=(const MemoryBuffer& copy)
		{
			*this = MemoryBuffer(copy);
		}

		~MemoryBuffer() { reset(); }

		void reset()
		{
			if (m_start != nullptr)
				delete m_start;

			m_start = nullptr;
			m_end = nullptr;
		}


		template<typename Type, typename = std::enable_if<std::is_pod<Type>::value>::type>
		static MemoryBuffer from(const Type& t)
		{
			return MemoryBuffer((const byte*)&t, sizeof(Type));

		}

		template<typename Type, typename = std::enable_if<std::is_pod<Type>::value>::type>
		static MemoryBuffer fromVector(const std::vector<Type>& v)
		{
			return MemoryBuffer((const byte*)&v[0], v.size() * sizeof(T));
		}
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////
}
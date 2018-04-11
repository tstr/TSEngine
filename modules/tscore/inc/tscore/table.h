/*
	Table

	contains two main classes:
		
		- HandleAllocator - a class responsible for allocating unique integer handles

		- Table - a wrapper around a flat array and HandleAllocator object,
				  it functions as an object pool where objects can be referenced
				  by a unique handle instead of by direct pointer.
*/

#pragma once

#include <tscore/types.h>

#include <vector>
#include <deque>

namespace ts
{
	template<typename int_t>
	struct HandleProperties;

	/*
		Properties of 64 bit handle
	*/
	template<>
	struct HandleProperties<uint64>
	{
		static const uint8 genBits = 16;
		static const uint8 idxBits = 48;
		static const uint64 maxIdx = ((uint64)1 << idxBits) - 1;
		static const uint64 maxGen = ((uint64)1 << genBits) - 1;

		typedef uint64 IdxType;
		typedef uint16 GenType;
	};

	/*
		Properties of 32 bit handle
	*/
	template<>
	struct HandleProperties<uint32>
	{
		static const uint8 genBits = 4;
		static const uint8 idxBits = 28;
		static const uint32 maxIdx = ((uint32)1 << idxBits) - 1;
		static const uint32 maxGen = ((uint32)1 << genBits) - 1;

		typedef uint32 IdxType;
		typedef uint8 GenType;
	};

	/*
		Helper struct for extracting the generation/index from a handle
	*/
	template<typename Handle_t>
	struct HandleInfo
	{
		typedef HandleProperties<Handle_t> Prop;

		union
		{
			Handle_t handle = 0;

			struct
			{
				Handle_t index		: Prop::idxBits;
				Handle_t generation : Prop::genBits;
			};
		};

		HandleInfo() : handle(0) {}

		HandleInfo(Handle_t h) : handle(h) {}
	};
	
	/*
		Handle Allocator class

		- allocates unique integers.
	*/
	template<typename Handle_t = uint32, uint32 MIN_FREE_INDICES = 512>
	class HandleAllocator
	{
	private:

		typedef HandleProperties<Handle_t> Prop;

		typedef typename Prop::IdxType Idx_t;
		typedef typename Prop::GenType Gen_t;

		std::vector<Gen_t> m_generations;
		std::deque<Idx_t> m_freeIndices;

		inline Handle_t formatHandle(Idx_t idx, Gen_t gen)
		{
			if (idx > Prop::maxIdx)
				throw std::bad_alloc();

			return (idx | ((Handle_t)gen << Prop::idxBits));
		}

		inline Gen_t getGen(Handle_t h) const
		{
			return HandleInfo<Handle_t>(h).generation;
		}

		inline Idx_t getIdx(Handle_t h) const
		{
			return HandleInfo<Handle_t>(h).index;
		}

	public:

		//Allocates a unique integer
		void alloc(Handle_t& h)
		{
			Idx_t idx = 0;

			if (m_freeIndices.size() > MIN_FREE_INDICES)
			{
				idx = m_freeIndices.front();
				m_freeIndices.pop_front();
			}
			else
			{
				m_generations.push_back(0);
				idx = (Idx_t)m_generations.size() - 1;
			}

			h = Handle_t(formatHandle(idx, m_generations[(Idx_t)idx]));
		}

		//Checks if a given handle has been allocated
		bool exists(Handle_t h) const
		{
			return ((Idx_t)getIdx(h) < m_generations.size()) && (m_generations[(Idx_t)getIdx(h)] == getGen(h));
		}

		//Free's the given handle for reuse
		void free(Handle_t h)
		{
			if (exists(h))
			{
				Idx_t idx = getIdx(h);
				++m_generations[(size_t)idx];
				m_freeIndices.push_back(idx);
			}
		}
	};

	/*
		Table class
		
		An array where it's elements are referenced by unique handles rather than pointers.
	*/
	template<typename value_t, typename Handle_t = uint32>
	class Table
	{
	private:

		//Table entry
		struct Entry
		{
			Handle_t handle;
			value_t  value;
		};

		HandleAllocator<Handle_t> m_allocator;
		std::vector<Entry> m_table;

		inline Handle_t getIdx(Handle_t h) const
		{
			return HandleInfo<Handle_t>(h).index;
		}

	public:
		
		//Create a table entry and initialize it with a given value
		void create(value_t&& val, Handle_t& h)
		{
			m_allocator.alloc(h);

			if (m_table.size() <= getIdx(h))
				m_table.resize((size_t)getIdx(h) + 1);

			Entry& e = m_table.at((size_t)getIdx(h));
			e.handle = h;
			e.value = move(val);
		}

		void create(const value_t& val, Handle_t& h)
		{
			this->create(value_t(val), h);
		}

		//Destroy a table entry
		void destroy(Handle_t h)
		{
			if (!m_allocator.exists(h))
			{
				return;
			}

			Entry& e = m_table.at((size_t)getIdx(h));
			e.handle = 0;
			e.value = value_t();

			m_allocator.free(h);
		}

		// Set a table entry to a given value
		// Returns true if the entry exists
		bool set(Handle_t h, value_t&& val)
		{
			if (!m_allocator.exists(h))
			{
				return false;
			}

			m_table.at(getIdx(h)).value = val;

			return true;
		}

		bool set(Handle_t h, const value_t& val)
		{
			return this->set(h, value_t(val));
		}

		// Get the value of a given table entry
		// Returns true if the entry exists
		const value_t& get(Handle_t h) const
		{
			if (!m_allocator.exists(h))
			{
				return value_t();
			}

			return m_table.at(getIdx(h)).value;
		}

		void clear()
		{
			for (Entry& e : m_table)
			{
				m_allocator.free(e.handle);
			}

			m_table.clear();
		}

		// Return copy of array values - todo: proper iterators
		std::vector<value_t> getArray()
		{
			std::vector<value_t> copy;
			copy.reserve(m_table.size());

			for (const Entry& e : m_table)
			{
				copy.push_back(e.value);
			}

			return move(copy);
		}
	};
}

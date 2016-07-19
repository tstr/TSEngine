/*
	Entity system
*/

#pragma once

#include "corecommon.h"
#include "assert.h"

#include <vector>
#include <deque>


#define ENTITY_INDEX_BITS 48
#define ENTITY_GEN_BITS 16
#define ENTITY_MINIMUM_FREE_INDICES 512
#define ENTITY_MAXIMUM_INDICES 281474976710654 /*48 bit maximum value*/
#define ENTITY_MAXIMUM_GEN 65535			   /*16 bit maximum value*/

namespace C3E
{
	class Entity
	{
	public:

		uint64 id() const { return m_id; };
		uint64 index() const { return m_index; }
		uint64 generation() const { return m_generation; }

		Entity() { m_id = 0; }
		Entity(uint64 id) { m_id = id; }

	private:

		union
		{
			uint64 m_id;

			struct
			{
				uint64 m_index : ENTITY_INDEX_BITS;
				uint64 m_generation : ENTITY_GEN_BITS;
			};
		};

	};

	class EntityManager
	{
	private:

		std::vector<uint16> m_generations;
		std::deque<uint64> m_freeIndices;

		inline uint64 createID(uint64 idx, uint16 gen)
		{
			if (idx > ENTITY_MAXIMUM_INDICES)
				throw std::bad_alloc();

			return (idx | ((uint64)gen << ENTITY_INDEX_BITS));
		}

	public:

		void create(Entity& e)
		{
			uint64 idx = 0;

			if (m_freeIndices.size() > ENTITY_MINIMUM_FREE_INDICES)
			{
				idx = m_freeIndices.front();
				m_freeIndices.pop_front();
			}
			else
			{
				m_generations.push_back(0);
				idx = m_generations.size() - 1;
			}

			e = Entity(createID(idx, m_generations[(size_t)idx]));
		}

		bool alive(Entity e) const
		{
			return (m_generations[(size_t)e.index()] == e.generation());
		}

		void destroy(Entity e)
		{
			uint64 idx = e.index();
			++m_generations[(size_t)idx];
			m_freeIndices.push_back(idx);
		}
	};

	template<class cmp_t>
	class ComponentManager
	{
	private:

		EntityManager* m_manager = nullptr;
		vector<cmp_t> m_components;

	public:

		ComponentManager(EntityManager* manager) :
			m_manager(manager)
		{
			assert(m_manager);
		}

		bool getComponent(Entity e, cmp_t& cmp)
		{
			if (m_components.size() <= e.index() || !m_manager->alive(e))
				return false;

			cmp = m_components.at((size_t)e.index());

			return true;
		}

		bool setComponent(Entity e, const cmp_t& cmp)
		{
			if (!m_manager->alive(e))
				return false;

			if (m_components.size() <= e.index())
				m_components.resize((size_t)e.index() + 1);

			m_components.at((size_t)e.index()) = cmp;

			return true;
		}
	};
}
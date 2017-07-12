/*
	Entity System
*/

#pragma once

#include <tscore/table.h>
#include <tscore/debug/assert.h>

namespace ts
{
	typedef uint32 Entity;

	class EntityManager
	{
	private:

		HandleAllocator<Entity> m_allocator;

	public:

		void create(Entity& e)
		{
			m_allocator.alloc(e);
		}

		bool alive(Entity e) const
		{
			return m_allocator.exists(e);
		}

		void destroy(Entity e)
		{
			m_allocator.free(e);
		}
	};

	template<class cmp_t>
	class ComponentManager
	{
	private:

		EntityManager* m_manager = nullptr;
		std::vector<cmp_t> m_components;

	public:

		ComponentManager(EntityManager* manager) :
			m_manager(manager)
		{
			tsassert(m_manager);
		}

		bool getComponent(Entity e, cmp_t& cmp)
		{
			const size_t idx = HandleInfo<Entity>(e).index;

			if (m_components.size() <= idx || !m_manager->alive(e))
				return false;

			cmp = m_components.at(idx);

			return true;
		}

		bool setComponent(Entity e, const cmp_t& cmp)
		{
			if (!m_manager->alive(e))
				return false;

			const size_t idx = HandleInfo<Entity>(e).index;

			if (m_components.size() <= idx)
				m_components.resize(idx + 1);

			m_components.at(idx) = cmp;

			return true;
		}
	};

}

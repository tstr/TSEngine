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

	template<class Component_t, typename = std::enable_if<std::is_move_constructible<Component_t>::value>>
	class ComponentMap
	{
	private:

		struct Entry
		{
			//Empty entity is -1
			Entity key = -1;
			Component_t value;

			Entry() {}
			Entry(Entity e, Component_t c) : key(e), value(std::move(c)) {}
		};

		EntityManager* m_manager = nullptr;
		std::vector<Entry> m_entries;

		Component_t m_default;

	public:

		ComponentMap() = default;

		/*
			Construct a component map object:

			Only maps entities from the entity manager specified as an argument.
		*/
		ComponentMap(EntityManager* manager) :
			m_manager(manager)
		{
			tsassert(m_manager);
		}

		ComponentMap(ComponentMap&& map)
		{
			std::swap(this->m_manager, map.m_manager);
			std::swap(this->m_entries, map.m_entries);
		}

		ComponentMap& operator=(ComponentMap&& map)
		{
			std::swap(this->m_manager, map.m_manager);
			std::swap(this->m_entries, map.m_entries);
			return *this;
		}

		ComponentMap(const ComponentMap& map) = delete;

		/*
			Check if a given entity has a corresponding component in this component map
		*/
		bool hasComponent(Entity e)
		{
			const size_t idx = HandleInfo<Entity>(e).index;

			return m_manager && (m_entries.at(idx).key != -1);
		}

		/*
			Get a component entry
		* /
		Component_t& getComponent(Entity e)
		{
			const size_t idx = HandleInfo<Entity>(e).index;

			//If entity index in range and entity is still alive
			if (m_manager && m_entries.size() > idx && m_manager->alive(e))
			{
				const Entry& entry = m_entries.at(idx);

				//If entity has this component then it will have an entry in component list
				if (entry.key != -1)
				{
					return entry.value;
				}
			}

			return m_default;
		}
		//*/

		const Component_t& getComponent(Entity e) const
		{
			const size_t idx = HandleInfo<Entity>(e).index;

			//If entity index in range and entity is still alive
			if (m_manager && m_entries.size() > idx && m_manager->alive(e))
			{
				const Entry& entry = m_entries.at(idx);

				//If entity has this component then it will have an entry in component list
				if (entry.key != -1)
				{
					return entry.value;
				}
			}

			return m_default;
		}

		/*
			Set a component entry
		*/
		bool setComponent(Entity e, const Component_t& cmp)
		{
			//If entity exists
			if (m_manager && m_manager->alive(e))
			{
				const size_t idx = HandleInfo<Entity>(e).index;

				//If component array is not correct size then resize 
				if (m_entries.size() <= idx)
					m_entries.resize(idx + 1);

				m_entries.at(idx) = Entry(e, cmp);

				return true;
			}
			
			return false;
		}

		bool setComponent(Entity e, Component_t&& cmp)
		{
			//If entity exists
			if (m_manager && m_manager->alive(e))
			{
				const size_t idx = HandleInfo<Entity>(e).index;

				//If component array is not correct size then resize 
				if (m_entries.size() <= idx)
					m_entries.resize(idx + 1);

				m_entries.at(idx) = Entry(e, move(cmp));

				return true;
			}

			return false;
		}
	};

}

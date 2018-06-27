/*
    Binding set helper class
*/

#pragma once

#include <tscore/types.h>
#include <vector>

namespace ts
{
    template<typename Type>
    class BindingSet
    {
    public:

		//Set the value of a slot
		void set(uint32 slot, const Type& value)
		{
			checkCapacity(slot);
			m_isBound[slot] = true;
			m_values[slot] = value;
		}

		//Set the value of a slot
		void set(uint32 slot, Type&& value)
		{
			checkCapacity(slot);
			m_isBound[slot] = true;
			m_values[slot] = value;
		}

		//Unset the value of a slot
		void unset(uint32 slot)
		{
			if (bound(slot))
			{
				m_isBound[slot] = false;
				m_values[slot] = Type();
			}
		}

		//Get a value at the given slot, returns Type() if the slot is not bound
		Type get(uint32 slot)
		{
			if (bound(slot))
				return m_values[slot];

			return Type();
		}
        
		//Check if the given slot has been set
        bool bound(uint32 slot) const
		{
			if (slot < count())
			{
				return m_isBound[slot];
			}

			return false;
		}

		//Access a slot, creates a new value if it does not exist
		Type& at(uint32 slot)
		{
			checkCapacity(slot);
			
			if (!bound(slot))
			{
				m_isBound[slot] = true;
				m_values[slot] = Type();
			}
			
			return m_values[slot];
		}

		Type& operator[](uint32 slot)
		{
			return at(slot);
		}

		const Type* data() const { return m_values.data(); }
		size_t count() const { return m_values.size(); }
		void clear() { m_slots.clear(); }
        
    private:

		void checkCapacity(uint32 slot)
		{
			if (slot >= count())
			{
				m_isBound.resize(slot + 1);
				m_values.resize(slot + 1);
			}
		}

		std::vector<bool> m_isBound;
        std::vector<Type> m_values;
    };
}

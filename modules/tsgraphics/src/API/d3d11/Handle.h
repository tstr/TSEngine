/*
	Render API
	
	Handle class
*/

#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	template<typename interface_t, typename handle_t>
	class Handle
	{
	private:

		typedef size_t Hash;

		static Hash getHash() { return typeid(interface_t).hash_code(); }
		
		Hash m_id;

	public:

		Handle() :
			m_id(Handle<interface_t, handle_t>::getHash())
		{}
		
		static interface_t* upcast(handle_t h)
		{
			if (Handle* i = reinterpret_cast<Handle*>(h))
			{
				if (i->m_id == getHash())
				{
					return static_cast<interface_t*>(i);
				}
			}

			return nullptr;
		}

		static handle_t downcast(interface_t* i)
		{
			return (handle_t)reinterpret_cast<handle_t&>(i);
		}
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Delegate class
*/

#pragma once

#include <tscore/types.h>

namespace ts
{
	template<typename t> class Delegate;

	template<typename ret_t, typename ... args_t>
	class Delegate<ret_t(args_t...)>
	{
	private:
		
		using signature_t = ret_t(*)(args_t...);
		using stubptr_t = ret_t(*)(void*, args_t...);
		using storage_t = void*;
		
		storage_t m_objectPtr = nullptr;
		stubptr_t m_stubPtr = nullptr;
		
		template<typename object_t, ret_t(object_t::*object_method)(args_t...)>
		static ret_t methodStub(void* objectptr, args_t ... args)
		{
			object_t* object = static_cast<object_t*>(objectptr);
			return (object->*object_method)(args...);
		};

		template<typename functor_t>
		static ret_t functorStub(void* funcptr, args_t ... args)
		{
			auto& functor = *static_cast<functor_t*>(funcptr);
			functor(args...);
		}
		
	public:
		
		//Constructors
		Delegate() = default;

		Delegate(Delegate const&) = default;

		Delegate(Delegate&&) = default;
		
		Delegate(std::nullptr_t const) noexcept : Delegate() { }

		//Construct delegate from free function pointer
		Delegate(ret_t(*funcptr)(args_t...))
		{
			m_objectPtr = static_cast<void*>(funcptr);
			m_stubPtr = [](void* ptr, args_t... args) { static_cast<signature_t>(ptr)(args...); };
		}

		//Construct a delegate from a functor
		template<typename functor_t>
		explicit Delegate(functor_t& functor)
		{
			m_objectPtr = static_cast<void*>(&functor);
			m_stubPtr = &functorStub<functor_t>;
		}
		
		//Call the delegate
		ret_t operator()(args_t ... args) const
		{
			return (*m_stubPtr)(m_objectPtr, args...);
		}

		Delegate& operator=(Delegate d)
		{
			m_objectPtr = d.m_objectPtr;
			m_stubPtr = d.m_stubPtr;
			return *this;
		}

		//Operator overloads
		bool operator==(Delegate d) const noexcept
		{
			return (d.m_stubPtr == m_stubPtr) && (d.m_objectPtr == m_objectPtr);
		}
		
		bool operator!=(Delegate d) const noexcept
		{
			return !this->operator==(d);
		}
		
		bool operator==(std::nullptr_t const) const noexcept
		{
			return m_stubPtr == nullptr;
		}

		bool operator!=(std::nullptr_t const) const noexcept
		{
			return m_stubPtr != nullptr;
		}
		
		//Static methods
		template<typename object_t, ret_t(object_t::*object_method)(args_t...)>
		static Delegate fromMethod(object_t* object)
		{
			Delegate d;
			d.m_objectPtr = (void*)object;
			d.m_stubPtr = &Delegate::methodStub<object_t, object_method>;
			return d;
		}
	};
}
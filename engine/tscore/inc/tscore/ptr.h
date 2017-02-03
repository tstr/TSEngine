/*
	Pointer header -
		
		contains OpaquePtr template class and a useful macro for managing classes using the Pimpl idiom
*/

#pragma once


#include <memory>
#include <utility>

/*
	OPAQUE_PTR macro should be placed in the public section of a class using the Pimpl idiom
	and can be used in conjunction with the OpaquePtr<> smart pointer
	
	example:
	
		//public.h
		
		class PublicClass
		{
		public:
			
			PublicClass() {}
			PublicClass(int A);
			~PublicClass();		//destructor must not be defined in header
			
			//other methods
			
			class PrivateImpl;
			
			OPAQUE_PTR(PublicClass, pImpl)
			
		private:
			
			OpaquePtr<PrivateImpl> pImpl;
		};
		
		//////////////////////////////////////////////////////////////
		
		//private.cpp
		
		#include "public.h"
		
		
		class PublicClass::PrivateImpl
		{
		private:
			
			int m_A;
			
		public:
			
			PublicClass(int A) : m_A(A)
			{
				
			}
		};
		
		PublicClass::PublicClass(int A) : pImpl(new PublicClass(A))
		{
			
		}
		
		PublicClass::~PublicClass()
		{
			
		}
		

*/
#define OPAQUE_PTR(CLASSNAME, IMPL_PTR)\
CLASSNAME(const CLASSNAME&) = delete;\
CLASSNAME(CLASSNAME&& rhs)\
{\
	using namespace std;\
	auto i = move(this->IMPL_PTR);\
	this->IMPL_PTR = move(rhs.IMPL_PTR);\
	rhs.IMPL_PTR = move(i);\
}\
CLASSNAME& operator=(const CLASSNAME&) = delete;\
CLASSNAME& operator=(CLASSNAME&& rhs)\
{\
	using namespace std;\
	auto i = move(this->IMPL_PTR);\
	this->IMPL_PTR = move(rhs.IMPL_PTR);\
	rhs.IMPL_PTR = move(i);\
	return *this;\
}\
operator bool() const\
{\
	return IMPL_PTR != nullptr;\
}

namespace ts
{
	template<typename type_t>
	class OpaquePtr
	{
	private:

		type_t* m_ptr;

	public:

		OpaquePtr() : m_ptr(nullptr) {}
		OpaquePtr(type_t* ptr) : m_ptr(ptr) {}

		OpaquePtr(const OpaquePtr<type_t>&) = delete;

		OpaquePtr(OpaquePtr<type_t>&& rhs)
		{
			auto ptr = this->m_ptr;
			this->m_ptr = rhs.m_ptr;
			rhs.m_ptr = ptr;
		}

		OpaquePtr<type_t>& operator=(const OpaquePtr<type_t>&) = delete;

		OpaquePtr<type_t>& operator=(OpaquePtr<type_t>&& rhs)
		{
			auto ptr = this->m_ptr;
			this->m_ptr = rhs.m_ptr;
			rhs.m_ptr = ptr;

			return *this;
		}

		operator bool() const
		{
			return m_ptr != nullptr;
		}

		bool operator==(std::nullptr_t) const
		{
			return m_ptr == nullptr;
		}

		bool operator!=(std::nullptr_t) const
		{
			return m_ptr != nullptr;
		}

		type_t& operator*()
		{
			return *m_ptr;
		}

		const type_t& operator*() const
		{
			return *m_ptr;
		}

		type_t* operator->() const
		{
			return get();
		}


		type_t* get() const
		{
			return m_ptr;
		}

		void reset(type_t* ptr = nullptr)
		{
			if (m_ptr)
			{
				delete m_ptr;
			}

			m_ptr = ptr;
		}

		type_t* detach()
		{
			auto ptr = m_ptr;
			m_ptr = nullptr;
			return ptr;
		}
	};
}

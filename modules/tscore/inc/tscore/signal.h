/*
	Event Signal class
*/

#pragma once

#include <functional>
#include <vector>
#include "delegate.h"

namespace ts
{
	template<typename ... ArgTypes>
	class Signal
	{
	public:

		using CallbackType = Delegate<void(ArgTypes...)>;

		//Connect a signal to a callback
		template<typename FunctionType>
		void connect(FunctionType& callback)
		{
			m_callbacks.push_back(callback);
		}

		//Alias for connect()
		template<typename FunctionType>
		void operator+=(FunctionType& callback)
		{
			connect(callback);
		}

		//Invoked all connected callbacks with the supplied arguments
		void operator()(ArgTypes& ... args)
		{
			for (const CallbackType& c : m_callbacks)
			{
				c(args...);
			}
		}

	private:

		std::vector<CallbackType> m_callbacks;
	};
}

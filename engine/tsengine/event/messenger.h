/*
	Event system
*/

#pragma once

#include <tscore/types.h>
#include <tscore/containers/threadqueue.h>

namespace ts
{
	template<typename Message_t>
	class CMessageReciever
	{
	public:

		inline void post(const Message_t& msg)
		{
			m_messageQueue.push(msg);
		}

		inline void get(Message_t& msg)
		{
			msg = m_messageQueue.pop();
		}

	private:

		ThreadQueue<Message_t> m_messageQueue;
	};
}

/*

{
	gSystem->getEventDispatcher()->post(eEventPreRender)
}

*/
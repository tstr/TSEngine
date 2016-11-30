/*
	Event system
*/

#pragma once

#include <tsengine/abi.h>
#include <tscore/types.h>
#include <tscore/containers/threadqueue.h>

namespace ts
{
	template<typename Message_t>
	class CMessageReciever
	{
	public:

		//Enqueue a message
		inline void post(const Message_t& msg)
		{
			m_messageQueue.push(msg);
		}

		//Read a message off of the queue, if the queue is empty the method waits until a message is posted
		inline void get(Message_t& msg)
		{
			msg = m_messageQueue.pop();
		}

		//Read a message off of the queue, the method exits if the queue is empty
		inline void peek(Message_t& msg)
		{
			msg = m_messageQueue.peek();
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
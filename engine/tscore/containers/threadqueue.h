/*
	Thread safe queue container	
*/

#pragma once

#include <tscore/types.h>

#include <mutex>
#include <thread>
#include <queue>

namespace ts
{
	template<typename type>
	class ThreadQueue
	{
	private:

		std::queue<type> m_queue;
		std::mutex m_mutex;
		std::condition_variable m_notifier;

	public:

		type pop()
		{
			std::unique_lock<std::mutex> lk(m_mutex);

			while (m_queue.empty())
				m_notifier.wait(lk);

			type val(std::move(m_queue.front()));
			m_queue.pop();
			return std::move(val);
		}

		void push(const type& val)
		{
			std::unique_lock<std::mutex> lk(m_mutex);
			m_queue.push(val);
			lk.unlock();
			m_notifier.notify_all();
		}

		void push(type&& val)
		{
			std::unique_lock<std::mutex> lk(m_mutex);
			m_queue.push(val);
			lk.unlock();
			m_notifier.notify_all();
		}
	};
}
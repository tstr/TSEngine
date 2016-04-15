/*
	Threading header - provides useful multithreading classes
*/

#pragma once

#include <C3E\Core\corecommon.h>
#include <C3E\Core\time.h>
#include <C3E\Core\memory.h>

#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>
#include <queue>

//Thread Local Storage specifier for variables
#define THREAD_LOCAL __declspec(thread)

namespace C3E
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////
	//STL threading primitives
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	using std::mutex;
	using std::condition_variable;
	using std::atomic;
	using std::thread;
	using std::lock_guard;
	using std::unique_lock;

	///////////////////////////////////////////////////////////////////////////////////////////////////////

	class BasicRoutine
	{
	private:

		atomic<bool> m_active = false;
		atomic<int> m_count = 0;
		mutex m_mutex;

	public:

		BasicRoutine() : m_active(false) {}
		BasicRoutine(const BasicRoutine&) = delete;

		inline int lock()
		{
			if (!m_active) return 0;

			if (m_count == 0)
				m_mutex.lock();
			return ++m_count;
		}

		inline int unlock()
		{
			if (!m_active) return 0;

			int x = --m_count;
			if (x == 0)
				m_mutex.unlock();
			return x;
		}

		inline void force_unlock() { m_mutex.unlock(); m_count = 0; }
		inline void force_lock() { m_mutex.lock(); m_count = 1; }

		template<typename fnc_t> inline
			void start(fnc_t f)
		{
			m_active = true;

			while (true)
			{
				lock_guard<mutex> lk(m_mutex);
				if (!f()) return;
			}

			m_active = false;
		}
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////

	//Thread safe queue container
	template<typename type>
	class ThreadQueue
	{
	private:

		std::queue<type> m_queue;
		mutex m_mutex;
		condition_variable m_notifier;

	public:

		type pop()
		{
			unique_lock<mutex> lk(m_mutex);

			while (m_queue.empty())
				m_notifier.wait(lk);

			type val(move(m_queue.front()));
			m_queue.pop();
			return move(val);
		}

		void push(const type& val)
		{
			unique_lock<mutex> lk(m_mutex);
			m_queue.push(val);
			lk.unlock();
			m_notifier.notify_all();
		}

		void push(type&& val)
		{
			unique_lock<mutex> lk(m_mutex);
			m_queue.push(val);
			lk.unlock();
			m_notifier.notify_all();
		}
	};

	class ThreadPool
	{
	public:

		struct ITask
		{
			virtual void execute() = 0;
		};

		ThreadPool() : ThreadPool((int)thread::hardware_concurrency()) {}

		ThreadPool(int num_threads)
		{
			for (int i = 0; i < num_threads; i++)
			{
				m_threads.push_back(thread(&ThreadPool::procedure, this, i));
			}
		}

		explicit ThreadPool(const ThreadPool&) = delete;

		~ThreadPool()
		{
			for (size_t i = 0; i < m_threads.size(); i++)
			{
				m_queue.push(std::make_pair(false, nullptr));
			}

			for (auto& t : m_threads)
				t.join();
		}

		void add_task(ITask& task)
		{
			m_queue.push(std::make_pair(true, &task));
		}

		void operator+=(ITask& task)
		{
			return add_task(task);
		}

	private:

		typedef std::pair<bool, ITask*> package_t;

		std::vector<thread> m_threads;
		ThreadQueue<package_t> m_queue;

		void procedure(int thread_index)
		{
			while (true)
			{
				package_t pack = m_queue.pop();

				if (!pack.first)
					break;

				pack.second->execute();
			}
		}
	};

	template<typename fnc_t>
	class GenericTask : public ThreadPool::ITask
	{
	private:

		fnc_t m_f;

	public:

		GenericTask(fnc_t _f) : m_f(_f) {}
		explicit GenericTask(const GenericTask& copy) : m_f(copy.m_f) {}
		GenericTask& operator=(const GenericTask& copy) { m_f(copy.m_f); }

		void execute()
		{
			f();
		}
	};

	template<typename fnc_t>
	GenericTask<fnc_t> make_task(fnc_t f)
	{
		return GenericTask<fnc_t>(f);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
}
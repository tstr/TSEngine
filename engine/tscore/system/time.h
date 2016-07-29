/*
	High precision timers
*/

#pragma once

#include <tsconfig.h>

#ifdef TS_PLATFORM_WIN32

#include <Windows.h>
#include <ctime>

namespace ts
{
	/////////////////////////////////////////////////////////////////////////////////////////////

	typedef LARGE_INTEGER time64;

	namespace internal
	{
		class BaseTime
		{
		protected:

			time64 frequency;

		public:

			BaseTime() { updateFrequency(); }

			//Recalculate the CPU frequency value
			inline void updateFrequency()
			{
				QueryPerformanceFrequency(&frequency);
			}
		};
	}

	/////////////////////////////////////////////////////////////////////////////////////////////

	class Timer : public internal::BaseTime
	{
	private:

		time64 ut, vt, dt;

	public:

		Timer() : BaseTime() {}
		~Timer() {}

		//Reset counter
		inline void tick()
		{
			QueryPerformanceCounter(&vt);
			dt.QuadPart = vt.QuadPart - ut.QuadPart;
			ut.QuadPart = vt.QuadPart;
		}

		//Retrieves delta time between ticks in seconds
		inline double deltaTime()
		{
			return ((double)dt.QuadPart / (double)frequency.QuadPart);
		}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////

	class Stopwatch : public internal::BaseTime
	{
	private:

		time64 ut, vt;

	public:

		Stopwatch() : BaseTime()
		{
			
		}

		~Stopwatch() {}

		inline void start()
		{
			memset(&vt, 0, sizeof(vt));
			QueryPerformanceCounter(&ut);
		}

		inline void stop()
		{
			QueryPerformanceCounter(&vt);
		}

		//Retrieves delta time between ticks in milliseconds
		inline double deltaTime()
		{
			return ((1000.0 * (double)(vt.QuadPart - ut.QuadPart)) / (double)frequency.QuadPart);
		}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////
}

#endif
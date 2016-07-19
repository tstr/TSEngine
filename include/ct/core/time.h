/*
	High precision timers
*/

#pragma once

#include <ct\core\corecommon.h>
#include <Windows.h>
#include <ctime>

namespace CT
{
	/////////////////////////////////////////////////////////////////////////////////////////////

	typedef LARGE_INTEGER time64;

	namespace Internal
	{
		class BaseTime
		{
		protected:

			time64 frequency;

		public:

			BaseTime() { UpdateFrequency(); }

			//Recalculate the CPU frequency value
			inline void UpdateFrequency()
			{
				QueryPerformanceFrequency(&frequency);
			}
		};
	}

	/////////////////////////////////////////////////////////////////////////////////////////////

	class Timer : public Internal::BaseTime
	{
	private:

		time64 ut, vt, dt;

	public:

		Timer() : BaseTime() {}
		~Timer() {}

		//Reset counter
		inline void Tick()
		{
			QueryPerformanceCounter(&vt);
			dt.QuadPart = vt.QuadPart - ut.QuadPart;
			ut.QuadPart = vt.QuadPart;
		}

		//Retrieves delta time between ticks in seconds
		inline double DeltaTime()
		{
			return ((double)dt.QuadPart / (double)frequency.QuadPart);
		}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////

	class Stopwatch
	{
	private:

		typedef LARGE_INTEGER time64;

		time64 ut, vt,
			frequency;

	public:

		Stopwatch()
		{
			UpdateFrequency();
		}

		~Stopwatch() {}

		inline void Start()
		{
			memset(&vt, 0, sizeof(vt));
			QueryPerformanceCounter(&ut);
		}

		inline void Stop()
		{
			QueryPerformanceCounter(&vt);
		}

		//Retrieves delta time between ticks in milliseconds
		inline double DeltaTime()
		{
			return ((1000.0 * (double)(vt.QuadPart - ut.QuadPart)) / (double)frequency.QuadPart);
		}

		//Recalculate the CPU frequency value
		inline void UpdateFrequency()
		{
			QueryPerformanceFrequency(&frequency);
		}
	};

	/////////////////////////////////////////////////////////////////////////////////////////////
}
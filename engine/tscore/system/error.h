/*
	Exception handling and memory leak handling functions - these functions are only meant to be called on startup and in no other place
*/

#pragma once

#include <tscore/internal/tscoreapi.h>

namespace ts
{
	namespace internal
	{
		void TSCORE_API initializeSystemExceptionHandlingFilter();

		void TSCORE_API initializeSystemMemoryLeakDetector();
	}
}
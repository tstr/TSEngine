/*
	Exception handling and memory leak handling functions - these functions are only meant to be called on startup and in no other place
*/

#pragma once

#include <tsconfig.h>

namespace ts
{
	namespace internal
	{
		void initializeSystemExceptionHandlingFilter();

		void initializeSystemMemoryLeakDetector();
	}
}
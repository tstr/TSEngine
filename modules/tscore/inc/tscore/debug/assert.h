/*
	Assertion macro
*/

#pragma once

#include <tscore/abi.h>

#include <exception>

namespace ts
{
	namespace internal
	{
		void TSCORE_API _internal_assert(
			const char* file,
			const char* func,
			const char* expr,
			int line,
			bool assert
		);
	}
}

#define tsassert(expression) ts::internal::_internal_assert(__FILE__, __FUNCTION__, #expression, __LINE__, !!(expression))

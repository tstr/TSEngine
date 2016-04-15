/*
	Assertion macro
*/

#pragma once

#include <C3E\core\corecommon.h>
#include <exception>

namespace C3E
{
	namespace Internal
	{
		C3E_CORE_API void _internal_assert(
			const char* file,
			const char* func,
			const char* expr,
			int line,
			bool a
		);
	}
}

#define C3E_ASSERT(expression) C3E::Internal::_internal_assert(__FILE__, __FUNCTION__, #expression, __LINE__, !!(expression))
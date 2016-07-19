/*
	Assertion macro
*/

#pragma once

#include <ct\core\corecommon.h>
#include <exception>

namespace CT
{
	namespace Internal
	{
		CT_CORE_API void _internal_assert(
			const char* file,
			const char* func,
			const char* expr,
			int line,
			bool a
		);
	}
}

#define CT_ASSERT(expression) CT::Internal::_internal_assert(__FILE__, __FUNCTION__, #expression, __LINE__, !!(expression))
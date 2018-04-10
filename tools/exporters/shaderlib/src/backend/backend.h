/*
	Shader compiler backend
*/

#pragma once

#include "../common.h"
#include <tscore/system/memory.h>

namespace ts
{
	enum EShaderBackend : ts::uint8
	{
		eBackendHLSL_SM5 = 1,
		eBackendHLSL_SM5_1 = 2,
		eBackendHLSL_SM6 = 3,
		eBackendSPIRV = 4,
	};

	class IShaderBackend
	{
	public:

		virtual EShaderBackend getId() const = 0;
		virtual bool compile(const std::string& code, ts::MemoryBuffer& codebuffer, const char* entrypoint, EShaderStage stage) = 0;
	};
}

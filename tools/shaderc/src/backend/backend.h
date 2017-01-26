/*
	Shader compiler backend
*/

#pragma once

#include "../compiler.h"

enum EShaderBackend
{
	eBackendHLSL_SM5 = 1,
	//eBackendHLSL_SM5_1 = 2,
	//eBackendHLSL_SM6 = 3,
	//eBackendSPIRV	 = 4,
};

class IShaderBackend
{
public:

	virtual ts::uint16 getId() const = 0;
	virtual bool compile(const std::string& code, ts::MemoryBuffer& codebuffer, const char* entrypoint, ts::EShaderStage stage) = 0;
};

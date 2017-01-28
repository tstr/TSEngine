/*
	HLSL shader compiler implementation
*/

#pragma once

#include "backend.h"

#include <string>
#include <tscore/system/memory.h>

/////////////////////////////////////////////////////////////////////////////////////////////////

class HLSLCompiler : public IShaderBackend
{
private:

	void* m_module = nullptr;

public:

	HLSLCompiler();
	~HLSLCompiler();
	
	EShaderBackend getId() const override { return eBackendHLSL_SM5; }
	bool compile(const std::string& code, ts::MemoryBuffer& codebuffer, const char* entrypoint, EShaderStage stage) override;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

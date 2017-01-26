/*
	Shader Compiler Class
*/

#pragma once

#include "common.h"
#include "preprocessor.h"

#include <tscore/filesystem/path.h>
#include <tscore/system/memory.h>
#include <tsgraphics/api/renderapi.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class CShaderCompileEngine
{
private:

	struct Impl;
	Impl* pImpl = nullptr;

public:

	CShaderCompileEngine() :
		pImpl(nullptr)
	{}

	CShaderCompileEngine(
		const ts::Path& srcdir,
		const ts::Path& outdir
	);

	~CShaderCompileEngine();

	CShaderCompileEngine(const CShaderCompileEngine&) = delete;
	CShaderCompileEngine(CShaderCompileEngine&& shce)
	{
		Impl* i = this->pImpl;
		this->pImpl = shce.pImpl;
		shce.pImpl = i;
	}

	int compileShader(const char* name, const SShaderInfo& shaderInfo);
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Shader manager header
*/

#pragma once

#include "rendercommon.h"
#include "renderapi.h"

#include <tscore/filesystem/path.h>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	class CRenderModule;
	class CShaderManager;

	class CShader
	{
	private:

		CShaderManager* m_manager = nullptr;
		ResourceProxy m_shader;

	public:

		CShader() {}
		CShader(
			CShaderManager* manager,
			const MemoryBuffer& bytecode,
			EShaderStage stage
		);
		~CShader();

		ResourceProxy getShader() const { return m_shader; }
	};

	class CShaderManager
	{
	private:

		IShaderCompiler* m_shaderCompiler = nullptr;
		CRenderModule* m_renderModule = nullptr;
		Path m_rootpath;

	public:

		CShaderManager(CRenderModule* module, const Path& rootpath = "");
		~CShaderManager();

		CRenderModule* const getModule() const { return m_renderModule; }

		void setRootpath(const Path& rootpath) { m_rootpath = rootpath; }

		bool compileAndLoadShader(CShader& shader, const char* code, const SShaderCompileConfig& config);
		bool compileAndLoadShaderFile(CShader& shader, const Path& codefile, const SShaderCompileConfig& config);
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////
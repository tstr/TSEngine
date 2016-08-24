/*
	Render API

	DirectX 11 implementation of shader compiler
*/

#include "DX11render.h"
#include <d3dcompiler.h>

using namespace std;
using namespace ts;
using namespace ts::dx11;

/////////////////////////////////////////////////////////////////////////////////////////////////

DX11ShaderCompiler::DX11ShaderCompiler()
{
	char lib[] = "d3dcompiler_47.dll";
	m_d3dcompiler = LoadLibraryA(lib);

	if (!m_d3dcompiler)
	{
		tswarn("unable to load compiler '%'", lib);
		return;
	}
}

DX11ShaderCompiler::~DX11ShaderCompiler()
{
	if (m_d3dcompiler)
		FreeLibrary(m_d3dcompiler);
}

/////////////////////////////////////////////////////////////////////////////////////////////////


bool DX11ShaderCompiler::compile(const char* str, const SShaderCompileConfig& options, MemoryBuffer& bytecode)
{
	return false;
}


bool DX11ShaderCompiler::compileFromFile(const Path& file, const SShaderCompileConfig& options, MemoryBuffer& bytecode)
{
	return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////
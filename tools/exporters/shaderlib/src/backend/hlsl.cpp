/*
	Hlsl compiler implementation
*/

#include "hlsl.h"

#include <Windows.h>
#include <d3dcompiler.h>
#include <d3d11.h>
#include <comdef.h>
#include <wrl/client.h>
#include <iostream>
#include <cassert>

template<typename t>
using ComPtr = Microsoft::WRL::ComPtr<t>;

using namespace std;
using namespace ts;

#define LOAD_FUNC(h, x) (decltype(x)*)GetProcAddress(h, #x);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class ShaderModel
{
private:

	char stage[2];
	char r0 = '_';
	char major = 'x';
	char r1 = '_';
	char minor = 'x';
	char null = 0; //null terminating character

public:

	inline void setStage(char a[2]) { memcpy(stage, a, sizeof(char[2])); }

	inline void setTargetMajor(char c) { major = c; }
	inline void setTargetMinor(char c) { minor = c; }

	inline const char* tostring()
	{
		r0 = '_';
		r1 = '_';
		return reinterpret_cast<const char*>(this);
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HLSLCompiler::HLSLCompiler()
{
	char lib[] = D3DCOMPILER_DLL_A;
	m_module = LoadLibraryA(lib);

	assert(m_module);
}

HLSLCompiler::~HLSLCompiler()
{
	if (m_module)
		FreeLibrary((HMODULE)m_module);
}

bool HLSLCompiler::compile(const std::string& code, MemoryBuffer& codebuffer, const char* entrypoint, EShaderStage stage, std::string& errorbuffer)
{
	auto fn_compile = LOAD_FUNC((HMODULE)m_module, D3DCompile);

	if (!fn_compile)
		return false;

	UINT compileflag = 0;

	bool debug = false;
	if (debug)
	{
		compileflag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
	}
	else
	{
		compileflag = D3DCOMPILE_OPTIMIZATION_LEVEL3;
	}

	ShaderModel shadertarget;
	shadertarget.setTargetMajor('5');
	shadertarget.setTargetMinor('0');

	switch (stage)
	{
	case eShaderStageVertex:
		shadertarget.setStage("vs");
		break;
	case eShaderStagePixel:
		shadertarget.setStage("ps");
		break;
	case eShaderStageGeometry:
		shadertarget.setStage("gs");
		break;
	case eShaderStageTessCtrl:
		shadertarget.setStage("hs");
		break;
	case eShaderStageTessEval:
		shadertarget.setStage("ds");
		break;
	case eShaderStageCompute:
		shadertarget.setStage("cs");
		break;
	}

	ComPtr<ID3DBlob> bytecode;
	ComPtr<ID3DBlob> errors;

	HRESULT hr = fn_compile(
		code.c_str(),
		code.size(),
		nullptr,
		0,
		nullptr,
		entrypoint,
		shadertarget.tostring(),
		compileflag,
		0,
		bytecode.GetAddressOf(),
		errors.GetAddressOf()
	);

	if (FAILED(hr))
	{
		cerr << ((const char*)errors->GetBufferPointer());
		return false;
	}

	codebuffer = MemoryBuffer(bytecode->GetBufferPointer(), bytecode->GetBufferSize());
	errorbuffer = std::string((const char*)errors->GetBufferPointer(), (const char*)errors->GetBufferPointer() + errors->GetBufferSize());

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

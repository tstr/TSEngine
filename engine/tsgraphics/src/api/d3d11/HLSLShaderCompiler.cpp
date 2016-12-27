/*
	Render API

	DirectX 11 implementation of shader compiler
*/

#include "Render.h"

#include <Windows.h>
#include <d3dcompiler.h>

#include <iostream>

using namespace std;
using namespace ts;

#define LOAD_FUNC(h, x) (decltype(x)*)GetProcAddress(h, #x);

/////////////////////////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////////////////////////

class D3D11ShaderCompiler : public ts::IShaderCompiler
{
private:

	HMODULE m_d3dcompiler;

public:

	D3D11ShaderCompiler()
	{
		char lib[] = D3DCOMPILER_DLL_A;
		m_d3dcompiler = LoadLibraryA(lib);

		if (!m_d3dcompiler)
		{
			tswarn("unable to load compiler '%'", lib);
			return;
		}
	}
	
	~D3D11ShaderCompiler()
	{
		if (m_d3dcompiler)
			FreeLibrary(m_d3dcompiler);
	}
	
	bool compile(const char* code, const SShaderCompileConfig& options, MemoryBuffer& codebuffer) override
	{
		auto fn_compile = LOAD_FUNC(m_d3dcompiler, D3DCompile);
		
		if (!fn_compile)
			return false;
		
		UINT compileflag = 0;

		if (options.debuginfo)
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
		
		switch(options.stage)
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
			case eShaderStageHull:
				shadertarget.setStage("hs");
				break;
			case eShaderStageDomain:
				shadertarget.setStage("ds");
				break;
			case eShaderStageCompute:
				shadertarget.setStage("cs");
				break;
		}
		
		ComPtr<ID3DBlob> bytecode;
		ComPtr<ID3DBlob> errors;
		
		HRESULT hr = fn_compile(
			code,
			strlen(code),
			nullptr,
			0,
			nullptr,
			options.entrypoint,
			shadertarget.tostring(),
			compileflag,
			0,
			bytecode.GetAddressOf(),
			errors.GetAddressOf()
		);
		
		if (FAILED(hr))
		{
			tserror((const char*)errors->GetBufferPointer());
			
			return false;
		}
		
		codebuffer = MemoryBuffer(bytecode->GetBufferPointer(), bytecode->GetBufferSize());
		
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	namespace abi
	{
		int createShaderCompiler(IShaderCompiler** compiler)
		{
			*compiler = new D3D11ShaderCompiler();
			return 0;
		}

		void destroyShaderCompiler(IShaderCompiler* compiler)
		{
			if (auto sc = dynamic_cast<D3D11ShaderCompiler*>(compiler))
			{
				delete sc;
			}
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////

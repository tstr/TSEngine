/*
	Effect compiler
*/

#include "pch.h"

#include <C3E\gfx\abi\graphicsabi.h>
#include <C3E\core\console.h>

#include <d3dcompiler.h>
#include "DX11ShaderCompiler.h"

#include <sstream>
#include <algorithm>
#include <fstream>

using namespace std;
using namespace C3E;
using namespace C3E::DX11;

#define LOAD_FUNC(h, x) (decltype(x)*)GetProcAddress(h, #x);

////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace C3E
{
	namespace ABI
	{
		extern "C"
		{
			EXPORT_ATTRIB bool CompileShaderCode(
				const char* sourcefile,
				const char* shadercode,
				size_t shadercode_size,
				std::ostream& output,
				std::ostream& erroutput,
				size_t& size,
				const char* entrypoint,
				ShaderType type,
				int flags
			)
			{
				ShaderCacheHeader header;
				ZeroMemory(&header, sizeof(ShaderCacheHeader));

				if (output.fail())
				{
					return false;
				}

				HMODULE hmodule = LoadLibraryA(D3DCOMPILER_DLL_A);
				if (!hmodule) return false;

				auto f_compile = LOAD_FUNC(hmodule, D3DCompile);
				if (!f_compile) return false;

				{
					stringstream bytecodestream(stringstream::in | stringstream::out);

					uint32 sflag = 0;

					if (flags & ShaderCompileFlag::SHADER_COMPILE_DEBUG)
					{
						sflag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
					}
					else
					{
						sflag = D3DCOMPILE_OPTIMIZATION_LEVEL3;
					}

					ShaderModel shadertarget;
					ShaderType shadertype;
					shadertarget.SetTargetMajor('5');
					shadertarget.SetTargetMinor('0');

					if (type == ShaderType::SHADER_TYPE_UNKNOWN)
					{
						return false;
					}
					else
					{
						switch (type)
						{
							case (ShaderType::SHADER_TYPE_VERTEX) :
							{
								shadertarget.SetStage("vs");
								shadertype = ShaderType::SHADER_TYPE_VERTEX;
								break;
							}
							case (ShaderType::SHADER_TYPE_PIXEL) :
							{
								shadertarget.SetStage("ps");
								shadertype = ShaderType::SHADER_TYPE_PIXEL;
								break;
							}
							case (ShaderType::SHADER_TYPE_GEOMETRY) :
							{
								shadertarget.SetStage("gs");
								shadertype = ShaderType::SHADER_TYPE_GEOMETRY;
								break;
							}
							case (ShaderType::SHADER_TYPE_COMPUTE) :
							{
								shadertarget.SetStage("cs");
								shadertype = ShaderType::SHADER_TYPE_COMPUTE;
								break;
							}
							default:
							{
								return false;
							}
						}
					}

					ComPtr<ID3DBlob> bytecode;
					ComPtr<ID3DBlob> errors;

					HRESULT hr = f_compile(
						shadercode,
						shadercode_size,
						sourcefile,
						0,
						D3D_COMPILE_STANDARD_FILE_INCLUDE,
						entrypoint,
						shadertarget.tostring(),
						sflag,
						0,
						bytecode.GetAddressOf(),
						errors.GetAddressOf()
					);

					if (FAILED(hr))
					{
						erroutput << "D3D11 Shader compiler error: 0x" << hex << hr << " " << GetMessageForHRESULT(hr);
						erroutput << endl << (const char*)errors->GetBufferPointer();

						return false;
					}

					output.write(reinterpret_cast<const char*>(bytecode->GetBufferPointer()), bytecode->GetBufferSize());

					if (bytecodestream.fail())
					{
						cout << "D3D11 Effect compiler error: stream could not be written\n";
						return false;
					}

					size = bytecode->GetBufferSize();

					/*
					header.model = shadertarget;
					header.type = shadertype;
					header.size = (uint32)bytecode->GetBufferSize();
					strcpy_s(header.entrypoint, entrypoint);
					*/

					//output.write(reinterpret_cast<const char*>(&header), sizeof(header));
				}

				//For the moment do not free the library because this function is called multiple times normally
				//FreeLibrary(hmodule);

				return output.good();
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
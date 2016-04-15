/*
	Library loading class
*/

#pragma once

#include <C3Ext\win32.h>
#include <C3E\core\corecommon.h>
#include <C3E\core\memory.h>
#include <C3E\gfx\graphics.h>
#include <C3E\gfx\abi\graphicsabi.h>

#include <string>
#include <map>

namespace C3E
{
	class GraphicsBackend
	{
	public:

		typedef decltype(ABI::CreateRenderingInterface)* fcreate_t;
		typedef decltype(ABI::DestroyRenderingInterface)* fdestroy_t;
		typedef decltype(ABI::CompileShaderCode)* fcompile_t;

		GraphicsBackend() {}
		GraphicsBackend(Graphics::API api) { load(api); }

		~GraphicsBackend()
		{
			FreeLibrary(m_module);
		}

		void load(Graphics::API api)
		{
			switch (api)
			{
			case(Graphics::API_D3D11) :
			{
				internal_load("C3EGraphicsD3D11.dll");
				break;
			}
			case (Graphics::API_D3D12) :
			{
				internal_load("C3EGraphicsD3D12.dll");
				break;
			}
			default:
			{

			}
			}
		}

		fcreate_t f_create = nullptr;
		fdestroy_t f_destroy = nullptr;
		fcompile_t f_compile = nullptr;

	private:

		void internal_load(const char* lib)
		{
			C3E_ASSERT(m_module = LoadLibraryA(lib));

			f_create = (fcreate_t)GetProcAddress(m_module, "CreateRenderingInterface");
			f_destroy = (fdestroy_t)GetProcAddress(m_module, "DestroyRenderingInterface");
			f_compile = (fcompile_t)GetProcAddress(m_module, "CompileShaderCode");

			C3E_ASSERT(f_create);
			C3E_ASSERT(f_destroy);
			C3E_ASSERT(f_compile);
		}

		HMODULE m_module = 0;
	};
}
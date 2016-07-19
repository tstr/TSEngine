/*
Graphics helpers
*/

#pragma once

#include <d3d11_1.h>
#include <exception>

#include <CT\core\memory.h>

#include <wrl\client.h>
#include <comdef.h>

//////////////////////////////////////////////////////////////////////////////////////////////

#define IID_OF(i) __uuidof(i)

namespace CT
{
	namespace DX11
	{
		using Microsoft::WRL::ComPtr;

		typedef std::exception GraphicsException;

		inline void ThrowIf(HRESULT hr)
		{
			if (FAILED(hr))
				throw GraphicsException("");
		}

		inline void SetDebugObjectName(ID3D11DeviceChild* child, const char* name)
		{
			//using namespace DirectX;

#ifdef _DEBUG

			if (child != nullptr && name != nullptr)
				D3D_SET_OBJECT_NAME_A(child, name);

#endif

		}

		inline const TCHAR* GetMessageForHRESULT(HRESULT hr)
		{
			_com_error error(hr);
			return error.ErrorMessage();
		}

		static DXGI_FORMAT TextureFormatToDXGIFormat(ETextureFormat f)
		{
			DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
			switch (f)
			{
			case (ETextureFormat::FormatByte) : { format = DXGI_FORMAT_R8_UNORM; break; }
			case (ETextureFormat::FormatRGBA8_INT) : { format = DXGI_FORMAT_R8G8B8A8_UNORM; break; }
			case (ETextureFormat::FormatBGRA8_INT) : { format = DXGI_FORMAT_B8G8R8A8_UNORM; break; }
			case (ETextureFormat::FormatRGBA32_FLOAT) : { format = DXGI_FORMAT_R32G32B32A32_FLOAT; break; }
			case (ETextureFormat::FormatRGB32_FLOAT) : { format = DXGI_FORMAT_R32G32B32_FLOAT; break; }
			case (ETextureFormat::FormatRG32_FLOAT) : { format = DXGI_FORMAT_R32G32_FLOAT; break; }
			}

			return format;
		}


		//using namespace DirectX;

		static uint32 GetNumMipLevels(uint32 width, uint32 height)
		{
			uint32 numLevels = 1;
			while (width > 1 && height > 1)
			{
				width = std::max(width / 2, 1u);
				height = std::max(height / 2, 1u);
				++numLevels;
			}

			return numLevels;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
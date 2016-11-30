/*
	Helper functions
*/

#include "DX11helpers.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	namespace dx11
	{
		DXGI_FORMAT TextureFormatToDXGIFormat(ETextureFormat format)
		{
			switch (format)
			{
			case(eTextureFormatByte):
				return DXGI_FORMAT_R8_UINT;

			case(eTextureFormatColourARGB):
				return DXGI_FORMAT_B8G8R8A8_UNORM;
			case(eTextureFormatColourRGB):
			case(eTextureFormatColourRGBA):
				return DXGI_FORMAT_R8G8B8A8_UNORM;

			case(eTextureFormatFloat1):
				return DXGI_FORMAT_R32_FLOAT;
			case(eTextureFormatFloat2):
				return DXGI_FORMAT_R32G32_FLOAT;
			case (eTextureFormatFloat3):
				return DXGI_FORMAT_R32G32B32A32_FLOAT;
				//return DXGI_FORMAT_R32G32B32_FLOAT;
			case(eTextureFormatFloat4):
				return DXGI_FORMAT_R32G32B32A32_FLOAT;

			case(eTextureFormatDepth16):
				return DXGI_FORMAT_D16_UNORM;
			case(eTextureFormatDepth32):
				return DXGI_FORMAT_D32_FLOAT;
			}
			
			return DXGI_FORMAT_UNKNOWN;
		}
		
		ERenderStatus RenderStatusFromHRESULT(HRESULT hr)
		{
			if (SUCCEEDED(hr))
				return ERenderStatus::eOk;

			return ERenderStatus::eFail;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
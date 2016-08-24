/*
	Helper functions
*/

#pragma once

#include <tsgraphics/renderapi.h>
#include "DX11base.h"

namespace ts
{
	namespace dx11
	{
		DXGI_FORMAT TextureFormatToDXGIFormat(ETextureFormat format);
		
		ERenderStatus RenderStatusFromHRESULT(HRESULT hr);

		inline uint32 getNumMipLevels(uint32 width, uint32 height)
		{
			using namespace std;

			uint32 numLevels = 1;
			while (width > 1 && height > 1)
			{
				width = max(width / 2, 1u);
				height = max(height / 2, 1u);
				++numLevels;
			}

			return numLevels;
		}
	}
}
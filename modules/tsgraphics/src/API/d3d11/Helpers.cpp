/*
	Helper functions
*/

#include "helpers.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	DXGI_FORMAT ImageFormatToDXGIFormat(ImageFormat format)
	{
		switch (format)
		{
		case(ImageFormat::BYTE):
			return DXGI_FORMAT_R8_UINT;

		case(ImageFormat::ARGB):
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		case(ImageFormat::RGB):
		case(ImageFormat::RGBA):
			return DXGI_FORMAT_R8G8B8A8_UNORM;

		case(ImageFormat::FLOAT1):
			return DXGI_FORMAT_R32_FLOAT;
		case(ImageFormat::FLOAT2):
			return DXGI_FORMAT_R32G32_FLOAT;
		case (ImageFormat::FLOAT3):
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
			//return DXGI_FORMAT_R32G32B32_FLOAT;
		case(ImageFormat::FLOAT4):
			return DXGI_FORMAT_R32G32B32A32_FLOAT;

		case(ImageFormat::DEPTH16):
			return DXGI_FORMAT_D16_UNORM;
		case(ImageFormat::DEPTH32):
			return DXGI_FORMAT_D32_FLOAT;
		}
			
		return DXGI_FORMAT_UNKNOWN;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Base graphics header
*/

#pragma once

#include <ct\core\corecommon.h>

#ifdef CT_USE_DYNAMIC_LIB

#ifndef CT_GFX_API
#define CT_GFX_API IMPORT_ATTRIB
#endif

#else

#ifndef CT_GFX_API
#define CT_GFX_API
#endif

#endif

namespace C3E
{
	namespace ABI
	{
		enum ResourceLimits
		{
			MAX_SHADER_BUFFERS = 16,
			MAX_VERTEX_BUFFERS = 8,

			MAX_SRVS = 16,
			MAX_UAVS = 8
		};

		enum ShaderType
		{
			SHADER_TYPE_UNKNOWN = 0,
			SHADER_TYPE_VERTEX = 1,
			SHADER_TYPE_PIXEL = 2,
			SHADER_TYPE_GEOMETRY = 4,
			SHADER_TYPE_COMPUTE = 8
		};
	}

	typedef void* ResourceHandle;
	typedef ResourceHandle ResourceViewHandle;

	enum EResourceType
	{
		TypeUnknown,
		TypeVertexBuffer,
		TypeIndexBuffer,
		TypeShaderBuffer,
		TypeTexture
	};

	enum EResourceUsage
	{
		UsageDefault, //Only accessible from GPU
		UsageDynamic, //Accessible from GPU and CPU
		UsageStaging,
		UsageStatic   //Is not modifiable
	};

	enum ETextureFlags
	{
		TexFlagUnknown = 0,
		TexFlagMipmaps = 1,
		TexFlagCubemap = 2
	};

	enum ETextureUsage
	{
		TexUsageUnknown = 0,
		TexUsageShaderResource = 1,
		TexUsageDepthBuffer = 2,
		TexUsageRenderTarget = 4
	};

	enum ETextureType
	{
		TypeTexture1D,
		TypeTexture2D,
		TypeTexture3D
	};

	enum ETextureFormat
	{
		FormatUnknown,
		FormatByte,
		FormatBGRA8_INT, //Alias for ARGB format
		FormatRGBA8_INT,

		FormatRGBA32_FLOAT,
		FormatRGB32_FLOAT,
		FormatRG32_FLOAT
	};

	enum ETextureSampler
	{
		SamplerUnknown,
		SamplerLinear,
		SamplerPoint,
		SamplerAnisotropic2x,
		SamplerAnisotropic4x,
		SamplerAnisotropic8x,
		SamplerAnisotropic16x
	};
}
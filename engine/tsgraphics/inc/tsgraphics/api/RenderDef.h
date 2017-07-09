/*
	Render Module Common header
	
	Defines common types shared between high level module and low level rendering implementation
*/

#pragma once

#include <tscore/types.h>
#include <tscore/maths.h>
#include <tscore/strings.h>
#include <tsgraphics/colour.h>

namespace ts
{
	struct IRender;
	struct IRenderContext;

	///////////////////////////////////////////////////////////////////////////////////////////
	// Handle types
	///////////////////////////////////////////////////////////////////////////////////////////
	
	//Resource handles
	enum HBuffer : intptr { HBUFFER_NULL = 0 };
	enum HTexture : intptr { HTEXTURE_NULL = 0 };
	enum HTarget : intptr { HTARGET_NULL = 0 };
	enum HShader : intptr { HSHADER_NULL = 0 };
	enum HDrawCmd : intptr { HDRAWCMD_NULL = 0 };
	
	///////////////////////////////////////////////////////////////////////////////////////////
	// Core structures/enums
	///////////////////////////////////////////////////////////////////////////////////////////
	
	enum ERenderStatus
	{
		eOk = 0,
		eFail,
		eInvalidParameter,
		eInvalidResource,
		eInvalidTextureFormat,
		eInvalidShaderByteCode
	};
	
	struct SDisplayConfig
	{
		uint16 resolutionW = 0;
		uint16 resolutionH = 0;
		uint8 multisampleLevel = 0;
		bool fullscreen = false;
	};
	
	enum ERenderApiFlags : uint8
	{
		eFlagNull		   = 0,
		eFlagDebug		   = 1,
		eFlagReportObjects = 2
	};

	struct SRenderApiConfig
	{
		intptr windowHandle = 0;
		uint32 adapterIndex = 0;
		SDisplayConfig display;
		uint8 flags = 0;
	};

	struct SRenderStatistics
	{
		uint32 drawcalls = 0;
	};
	
	struct SRenderAdapterDesc
	{
		StaticString<128> adapterName;
		uint64 gpuVideoMemory;		//GPU accessible video memory capacity
		uint64 gpuSystemMemory;		//GPU accessible system memory capacity
		uint64 sharedSystemMemory;	//GPU/CPU accessible system memory capacity
	};
	
	struct SViewport
	{
		uint32 w = 0; //width
		uint32 h = 0; //height
		uint32 x = 0; //x offset
		uint32 y = 0; //y offset

		SViewport() {}
		SViewport(uint32 _w, uint32 _h, uint32 _x, uint32 _y) :
			w(_w),
			h(_h),
			x(_x),
			y(_y)
		{}
	};
	
	struct SMultisampling
	{
		uint32 count = 1;

		SMultisampling() {}
		SMultisampling(uint32 c) : count(c) {}
	};
	
	///////////////////////////////////////////////////////////////////////////////////////////
	// Buffers
	///////////////////////////////////////////////////////////////////////////////////////////
	
	enum EBufferType
	{
		eBufferTypeUnknown,
		eBufferTypeVertex,
		eBufferTypeIndex,
		eBufferTypeConstant
	};

	struct SBufferResourceData
	{
		const void* memory = nullptr;
		uint32 size = 0;
		EBufferType usage = eBufferTypeUnknown;
	};

	///////////////////////////////////////////////////////////////////////////////////////////
	// Textures
	///////////////////////////////////////////////////////////////////////////////////////////

	enum ETextureFormat
	{
		eTextureFormatUnknown,

		eTextureFormatByte,

		//8 bits per channel - int
		eTextureFormatColourRGB,
		eTextureFormatColourRGBA,
		eTextureFormatColourARGB,

		//32 bits per channel - float
		eTextureFormatFloat1,
		eTextureFormatFloat2,
		eTextureFormatFloat3,
		eTextureFormatFloat4,

		eTextureFormatDepth16,
		eTextureFormatDepth32
	};

	struct STextureResourceData
	{
		const void* memory = nullptr;
		uint32 memoryByteWidth = 0;
		uint32 memoryByteDepth = 0;
	};

	enum ETextureResourceMask : uint8
	{
		eTextureMaskShaderResource	= 1,
		eTextureMaskRenderTarget	= 2,
		eTextureMaskDepthTarget		= 4
	};

	enum ETextureResourceType : uint8
	{
		eTypeTexture1D   = 1,
		eTypeTexture2D	 = 2,
		eTypeTexture3D	 = 3,
		eTypeTextureCube = 4
	};

	struct STextureResourceDesc
	{
		ETextureFormat texformat = ETextureFormat::eTextureFormatUnknown;
		ETextureResourceType textype = ETextureResourceType::eTypeTexture2D;
		uint8 texmask = ETextureResourceMask::eTextureMaskShaderResource;

		uint32 width = 0;
		uint32 height = 0;
		uint32 depth = 0;	
		
		uint32 arraySize = 1;
		
		bool useMips = false;
		SMultisampling multisampling;
	};
	
	enum ETextureFilterMode : uint8
	{
		eTextureFilterPoint,
		eTextureFilterBilinear,
		eTextureFilterTrilinear,
		eTextureFilterAnisotropic2x,
		eTextureFilterAnisotropic4x,
		eTextureFilterAnisotropic8x,
		eTextureFilterAnisotropic16x,
	};

	enum ETextureAddressMode : uint8
	{
		eTextureAddressWrap,
		eTextureAddressMirror,
		eTextureAddressClamp,
		eTextureAddressBorder
	};

	struct STargetDesc
	{
		enum
		{
			eMaxRenderTextures = 8
		};

		HTexture renderTextures[eMaxRenderTextures] = { HTEXTURE_NULL };
		uint32 renderTextureIndices[eMaxRenderTextures] = {0};
		HTexture depthTexture = HTEXTURE_NULL;
		uint32 depthTextureIndex = 0;
	};

	///////////////////////////////////////////////////////////////////////////////////////////
	// Shaders
	///////////////////////////////////////////////////////////////////////////////////////////
	
	enum EShaderStage
	{
		eShaderStageUnknown,
		eShaderStageVertex,
		eShaderStagePixel,
		eShaderStageGeometry,
		eShaderStageHull,
		eShaderStageDomain,
		eShaderStageCompute
	};
	
	///////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//	Draw Command
	/////////////////////////////////////////////////////////////////////////////////////////////////
	
	struct STextureUnit
	{
		HTexture texture = (HTexture)0;
		ETextureResourceType textureType;
		uint32 arrayIndex = 0;
		uint32 arrayCount = 0;
	};
	
	struct STextureSampler
	{
		ETextureAddressMode addressU;
		ETextureAddressMode addressV;
		ETextureAddressMode addressW;
		ETextureFilterMode filtering;
		RGBA borderColour;
		bool enabled = false;
	};
	
	/////////////////////////////////////////////////////////////////////////////////////
	// Vertex attributes

	enum EVertexAttributeType : uint8
	{
		eAttribUnknown,
		eAttribFloat,
		eAttribFloat2,
		eAttribFloat3,
		eAttribFloat4,
		eAttribMatrix,
		eAttribInt32,
		eAttribUint32,
		eAttribRGBA,
		eAttribRGB
	};

	enum EVertexAttributeChannel : uint8
	{
		eChannelPerVertex,
		eChannelPerInstance
	};

	typedef StaticString<32> VertexAttributeString;

	struct SVertexAttribute
	{
		uint32 bufferSlot = 0;
		VertexAttributeString semanticName;
		uint32 byteOffset = 0;
		EVertexAttributeType type = EVertexAttributeType::eAttribUnknown;
		EVertexAttributeChannel channel = EVertexAttributeChannel::eChannelPerVertex;
	};
	
	/////////////////////////////////////////////////////////////////////////////////////
	// Render states

	enum ECullMode : uint8
	{
		eCullNone = 1,
		eCullBack = 2,
		eCullFront = 3
	};

	enum EFillMode : uint8
	{
		eFillSolid = 1,
		eFillWireframe = 2
	};

	struct SRasterState
	{
		bool enableScissor = false;
		ECullMode cullMode = ECullMode::eCullNone;
		EFillMode fillMode = EFillMode::eFillSolid;
	};
	
	struct SDepthState
	{
		bool enableDepth = false;
		bool enableStencil = false;
	};
	
	struct SBlendState
	{
		bool enable = false;
	};
	
	/////////////////////////////////////////////////////////////////////////////////////
	
	enum EVertexTopology
	{
		eTopologyUnknown,
		eTopologyPointList,
		eTopologyLineList,
		eTopologyLineStrip,
		eTopologyTriangleList,
		eTopologyTriangleStrip,
		
		eTopologyPatchList2,
		eTopologyPatchList3,
		eTopologyPatchList4,
	};
	
	enum EDrawMode : uint8
	{
		eDraw,
		eDrawIndexed,
		eDrawInstanced,
		eDrawIndexedInstanced
	};

	struct SDrawCommand
	{
		enum ELimits
		{
			eMaxTextureSlots = 16,
			eMaxTextureSamplerSlots = 8,
			eMaxVertexBuffers = 8,
			eMaxConstantBuffers = 8,
			eMaxVertexAttributes = 16
		};
		
		//Pipeline state
		SBlendState blendState;
		SRasterState rasterState;
		SDepthState depthState;
		//Shaders
		HShader shaderVertex = (HShader)0;
		HShader shaderPixel = (HShader)0;
		HShader shaderGeometry = (HShader)0;
		HShader shaderHull = (HShader)0;
		HShader shaderDomain = (HShader)0;
		
		//Textures
		STextureUnit textureUnits[eMaxTextureSlots];
		STextureSampler textureSamplers[eMaxTextureSamplerSlots];
		
		//Buffers
		HBuffer indexBuffer = (HBuffer)0;
		HBuffer constantBuffers[eMaxConstantBuffers] = { (HBuffer)0 };
		HBuffer vertexBuffers[eMaxVertexBuffers] = { (HBuffer)0 };
		uint32 vertexStrides[eMaxVertexBuffers];
		uint32 vertexOffsets[eMaxVertexBuffers];

		uint32 indexStart = 0;
		uint32 indexCount = 0;
		uint32 vertexStart = 0;
		uint32 vertexCount = 0;
		int32 vertexBase = 0;
		uint32 instanceCount = 1;
		
		EVertexTopology vertexTopology = EVertexTopology::eTopologyUnknown;
		SVertexAttribute vertexAttribs[eMaxVertexAttributes];
		uint32 vertexAttribCount = 0;
		
		EDrawMode mode;
	};
	
	/////////////////////////////////////////////////////////////////////////////////////////////////
}

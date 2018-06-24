/*
    Common global definitions
*/

#pragma once

#include <tscore/types.h>
#include <tscore/strings.h>

#include "Colour.h"

namespace ts
{
	struct RenderDevice;
	struct RenderContext;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	Resource proxies
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	enum class ResourceHandle : uintptr;
	enum class ResourceSetHandle : uintptr;
	enum class ShaderHandle : uintptr;
	enum class PipelineHandle : uintptr;
	enum class TargetHandle : uintptr;
	enum class CommandHandle : uintptr;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	Device structs
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	enum class RenderDriverID
	{
		NONE,
		DX11
	};

	struct DisplayConfig
	{
		uint16 resolutionW = 0;
		uint16 resolutionH = 0;
		uint8 multisampleLevel = 0;
		bool fullscreen = false;
	};

	struct RenderDeviceConfig
	{
		enum Flags : uint8
		{
			DEBUG,
			DEBUG_REPORT
		};

		intptr windowHandle = 0;
		uint32 adapterIndex = 0;
		DisplayConfig display;
		uint8 flags = 0;
	};

	struct RenderStats
	{
		uint32 drawcalls = 0;
	};

	struct RenderDeviceInfo
	{
		String adapterName;
		uint64 gpuVideoMemory;		//GPU accessible video memory capacity
		uint64 gpuSystemMemory;		//GPU accessible system memory capacity
		uint64 sharedSystemMemory;	//GPU/CPU accessible system memory capacity
	};

	struct Viewport
	{
		uint32 w = 0; //width
		uint32 h = 0; //height
		uint32 x = 0; //x offset
		uint32 y = 0; //y offset
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	Resources (buffers+images)
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	enum class BufferType
	{
		UNKNOWN,
		VERTEX,
		INDEX,
		CONSTANTS
	};

	enum class ImageFormat
	{
		UNKNOWN,
		BYTE,

		//8 bits per channel - int
		RGB,
		RGBA,
		ARGB,

		//32 bits per channel - float
		FLOAT1,
		FLOAT2,
		FLOAT3,
		FLOAT4,

		DEPTH16,
		DEPTH32
	};

	enum class ImageUsage : uint8
	{
		SRV = 1 << 0, //shader resource
		RTV = 1 << 1, //render target
		DSV = 1 << 2  //depth target
	};

	inline ImageUsage operator&(ImageUsage lhs, ImageUsage rhs) { return (ImageUsage)((uint8)lhs & (uint8)rhs); }
    inline ImageUsage operator|(ImageUsage lhs, ImageUsage rhs) { return (ImageUsage)((uint8)lhs | (uint8)rhs); }
	inline ImageUsage& operator|=(ImageUsage& lhs, ImageUsage rhs) { lhs = (ImageUsage)((uint8)lhs | (uint8)rhs); return lhs; }
	inline bool operator==(ImageUsage lhs, uint8 rhs) { return (uint8)lhs == rhs; }
	inline bool operator!=(ImageUsage lhs, uint8 rhs) { return (uint8)lhs != rhs; }

	enum class ImageType : uint8
	{
		_1D        = 1,
		_2D        = 2,
		_3D        = 3,
		CUBE       = 4,
	};

	struct ResourceData
	{
		const void* memory = nullptr;
		uint32 memoryByteWidth = 0;
		uint32 memoryByteDepth = 0;
	};

	struct BufferResourceInfo
	{
		uint32 size = 0;
		BufferType type = BufferType::UNKNOWN;
	};

	struct ImageResourceInfo
	{
		ImageFormat format = ImageFormat::UNKNOWN;
		ImageType type = ImageType::_2D;
		ImageUsage usage = ImageUsage::SRV;

		uint32 width = 0;
		uint32 height = 0;
		uint32 length = 0; //or array length

		bool useMips = false;
		uint32 msCount = 1;
        uint32 mipLevels = 1;
	};
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	Sampling structures
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	enum class ImageFilterMode : uint8
	{
		POINT,
		BILINEAR,
		TRILINEAR,
		ANISOTROPIC,
	};

	enum class ImageAddressMode : uint8
	{
		WRAP,
		MIRROR,
		CLAMP,
		BORDER
	};

	struct SamplerState
	{
		ImageAddressMode addressU;
		ImageAddressMode addressV;
		ImageAddressMode addressW;
		ImageFilterMode filtering;
		RGBA borderColour;
		uint32 anisotropy = 1;
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Vertex attributes
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	enum class VertexTopology
	{
		POINTLIST,
		LINELIST,
		LINESTRIP,
		TRIANGELIST,
		TRIANGESTRIP,
		PATCHLIST2,
		PATCHLIST3,
		PATCHLIST4,
	};

	enum class VertexAttributeType : uint8
	{
		UNKNOWN,
		FLOAT,
		FLOAT2,
		FLOAT3,
		FLOAT4,
		MATRIX,
		INT32,
		UINT32,
		RGBA,
		RGB
	};

	enum class VertexAttributeChannel : uint8
	{
		VERTEX,
		INSTANCE
	};

	struct VertexAttribute
	{
		uint32 bufferSlot = 0;
		const char* semanticName;
		uint32 byteOffset = 0;
		VertexAttributeType type = VertexAttributeType::UNKNOWN;
		VertexAttributeChannel channel = VertexAttributeChannel::VERTEX;
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Pipeline states
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	enum class CullMode : uint8
	{
		NONE = 1,
		BACK = 2,
		FRONT = 3
	};

	enum class FillMode : uint8
	{
		SOLID = 1,
		WIREFRAME = 2
	};

	struct RasterizerState
	{
		bool enableScissor = false;
		CullMode cullMode = CullMode::NONE;
		FillMode fillMode = FillMode::SOLID;
	};

	struct DepthState
	{
		bool enableDepth = false;
		bool enableStencil = false;
	};

	struct BlendState
	{
		bool enable = false;
	};

	/*
		Pipeline state info structure
	*/
	struct PipelineCreateInfo
	{
		RasterizerState raster;
		DepthState depth;
		BlendState blend;

		const SamplerState* samplers;
		size_t samplerCount;

		const VertexAttribute* vertexAttributeList;
		size_t vertexAttributeCount;

		VertexTopology topology;
	};
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //  Shaders
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
	enum class ShaderStage
	{
		VERTEX,
        GEOMETRY,
        TESSCTRL,
        TESSEVAL,
        PIXEL,
        COMPUTE,

		MAX_STAGES
	};

	struct ShaderBytecode
	{
		const void* bytecode = nullptr;
		size_t size = 0;
	};

	/*
		Shader info structure
	*/
	struct ShaderCreateInfo
	{
		ShaderBytecode stages[(size_t)ShaderStage::MAX_STAGES];
	};

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //  Resources
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    struct ImageView
    {
        ResourceHandle image = (ResourceHandle)0;
        uint32 index = 0;
		uint32 count = 1;
		ImageType type = ImageType::_2D;
    };
    
    struct VertexBufferView
    {
        ResourceHandle buffer = (ResourceHandle)0;
        uint32 stride = 0;
        uint32 offset = 0;
    };

	/*
		Target structure
	*/
	struct TargetCreateInfo
	{
        const ImageView* attachments;
        uint32 attachmentCount;
        
        ImageView depth;
        
        Viewport viewport;
        Viewport scissor;
	};

	/*
		Resource set structure
	*/
	struct ResourceSetCreateInfo
	{
        const ImageView* resources;  //shader resources
        uint32 resourceCount;
        
        const ResourceHandle* constantBuffers; //Constant buffers
        uint32 constantBuffersCount;
        
        const VertexBufferView* vertexBuffers;
        uint32 vertexBufferCount;
        
        ResourceHandle indexBuffer;
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//  Draw command
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	enum class DrawMode : uint8
	{
		VERTEX,
		INDEXED,
		INSTANCED,
		INDEXEDINSTANCED
	};

	struct DrawCommandInfo
	{
		TargetHandle outputs;
		PipelineHandle pipeline;
		ResourceSetHandle inputs;

		uint32 start = 0; //vertex/index start
		uint32 count = 0; //vertex/index count
		int32 vbase = 0;  //vertex base
		uint32 instances = 1;

		DrawMode mode = DrawMode::VERTEX;
	};
}

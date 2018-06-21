/*
    Common global definitions
*/

#pragma once

#include <tscore/types.h>

#include "Colour.h"

namespace ts
{
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	Resource proxies
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	enum class ResourceHandle : uintptr;
	enum class ResourceSetHandle : uintptr;
	enum class ShaderHandle : uintptr;
	enum class StateHandle : uintptr;
	enum class TargetHandle : uintptr;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	Device structs
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct DisplayConfig
	{
		uint16 resolutionW = 0;
		uint16 resolutionH = 0;
		uint8 multisampleLevel = 0;
		bool fullscreen = false;
	};

	struct GraphicsDeviceConfig
	{
		intptr windowHandle = 0;
		uint32 adapterIndex = 0;
		DisplayConfig display;
		uint8 flags = 0;
	};

	struct RenderStats
	{
		uint32 drawcalls = 0;
	};

	struct DeviceInfo
	{
		const char* adapterName;
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

	enum class ImageTypeMask : uint8
	{
		SRV = 1 << 0,
		RTV = 1 << 1,
		DSV = 1 << 2
	};

	enum class ImageType : uint8
	{
		_1D   = 1,
		_2D   = 2,
		_3D   = 3,
		CUBE = 4,
		ARRAY_2D = 5,
		ARRAY_CUBE = 6 
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
		uint8 mask = (uint8)ImageTypeMask::SRV;

		uint32 width = 0;
		uint32 height = 0;
		uint32 depth = 0; //or array length

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

	struct ImageSampler
	{
		uint32 slot;

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
		POINT,
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

	/*
		Shader info structure
	*/
	struct ShaderCreateInfo
	{
		const void* vsByteCode;		//vertex shader
		const void* psByteCode;		//pixel shader
		const void* tesByteCode;	//tessellation evaluation shader
		const void* tcsByteCode;	//tessellation control shader
		const void* gsByteCode;		//geometry shader

		size_t vsSize;
		size_t psSize;
		size_t tesSize;
		size_t tcsSize;
		size_t gsSize;

		const void* cosByteCode;	//compute shader
		size_t cosSize;
	};

	/*
		Pipeline state info structure
	*/
	struct StateCreateInfo
	{
		struct RasterizerState
		{
			bool enableScissor = false;
			CullMode cullMode = CullMode::NONE;
			FillMode fillMode = FillMode::SOLID;
		} raster;

		struct DepthState
		{
			bool enableDepth = false;
			bool enableStencil = false;
		} depth;

		struct BlendState
		{
			bool enable = false;
		} blend;


		const ImageSampler* samplers;
		size_t samplerCount;

		const VertexAttribute* vertexAttrib;
		size_t vertexAttribCount;

		VertexTopology topology;
	};
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //  Resources
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    struct ImageView
    {
        ResourceHandle image;
        uint32 index;
    };
    
    struct VertexBufferView
    {
        ResourceHandle buffer;
        uint32 stride;
        uint32 offset;
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
	struct ResourceSetInfo
	{
        const ImageView* resources;  //shader resources
        uint32 resourceCount;
        
        const ResourceHandle* constants; //Constant buffers
        uint32 constantsCount;
        
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

	struct DrawCommand
	{
		TargetHandle target;
		StateHandle state;
		ResourceSetHandle resources;

		uint32 indexStart = 0;
		uint32 indexCount = 0;
		uint32 vertexStart = 0;
		uint32 vertexCount = 0;
		int32 vertexBase = 0;
		uint32 instanceCount = 1;

		DrawMode draw = DrawMode::VERTEX;
	};
}

/*
	Render API

	The render api acts as a layer between the rendering module and the low level graphics implementation
*/

#pragma once

#include "rendercommon.h"
#include <map>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	///////////////////////////////////////////////////////////////////////////////////////////
	// Types
	///////////////////////////////////////////////////////////////////////////////////////////
	
	//Interfaces and structs
	struct IRender;
	struct IRenderContext;
	struct IAdapterFactory;
	struct IShaderCompiler;
	struct SDrawCommand;
	
	//Resource handles
	enum HBuffer : intptr {};
	enum HTexture : intptr {};
	enum HTarget : intptr {};
	enum HShader : intptr {};
	enum HDrawCmd : intptr {};
	
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
		eTextureFilterAnisotropic
	};

	enum ETextureAddressMode : uint8
	{
		eTextureAddressWrap,
		eTextureAddressMirror,
		eTextureAddressClamp,
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

	struct SViewport
	{
		uint32 w = 0; //width
		uint32 h = 0; //height
		uint32 x = 0; //x offset
		uint32 y = 0; //y offset
	};

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
		uint8 multisampleCount = 0;
		bool fullscreen = false;
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
	
	struct IRender
	{
		//Resource methods
		virtual ERenderStatus createResourceBuffer(HBuffer& rsc, const SBufferResourceData& data) = 0;
		virtual ERenderStatus createResourceTexture(HTexture& rsc, const STextureResourceData* data, const STextureResourceDesc& desc) = 0;
		virtual ERenderStatus createShader(HShader& shader, const void* bytecode, uint32 bytecodesize, EShaderStage stage) = 0;
		virtual ERenderStatus createTarget(HTarget& target, const HTexture* renderTexture, const uint32* renderTextureIndices, uint32 renderTextureCount, HTexture deptTextureProxy, uint32 deptTextureProxyIndex) = 0;
		
		virtual void destroyBuffer(HBuffer buffer) = 0;
		virtual void destroyTexture(HTexture texture) = 0;
		virtual void destroyShader(HShader shader) = 0;
		virtual void destroyTarget(HTarget target) = 0;
		
		//Command methods
		virtual ERenderStatus createDrawCommand(HDrawCmd& cmd, const SDrawCommand& desc) = 0;
		virtual void destroyDrawCommand(HDrawCmd cmd) = 0;
		
		//Render context
		virtual void createContext(IRenderContext** context) = 0;
		virtual void destroyContext(IRenderContext* context) = 0;
		
		//Display methods
		virtual void setDisplayConfiguration(const SDisplayConfig& displayCfg) = 0;
		virtual void getDisplayConfiguration(SDisplayConfig& displayCfg) = 0;
		virtual void getDisplayTarget(HTarget& target) = 0;
		
		virtual void getDrawStatistics(SRenderStatistics& stats) = 0;
		
		virtual void drawBegin(const Vector& vec) = 0;
		virtual void drawEnd(IRenderContext** contexts, uint32 numContexts) = 0;
	};
	
	/////////////////////////////////////////////////////////////////////////////////////////////////
	//	Draw Command
	/////////////////////////////////////////////////////////////////////////////////////////////////
	
	struct STextureUnit
	{
		HTexture texture;
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
		uint anisotropy = 0;
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

	struct SVertexAttribute
	{
		uint32 bufferSlot = 0;
		const char* semanticName = "";
		uint32 byteOffset = 0;
		EVertexAttributeType type = EVertexAttributeType::eAttribUnknown;
		EVertexAttributeChannel channel = EVertexAttributeChannel::eChannelPerVertex;
	};
	
	/////////////////////////////////////////////////////////////////////////////////////
	// Render states

	struct SRasterState
	{
		bool enableScissor = false;
	};
	
	struct SDepthState
	{
		bool enable = false;
	};
	
	struct SBlendState
	{
		bool enable = false;
	};
	
	/////////////////////////////////////////////////////////////////////////////////////
	
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
		HShader shaderVertex;
		HShader shaderPixel;
		HShader shaderGeometry;
		HShader shaderHull;
		HShader shaderDomain;
		
		//Textures
		STextureUnit textureUnits[eMaxTextureSlots];
		STextureSampler textureSamplers[eMaxTextureSamplerSlots];
		
		//Buffers
		HBuffer indexBuffer;
		HBuffer constantBuffers[eMaxConstantBuffers];
		HBuffer vertexBuffers[eMaxVertexBuffers];
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
	//	Render Context
	/////////////////////////////////////////////////////////////////////////////////////////////////
	struct IRenderContext
	{
		virtual void bufferUpdate(HBuffer rsc, const void* memory) = 0;
		virtual void bufferCopy(HBuffer src, HBuffer dest) = 0;
		virtual void textureUpdate(HTexture rsc, uint32 index, const void* memory) = 0;
		virtual void textureCopy(HTexture src, HTexture dest) = 0;
		virtual void textureResolve(HTexture src, HTexture dest) = 0;
		
		virtual void clearRenderTarget(HTarget target, const Vector& vec) = 0;
		virtual void clearDepthTarget(HTarget target, float depth) = 0;
		
		virtual void draw(
			HTarget target,
			const SViewport& viewport,
			const SViewport& scissor,
			HDrawCmd command
		) = 0;
		
		virtual void finish() = 0;
	};
	
	struct IAdapterFactory
	{	
		virtual uint32 getAdapterCount() const = 0;
		virtual bool enumAdapter(uint32 idx, SRenderAdapterDesc& desc) const = 0;
	};
	
	struct SShaderCompileConfig
	{
		const char* entrypoint = nullptr;
		EShaderStage stage;
		bool debuginfo = false;
	};
	
	struct IShaderCompiler
	{
		virtual bool compile(const char* code, const SShaderCompileConfig& options, MemoryBuffer& bytecode) = 0;
	};
	
	/////////////////////////////////////////////////////////////////////////////////////////////////
	
	namespace abi
	{
		extern "C"
		{
			int createRenderApi(IRender** api, const SRenderApiConfig& cfg);
			void destroyRenderApi(IRender* api);
			
			int createShaderCompiler(IShaderCompiler** compiler);
			void destroyShaderCompiler(IShaderCompiler* compiler);
			
			int createAdapterFactory(IAdapterFactory** adapterFactory);
			void destroyAdapterFactory(IAdapterFactory* adapterFactory);
		}
	}
	
	/////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////
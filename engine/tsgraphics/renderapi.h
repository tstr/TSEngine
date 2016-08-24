/*
	Render API

	The render api acts as a layer between the rendering module and the low level graphics implementation (D3D11 in this case)
*/

#pragma once

#include "rendercommon.h"

#include <tscore/filesystem/path.h>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	///////////////////////////////////////////////////////////////////////////////////////////
	//Render resources
	///////////////////////////////////////////////////////////////////////////////////////////
	
	class IRenderApi;

	enum EResourceType
	{
		eResourceUnknown,
		eResourceBuffer,
		eResourceTexture,
		eResourceViewTexture,
		eResourceViewRender,
		eResourceViewDepth,
		eResourceShader
	};

	struct IRenderResource
	{
		virtual EResourceType getType() const = 0;
		virtual IRenderApi* getApi() const = 0;
		virtual void release() const = 0;
	};

	class ResourceProxy
	{
	private:
		
		IRenderResource* m_handle = nullptr;
		
	public:

		void set(IRenderResource* rsc) { m_handle = rsc; }
		IRenderResource* const get() const { return m_handle; }
		IRenderResource** getPtr() { return &m_handle; }
		bool isNull() const { return (m_handle != nullptr); }
		
		ResourceProxy() {}
		ResourceProxy(IRenderResource* h) : m_handle(h) {}
		
		//ResourceProxy(const ResourceProxy& h) = delete;
		
		ResourceProxy(ResourceProxy&& h) : m_handle(h.m_handle)
		{
			if (m_handle)
			{
				h.m_handle->release();
			}
		}
		
		~ResourceProxy()
		{
			if (m_handle) m_handle->release();
		}
		
		//ResourceProxy& operator=(const ResourceProxy& handle) = delete;
		
		ResourceProxy& operator=(ResourceProxy&& handle)
		{
			if (m_handle)
				m_handle->release();
			
			if (m_handle = handle.m_handle)
			{
				handle.m_handle->release();
			}
		}
		
		EResourceType getType() const { return m_handle->getType(); }
	};

	///////////////////////////////////////////////////////////////////////////////////////////
	//Buffers
	///////////////////////////////////////////////////////////////////////////////////////////

	enum EBufferUsage
	{
		eUsageUnknown,
		eUsageVertex,
		eUsageIndex,
		eUsageUniform
	};

	struct SResourceBufferData
	{
		const void* memory = nullptr;
		uint32 size = 0;
		EBufferUsage usage = EBufferUsage::eUsageUnknown;
	};

	///////////////////////////////////////////////////////////////////////////////////////////
	//Textures
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

	enum ETextureResourceMask
	{
		eTextureMaskShaderResource	= 1,
		eTextureMaskRenderTarget	= 2,
		eTextureMaskDepthTarget		= 4
	};

	enum ETextureResourceType
	{
		eTypeTexture1D   = 1,
		eTypeTexture2D	 = 2,
		eTypeTexture3D	 = 3,
		eTypeTextureCube = 4
	};

	struct STextureResourceDescriptor
	{
		ETextureFormat texformat = ETextureFormat::eTextureFormatUnknown;
		ETextureResourceType textype = ETextureResourceType::eTypeTexture2D;
		uint32 texmask = ETextureResourceMask::eTextureMaskShaderResource;

		uint32 width = 0;
		uint32 height = 0;
		uint32 depth = 0;	
		
		uint32 arraySize = 1;
		
		bool useMips = false;
		SMultisampling multisampling;
	};

	struct STextureViewDescriptor
	{
		uint32 arrayIndex = 0;
		uint32 arrayCount = 0;
	};

	enum ETextureSampleFilter
	{
		eTextureFilterPoint,
		eTextureFilterLinear,
		eTextureFilterBilinear,
		eTextureFilterTrilinear,
		eTextureFilterAnisotropic2x,
		eTextureFilterAnisotropic4x,
		eTextureFilterAnisotropic8x,
		eTextureFilterAnisotropic16x
	};

	enum ETextureSampleAddress
	{
		eTextureAddressWrap,
		eTextureAddressMirror,
		eTextureAddressClamp,
	};

	struct STextureSampler
	{
		ETextureSampleAddress addressMode;
		ETextureSampleFilter filtering;
	};

	///////////////////////////////////////////////////////////////////////////////////////////
	//Shaders
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

	struct SShaderProgram
	{
		ResourceProxy stageVertex;
		ResourceProxy stagePixel;
		ResourceProxy stageGeometry;
		ResourceProxy stageHull;
		ResourceProxy stageDomain;
	};

	enum EShaderInputType
	{
		eShaderInputUnknown,
		eShaderInputFloat,
		eShaderInputFloat2,
		eShaderInputFloat3,
		eShaderInputFloat4,
		eShaderInputMatrix,
		eShaderInputInt32,
		eShaderInputUint32,
	};

	enum EShaderInputChannel
	{
		eInputPerVertex,
		eInputPerInstance
	};

	struct ShaderInputDescriptor
	{
		uint32 bufferSlot = 0;
		const char* semanticName = "";
		uint32 byteOffset = 0;
		EShaderInputType type = EShaderInputType::eShaderInputUnknown;
		EShaderInputChannel channel = EShaderInputChannel::eInputPerVertex;
	};

	///////////////////////////////////////////////////////////////////////////////////////////

	struct Viewport
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
		eInvalidTextureResource,
		eInvalidTextureView,
		eInvalidTextureFormat,
		eInvalidShaderByteCode
	};

	struct SRenderApiConfiguration
	{
		intptr windowHandle = 0;
		uint32 adapterIndex = 0;
		uint32 resolutionWidth = 0;
		uint32 resolutionHeight = 0;
		EWindowMode windowMode = EWindowMode::eWindowDefault;
		uint16 flags = 0;
	};

	class IRenderContext;
	class IRenderAdapterFactory;
	class IShaderCompiler;
	
	class IRenderApi
	{
	public:
		
		virtual ERenderStatus createResourceBuffer(ResourceProxy& rsc, const SResourceBufferData& data) = 0;
		virtual ERenderStatus createResourceTexture(ResourceProxy& rsc, const STextureResourceData* data, const STextureResourceDescriptor& desc) = 0;
		
		virtual ERenderStatus createTargetDepth(ResourceProxy& view, const ResourceProxy& rsc, const STextureViewDescriptor& desc) = 0;
		virtual ERenderStatus createTargetRender(ResourceProxy& view, const ResourceProxy& rsc, const STextureViewDescriptor& desc) = 0;

		virtual ERenderStatus createViewTextureCube(ResourceProxy& view, const ResourceProxy& rsc, const STextureViewDescriptor& desc) = 0;
		virtual ERenderStatus createViewTexture2D(ResourceProxy& view, const ResourceProxy& rsc, const STextureViewDescriptor& desc) = 0;
		virtual ERenderStatus createViewTexture3D(ResourceProxy& view,const ResourceProxy& rsc ) = 0;
		
		virtual ERenderStatus createShader(ResourceProxy& shader, const void* bytecode, uint32 bytecodesize, EShaderStage stage) = 0;
		virtual ERenderStatus createShaderInputDescriptor(ResourceProxy& rsc, const ResourceProxy& vertexshader, const ShaderInputDescriptor* sids, uint32 sidnum) = 0;
		
		virtual IRenderContext* createRenderContext() = 0;
		virtual void destroyRenderContext(IRenderContext* context) = 0;

		virtual void setWindowMode(EWindowMode mode) = 0;
		virtual void setWindowDimensions(uint32 w, uint32 h) = 0;

		virtual void drawBegin(const Vector& vec) = 0;
		virtual void drawEnd() = 0;

		virtual ~IRenderApi() {}
	};

	enum EResourceLimits
	{
		eMaxTextureSlots = 16,
		eMaxTextureSamplerSlots = 4,
		eMaxVertexBuffers = 8,
		eMaxUniformBuffers = 8,
		eMaxRenderTargets = 4,
		eMaxUnorderedAccessViews = 4
	};


	enum ERenderCommandFlag : uint16
	{
		eCommandFlag_Null		   = 0,
		eCommandFlag_DisableDepth  = 1,
		eCommandFlag_DisableColour = 2
	};

	struct SRenderCommand
	{
		ResourceProxy depthTarget;
		ResourceProxy renderTarget[EResourceLimits::eMaxRenderTargets];
		
		Viewport viewport;

		SShaderProgram shaders;
		ResourceProxy textures[EResourceLimits::eMaxTextureSlots];
		STextureSampler textureSamplers[EResourceLimits::eMaxTextureSamplerSlots];
		
		ResourceProxy indexBuffer;
		ResourceProxy vertexBuffer;
		uint32 vertexStride;
		//todo: multiple vertex buffers
		//ResourceProxy vertexBuffers[EResourceLimits::eMaxVertexBuffers];
		ResourceProxy uniformBuffers[EResourceLimits::eMaxUniformBuffers];

		uint32 indexStart = 0;
		uint32 indexCount = 0;
		uint32 vertexStart = 0;
		uint32 vertexCount = 0;
		uint32 instanceCount = 1;
		
		EVertexTopology vertexTopology = EVertexTopology::eTopologyUnknown;
		ResourceProxy vertexInputDescriptor;

		ERenderCommandFlag flags;
	};

	class IRenderContext
	{
	public:
		
		virtual void resourceBufferUpdate(const ResourceProxy& rsc, const void* memory) = 0;
		virtual void resourceBufferCopy(const ResourceProxy& src, ResourceProxy& dest) = 0;
		virtual void resourceTextureUpdate(uint32 index, ResourceProxy& rsc, const void* memory) = 0;
		virtual void resourceTextureCopy(const ResourceProxy& src, ResourceProxy& dest) = 0;
		virtual void resourceTextureResolve(const ResourceProxy& src, ResourceProxy& dest) = 0;
		
		virtual void clearRenderTarget(const ResourceProxy& renderview, const Vector& vec) = 0;
		virtual void clearDepthTarget(const ResourceProxy& depthview, float depth) = 0;

		//Execute a draw call
		virtual void execute(const SRenderCommand& command) = 0;
	};

	class IRenderAdapterFactory
	{
	public:

		virtual uint32 getAdapterCount() const = 0;
		virtual bool enumAdapter(uint32 idx, SRenderAdapterDesc& desc) const = 0;
	};

	struct SShaderCompileConfig
	{
		StaticString<64> entrypoint;
		EShaderStage stage;
		//Compile a shader with debug information
		bool debuginfo = false;
	};

	class IShaderCompiler
	{
	public:

		virtual bool compile(const char* str, const SShaderCompileConfig& options, MemoryBuffer& bytecode) = 0;
		virtual bool compileFromFile(const Path& file, const SShaderCompileConfig& options, MemoryBuffer& bytecode) = 0;
	};
	
	/////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////
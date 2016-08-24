
///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////

enum EResourceType
{
	eResourceUnknown,
	eResourceBuffer,
	eResourceTexture,
	eResourceViewTexture,
	eResourceViewRender,
	eResourceViewDepth,
	eResourceShader,
	eResourceShaderProgram
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

	IRenderResource* const get() const { return m_handle; }
	IRenderResource** getPtr() { return &m_handle; }
	bool isNull() const { return (m_handle != nullptr); }
	
	ResourceProxy() {}
	ResourceProxy(IRenderResource* h) : m_handle(h) {}
	
	ResourceProxy(const ResourceProxy& h) = delete;
	
	ResourceProxy(ResourceProxy&& h) : m_handle(h.m_handle)
	{
		if (m_handle)
		{
			h.m_handle->Release();
		}
	}
	
	~ResourceProxy()
	{
		if (m_handle) m_handle->Release();
	}
	
	ResourceProxy& operator=(const ResourceProxy& handle) = delete;
	
	ResourceProxy& operator=(ResourceProxy&& handle)
	{
		if (m_handle)
			m_handle->Release();
		
		if (m_handle = h.m_handle)
		{
			h.m_handle->Release();
		}
	}
	
	ResourceType getType() const { return m_type; }
};

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
	EBufferUsage usage = BufferUsage::UsageUnknown;
};

///////////////////////////////////////////////////////////////////////////////////////////

enum ETextureFormat
{
	eFormatUnknown,
	
	eFormatByte,
	
	//8 bits per channel - int
	eFormatColourRGB,
	eFormatColourRGBA,
	eFormatColourARGB,
	
	//32 bits per channel - float
	eFormatFloat1,
	eFormatFloat2,
	eFormatFloat3,
	eFormatFloat4,
	
	eFormatDepth16,
	eFormatDepth32
}

struct STextureResourceData
{
	const void* memory = nullptr;
	uint32 memoryByteWidth = 0;
	uint32 memoryByteDepth = 0;
};

enum ETextureResourceType
{
	eTextureShaderResource 		 = 1,
	eTextureRenderTargetResource = 2,
	eTextureDepthTargetResource  = 4
};

struct STextureResourceDescriptor
{
	TextureFormat format = TextureFormat::FormatUnknown;
	TextureResourceType typemask = 0;

	uint32 width = 0;
	uint32 height = 0;
	uint32 depth = 0;	
	
	uint32 arraySize = 1;
	
	bool useMips = false;
	Multisampling sampling;
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
	EShaderInputType type = ShaderInputType::InputUnknown;
	EShaderInputChannel channel = ShaderInputChannel::PerVertex;
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
	eOk,
	eInvalidParameter,
	eInvalidResource,
	eInvalidTextureResource,
	eInvalidTextureView,
	eInvalidTextureFormat,
	eInvalidShaderByteCode
};

class IRenderApi
{
public:
	
	virtual ERenderStatus createResourceBuffer(ResourceProxy& rsc, const ResourceBufferData& data) = 0;
	virtual ERenderStatus createResourceTexture2D(ResourceProxy& rsc, const TextureResourceData* data, const TextureResourceDescriptor& desc) = 0;
	virtual ERenderStatus createResourceTexture3D(ResourceProxy& rsc, const TextureResourceData* data, const TextureResourceDescriptor& desc) = 0;
	virtual ERenderStatus createResourceTextureCube(ResourceProxy& rsc, const TextureResourceData* data, const TextureResourceDescriptor& desc) = 0;
	
	virtual ERenderStatus createViewRenderTarget(ResourceProxy& view, const ResourceProxy& rsc, const TextureViewDescriptor& desc) = 0;
	virtual ERenderStatus createViewDepthTarget(ResourceProxy& view, const ResourceProxy& rsc, const TextureViewDescriptor& desc) = 0;
	
	virtual ERenderStatus createViewTextureCube(ResourceProxy& view, const ResourceProxy& rsc, const TextureViewDescriptor& desc) = 0;
	virtual ERenderStatus createViewTexture2D(ResourceProxy& view, const ResourceProxy& rsc, const TextureViewDescriptor& desc) = 0;
	virtual ERenderStatus createViewTexture3D(ResourceProxy& view,const ResourceProxy& rsc ) = 0;
	
	virtual ERenderStatus createShaderStage(ResourceProxy& shader, const void* bytecode, uint32 bytecodesize, ShaderType stage) = 0;
	virtual ERenderStatus createShaderProgram(ResourceProxy& shaderProg, const ShaderProgram& desc) = 0;
	virtual ERenderStatus createShaderInputDescriptor(ResourceProxy& rsc, const ShaderInputDescriptor& desc) = 0;
	
	virtual IRenderContext* createRenderContext() = 0;
	virtual void destroyRenderContext(IRenderContext* rc) = 0;
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

struct SRenderCommand
{
	ResourceProxy depthTarget;
	ResourceProxy renderTarget[ResourceLimits::MaxRenderTargets];
	
	Viewport viewport;

	ResourceProxy shaderProgram;
	ResourceProxy textures[ResourceLimits::MaxTextureSlots];
	TextureSampler textureSamplers[ResourceLimits::MaxTextureSamplerSlots];
	
	ResourceProxy indexBuffer;
	ResourceProxy vertexBuffers[ResourceLimits::MaxVertexBuffers];
	ResourceProxy uniformBuffers[ResourceLimits::MaxUniformBuffers];
	
	uint32 indexStart = 0;
	uint32 indexCount = 0;
	uint32 vertexStart = 0;
	uint32 vertexCount = 0;
	uint32 instanceCount = 1;
	
	VertexTopology vertexTopology = VertexTopology::TopologyUnknown;
	ResourceProxy vertexInputDescriptor;

	bool enableDepth = true;
	bool enableAlpha = true;
};

struct ComputeCall
{
	ResourceProxy computeShader;
	ResourceProxy textures[ResourceLimits::ResourceLimitMaxTextureSlots];
	ResourceProxy unorderedAccessViews[ResourceLimits::ResourceLimitMaxUnorderedAccessViews];
};

class IRenderContext
{
public:
	
	virtual void resourceBufferUpdate(const ResourceProxy& rsc, const void* memory) = 0;
	virtual void resourceBufferCopy(const ResourceProxy& src, ResourceProxy& dest) = 0;
	virtual void resourceTextureUpdate(uint32 index, ResourceProxy& rsc, const void* memory) = 0;
	virtual void resourceTextureCopy(const ResourceProxy& src, ResourceProxy& dest) = 0;
	virtual void resourceTextureResolve(const ResourceProxy& src, ResourceProxy& dest) = 0;
	
	virtual void SubmitCommand(const SRenderCommand& command) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////

// gfxcore.h
// gfxqueue.h
// gfx.h

// ct/graphics/gfx.h

///////////////////////////////////////////////////////////////////////////////////////////
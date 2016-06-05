
///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////



namespace ct
{
	namespace api
	{
		
	}
}

enum class ResourceType
{
	ResourceUnknown,
	ResourceBuffer,
	ResourceTexture,
	ResourceViewTexture,
	ResourceViewRender,
	ResourceViewDepth,
	ResourceShader,
	ResourceShaderProgram
};

class Resource
{
public:
	
	virtual ResourceType GetType() const = 0;
	virtual IRenderApi* GetApi() const = 0;
	
	virtual void AddRef() const = 0;
	virtual uint32 GetRef() const = 0;
	virtual void Release() const = 0;
};

class ResourcePtr
{
private:
	
	Resource* m_handle = nullptr;
	
public:

	Resource* const get() const { return m_handle; }
	bool isNull() const { return (m_handle != nullptr); }
	
	
	ResourcePtr() {}
	ResourcePtr(Resource* h) : m_handle(h) {}
	
	ResourcePtr(const ResourcePtr& h) : m_handle(h.m_handle)
	{
		if (m_handle) m_handle->AddRef();
	}
	
	ResourcePtr(ResourcePtr&& h) : m_handle(h.m_handle)
	{
		if (m_handle)
		{
			m_handle->AddRef();
			h.m_handle->Release();
		}
	}
	
	~ResourcePtr()
	{
		if (m_handle) m_handle->Release();
	}
	
	ResourcePtr& operator=(const ResourcePtr& handle)
	{
		if (m_handle)
			m_handle->Release();
		
		if (m_handle = h.m_handle) m_handle->AddRef();
	}
	
	ResourcePtr& operator=(ResourcePtr&& handle)
	{
		if (m_handle)
			m_handle->Release();
		
		if (m_handle = h.m_handle)
		{
			m_handle->AddRef();
			h.m_handle->Release();
		}
	}
	
	ResourceType GetType() const { return m_type; }
};

///////////////////////////////////////////////////////////////////////////////////////////

enum class BufferUsage
{
	UsageUnknown,
	UsageVertex,
	UsageIndex,
	UsageUniform
};

struct ResourceBufferData
{
	const void* memory = nullptr;
	uint32 size = 0;
	BufferUsage usage = BufferUsage::UsageUnknown;
};

///////////////////////////////////////////////////////////////////////////////////////////

enum TextureFormat
{
	FormatUnknown,
	
	FormatByte,
	
	//8 bits per channel - int
	FormatColourRGB,
	FormatColourRGBA,
	FormatColourARGB,
	
	//32 bits per channel - float
	FormatFloat1,
	FormatFloat2,
	FormatFloat3,
	FormatFloat4,
	
	FormatDepth16,
	FormatDepth32
}

struct TextureResourceData
{
	const void* memory = nullptr;
	uint32 memoryByteWidth = 0;
	uint32 memoryByteDepth = 0;
};

enum TextureResourceType
{
	TextureResource 			= 1,
	TextureRenderTargetResource = 2,
	TextureDepthTargetResource  = 4
};

struct TextureResourceDescriptor
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

struct TextureViewDescriptor
{
	uint32 arrayIndex = 0;
	uint32 arrayCount = 0;
};

enum TextureFilter
{
	TextureFilterPoint,
	TextureFilterLinear,
	TextureFilterBilinear,
	TextureFilterTrilinear,
	TextureFilterAnisotropic2x,
	TextureFilterAnisotropic4x,
	TextureFilterAnisotropic8x,
	TextureFilterAnisotropic16x
};

enum TextureAddress
{
	TextureAddressWrap,
	TextureAddressMirror,
	TextureAddressClamp,
};

struct TextureSampler
{
	TextureAddress addressMode;
	TextureFilter filtering;
};

///////////////////////////////////////////////////////////////////////////////////////////

enum class ShaderStage
{
	StageUnknown,
	StageVertex,
	StagePixel,
	StageGeometry,
	StageCompute
};

struct ShaderProgram
{
	ResourcePtr vertexShaderStage;
	ResourcePtr pixelShaderStage;
	ResourcePtr geometryShaderStage;
};

enum class ShaderInputType
{
	InputUnknown,
	InputFloat,
	InputFloat2,
	InputFloat3,
	InputFloat4,
	InputMatrix,
	InputInt32,
	InputUint32,
};

enum class ShaderInputChannel
{
	PerVertex,
	PerInstance
};

struct ShaderInputDescriptor
{
	uint32 bufferSlot = 0;
	const char* semanticName = "";
	uint32 byteOffset = 0;
	ShaderInputType type = ShaderInputType::InputUnknown;
	ShaderInputChannel channel = ShaderInputChannel::PerVertex;
};

///////////////////////////////////////////////////////////////////////////////////////////

struct Viewport
{
	uint32 w = 0; //width
	uint32 h = 0; //height
	uint32 x = 0; //x offset
	uint32 y = 0; //y offset
};

enum class RenderStatus
{
	Ok,
	InvalidParameter,
	InvalidResource,
	InvalidTextureResource,
	InvalidTextureView,
	InvalidTextureFormat,
	InvalidShaderByteCode
};

class IRenderApi
{
public:
	
	virtual RenderStatus CreateResourceBuffer(ResourcePtr& rsc, const ResourceBufferData& data) = 0;
	virtual RenderStatus CreateResourceTexture2D(ResourcePtr& rsc, const TextureResourceData* data, const TextureResourceDescriptor& desc) = 0;
	virtual RenderStatus CreateResourceTexture3D(ResourcePtr& rsc, const TextureResourceData* data, const TextureResourceDescriptor& desc) = 0;
	virtual RenderStatus CreateResourceTextureCube(ResourcePtr& rsc, const TextureResourceData* data, const TextureResourceDescriptor& desc) = 0;
	
	virtual RenderStatus CreateViewRenderTarget(ResourcePtr& view, const ResourcePtr& rsc, const TextureViewDescriptor& desc) = 0;
	virtual RenderStatus CreateViewDepthTarget(ResourcePtr& view, const ResourcePtr& rsc, const TextureViewDescriptor& desc) = 0;
	
	virtual RenderStatus CreateViewTextureCube(ResourcePtr& view, const ResourcePtr& rsc, const TextureViewDescriptor& desc) = 0;
	virtual RenderStatus CreateViewTexture2D(ResourcePtr& view, const ResourcePtr& rsc, const TextureViewDescriptor& desc) = 0;
	virtual RenderStatus CreateViewTexture3D(ResourcePtr& view,const ResourcePtr& rsc ) = 0;
	
	virtual RenderStatus CreateShaderStage(ResourcePtr& shader, const void* bytecode, uint32 bytecodesize, ShaderType stage) = 0;
	virtual RenderStatus CreateShaderProgram(ResourcePtr& shaderProg, const ShaderProgram& desc) = 0;
	virtual RenderStatus CreateShaderInputDescriptor(ResourcePtr& rsc, const ShaderInputDescriptor& desc) = 0;
	
	virtual IRenderContext* CreateRenderContext() = 0;
	virtual void DestroyRenderContext(IRenderContext* rc) = 0;
};

enum ResourceLimits
{
	MaxTextureSlots = 16,
	MaxTextureSamplerSlots = 4,
	MaxVertexBuffers = 8,
	MaxUniformBuffers = 8,
	MaxRenderTargets = 4,
	MaxUnorderedAccessViews = 4
};

struct DrawCall
{
	ResourcePtr depthTarget;
	ResourcePtr renderTarget[ResourceLimits::MaxRenderTargets];
	
	Viewport viewport;

	ResourcePtr shaderProgram;
	ResourcePtr textures[ResourceLimits::MaxTextureSlots];
	TextureSampler textureSamplers[ResourceLimits::MaxTextureSamplerSlots];
	
	ResourcePtr indexBuffer;
	ResourcePtr vertexBuffers[ResourceLimits::MaxVertexBuffers];
	ResourcePtr uniformBuffers[ResourceLimits::MaxUniformBuffers];
	
	uint32 indexStart = 0;
	uint32 indexCount = 0;
	uint32 vertexStart = 0;
	uint32 vertexCount = 0;
	uint32 instanceCount = 1;
	
	VertexTopology vertexTopology = VertexTopology::TopologyUnknown;
	ResourcePtr vertexInputDescriptor;

	bool enableDepth = true;
	bool enableAlpha = true;
};

struct ComputeCall
{
	ResourcePtr computeShader;
	ResourcePtr textures[ResourceLimits::ResourceLimitMaxTextureSlots];
	ResourcePtr unorderedAccessViews[ResourceLimits::ResourceLimitMaxUnorderedAccessViews];
};

class IRenderContext
{
public:
	
	virtual void ResourceBufferUpdate(const ResourcePtr& rsc, const void* memory) = 0;
	virtual void ResourceBufferCopy(const ResourcePtr& src, ResourcePtr& dest) = 0;
	virtual void ResourceTextureUpdate(uint32 index, ResourcePtr& rsc, const void* memory) = 0;
	virtual void ResourceTextureCopy(const ResourcePtr& src, ResourcePtr& dest) = 0;
	virtual void ResourceTextureResolve(const ResourcePtr& src, ResourcePtr& dest) = 0;
	
	virtual void SubmitCommand(const DrawCall& command) = 0;
	virtual void SubmitCommand(const ComputeCall& command) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////

// gfxcore.h
// gfxqueue.h
// gfx.h

// ct/graphics/gfx.h

///////////////////////////////////////////////////////////////////////////////////////////
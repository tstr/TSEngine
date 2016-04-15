/*
	Renderer
*/

#pragma once

#include <C3E\core\maths.h>
#include <C3E\core\memory.h>
#include <C3E\core\threading.h>

#include <C3E\gfx\graphicsbase.h>

#include <ostream>
#include <istream>
#include <array>

namespace C3E
{
	class Graphics;
	class GraphicsBuffer;
	class GraphicsQueue;
	class GraphicsPacket;
	class Effect;
	class EffectResource;

	namespace ABI
	{
		class IRenderContext;
		class IRenderApi;
	}

	struct Multisampling
	{
		uint32 count = 1;
		uint32 quality = 0;

		Multisampling() {}
		Multisampling(uint32 c, uint32 q) : count(c), quality(q) {}
	};

	//Renderer
	class C3E_GFX_API Graphics
	{
	private:

		struct Impl;
		Impl* pImpl = nullptr;

	public:

		enum API
		{
			API_NULL,
			API_D3D11,
			API_D3D12
		};

		struct ApiInfo
		{
			const char* api = nullstr();
			const char* api_version = nullstr();
			API api_enum = API_NULL;

			const char* driver_name = nullstr();
			const char* driver_version = nullstr();
			const char* driver_vendor = nullstr();
		};

		struct DeviceInfo
		{
			wchar description[128];
			uint64 VideoMemory = 0;
			uint64 SystemMemory = 0;
			uint64 SharedSystemMemory = 0;
		};

		struct DebugInfo
		{
			int drawcalls = 0;
		};

		struct Viewport
		{
			uint32 height = 0;
			uint32 width = 0;
			bool fullscreen = false;
		};

		enum AnisotropicFiltering
		{
			AnisoptropyNone = 0,
			Anisotropy1x	= 1,
			Anisotropy2x	= 2,
			Anisotropy4x	= 4,
			Anisotropy8x	= 8,
			Anisotropy16x	= 16,
		};

		struct Configuration
		{
			uint64 windowId = 0;

			uint32 refreshrate = 60;
			bool vsync = false;

			uint32 AAcount = 1;
			uint32 AAquality = 0;

			AnisotropicFiltering anisotropicFiltering = AnisoptropyNone;

			Viewport viewport;
			bool debug = false;
		};

		void QueryApi(ApiInfo& i) const;
		void QueryDevice(uint32 slot, DeviceInfo& i) const;
		void QueryDebugStatistics(DebugInfo& i) const;

		Graphics(API id, const Configuration& cfg);
		~Graphics();

		void SetViewport(const Viewport& vp);
		void GetViewport(Viewport& vp);

		void VECTOR_CALL DrawBegin(Colour colour);
		void DrawEnd();
		//Dispatch a queue of draw calls to the gpu
		void DispatchQueue(GraphicsQueue* queue);


		ABI::IRenderApi* api() const;
		API apiEnum() const;
	};

	class C3E_GFX_API GraphicsRenderTarget
	{
	private:

		Graphics* m_renderer = nullptr;
		ResourceHandle m_rtHandle = 0;
		ResourceHandle m_dbHandle = 0;

		class RenderTargetResource;

		EffectResource* m_depthTexture = nullptr;
		EffectResource* m_colourTexture = nullptr;

	public:

		enum : intptr_t { DEFAULT_HANDLE = INTPTR_MAX };

		GraphicsRenderTarget() {}
		GraphicsRenderTarget(Graphics* renderer, uint32 w, uint32 h, ETextureFormat format = ETextureFormat::FormatRGBA32_FLOAT, Multisampling ms = Multisampling(), bool cubemap = false);
		GraphicsRenderTarget(const GraphicsRenderTarget& rt) = delete;
		~GraphicsRenderTarget();

		GraphicsRenderTarget(GraphicsRenderTarget&& rt)
		{
			m_renderer = rt.m_renderer;
			m_rtHandle = rt.m_rtHandle;
			m_dbHandle = rt.m_dbHandle;
			m_depthTexture = rt.m_depthTexture;
			m_colourTexture = rt.m_colourTexture;

			ZeroMemory(&rt, sizeof(GraphicsRenderTarget));
		}

		GraphicsRenderTarget& operator=(GraphicsRenderTarget&& rt)
		{
			m_renderer = rt.m_renderer;
			m_rtHandle = rt.m_rtHandle;
			m_dbHandle = rt.m_dbHandle;
			m_depthTexture = rt.m_depthTexture;
			m_colourTexture = rt.m_colourTexture;

			ZeroMemory(&rt, sizeof(GraphicsRenderTarget));
			return *this;
		}

		//Get raw api handles
		ResourceHandle hrt() const { return m_rtHandle; }
		ResourceHandle hdb() const { return m_dbHandle; }

		const EffectResource* AsTexture() const { return m_colourTexture; }
		const EffectResource* AsDepthTexture() const { return m_depthTexture; }
	};

	//Attribute indices for vertex class
	enum class VertexAttributeIndex
	{
		Position,		// 0,
		Colour,			// 1,
		Normal,			// 2,
		Texcoord,		// 3,
		Tangent,		// 4,
		Bitangent,		// 5,
		Reserved0,		// 6,
		Reserved1,		// 7,
		MaxAttributeCount
	};

	//Vertex attribute flags
	enum class VertexAttributes
	{
		Position = 0,
		Colour = 1,
		Normal = 2,
		Texcoord = 4,
		Tangent = 8,
		Bitangent = 16
	};

	enum class VertexTopology
	{
		TopologyUnknown,
		TopologyPointList,
		TopologyLineList,
		TopologyLineStrip,
		TopologyTriangleList,
		TopologyTriangleStrip
	};

	class Vertex
	{
	private:

		Vector m_attributes[(uint32)VertexAttributeIndex::MaxAttributeCount];

	public:

		Vertex() { memset(m_attributes, 0, sizeof(m_attributes)); }

		Vertex(const Vertex& cpy)
		{
			memcpy(m_attributes, cpy.m_attributes, sizeof(m_attributes));
		}

		Vector& operator[](uint32 index){ return m_attributes[index]; }
		const Vector& operator[](uint32 index) const { return m_attributes[index]; }

		Vector& get(uint32 index) { return m_attributes[index]; }
		const Vector& get(uint32 index) const { return m_attributes[index]; }
		void VECTOR_CALL set(uint32 index, Vector v) { m_attributes[index] = v; }
	};

	typedef uint32 Index;

	class C3E_GFX_API GraphicsBuffer
	{
	private:

		Graphics* m_renderer = nullptr;
		ResourceHandle m_hRsc = 0;

	public:

		GraphicsBuffer(
			Graphics* renderer,
			const void* mem,
			uint32 memsize,
			EResourceType buffer_type,
			EResourceUsage buffer_usage = EResourceUsage::UsageDefault
		);

		GraphicsBuffer(
			Graphics* renderer,
			const MemoryBuffer& buffer,
			EResourceType buffer_type,
			EResourceUsage buffer_usage = EResourceUsage::UsageDefault
		) :
		GraphicsBuffer(
			renderer,
			buffer.pointer(),
			(uint32)buffer.size(),
			buffer_type,
			buffer_usage
			)
		{}

		~GraphicsBuffer();

		ResourceHandle handle() const { return m_hRsc; }
	};

	class C3E_GFX_API EffectFactory
	{
	private:
		
		struct Impl;
		Impl* pImpl = nullptr;

	public:

		Graphics* GetRenderer() const;

		enum FXFlags
		{
			FX_DEBUG = 1
		};

		EffectFactory(Graphics* renderer, int flags = 0);
		~EffectFactory();

		void SetWorkingDirectory(const char* path);
		void SetOutputDirectory(const char* path);

		//Compile an effect from an effect manifest file
		int CompileEffectManifest(const char* file);
		void CreateEffect(const char* file, std::shared_ptr<Effect>& fx);

		int GetFlags() const;
		void SetFlags(int flags);
	};

	enum class EffectType
	{
		TypeUnknown,
		TypeMaterial,
		TypePostprocess
	};

	class C3E_GFX_API Effect
	{
	private:

		struct Impl;
		Impl* pImpl = nullptr;

	public:

		Effect(Graphics* renderer, std::istream& cache);
		~Effect();

		EffectType GetType() const;
		uint32 GetVertexAttributeMask() const;

		ResourceHandle GetShader(uint32 stage) const;
		ResourceHandle GetShaderInputDescriptor() const;
	};

	class C3E_GFX_API EffectResource
	{
	protected:

		Graphics* m_renderer = nullptr;
		ResourceHandle m_rsc = 0;

	public:

		EffectResource(Graphics* renderer) : m_renderer(renderer) {}

		ResourceHandle handle() const { return m_rsc; }
	};

	class GraphicsPacket
	{
	public:

		struct CommandBlock
		{
			ResourceHandle m_renderTarget = 0;
			ResourceHandle m_depthBuffer = 0;

			ResourceHandle m_vertexBuffers[ABI::MAX_VERTEX_BUFFERS];
			ResourceHandle m_shaderBuffers[ABI::MAX_SHADER_BUFFERS];
			ResourceHandle m_srvs[ABI::MAX_SRVS]; //Shader resource views
			ResourceHandle m_uavs[ABI::MAX_UAVS]; //Unordered access views
			ResourceHandle m_indexBuffer = 0;
			
			uint32 m_indexStart = 0;
			uint32 m_indexCount = 0;
			uint32 m_vertexStart = 0;
			uint32 m_vertexCount = 0;

			uint32 m_instances = 0;

			VertexTopology m_vertexTopology = VertexTopology::TopologyUnknown;
			ResourceHandle m_vertexShaderDescriptor = 0;

			ResourceHandle m_pixelShader = 0;
			ResourceHandle m_vertexShader = 0;
			ResourceHandle m_geometryShader = 0;

			bool m_indexed = false;
			bool m_instanced = false;
			bool m_nodepth = false;
			bool m_norender = false;
			bool m_blending = false;

			uint32 m_depthIndex = 0;
			uint32 m_renderIndex = 0;
		};

		//Compute shader command block
		struct CommandBlockCompute
		{
			ResourceHandle m_uavs[ABI::MAX_UAVS];
			ResourceHandle m_computeShader = 0;
		};

	private:

		uint8 m_bufferUpdateCount = 0;
		std::array<std::pair<GraphicsBuffer*, MemoryBuffer>, (8 + 8 + 1)> m_bufferUpdateQueue;

		CommandBlock block;

		int flags = 0;

	public:

		GraphicsPacket()
		{
			ZeroMemory(&block, sizeof(block));

			//block.m_renderTarget = (ResourceHandle)GraphicsRenderTarget::DEFAULT_HANDLE;
			//block.m_depthBuffer = (ResourceHandle)GraphicsRenderTarget::DEFAULT_HANDLE;
		}

		GraphicsPacket(const GraphicsPacket&) = default;

		friend class GraphicsQueue;

		void SetEffect(const Effect* effect)
		{
			C3E_ASSERT(effect);

			block.m_vertexShader = effect->GetShader(ABI::ShaderType::SHADER_TYPE_VERTEX);
			block.m_pixelShader = effect->GetShader(ABI::ShaderType::SHADER_TYPE_PIXEL);
			block.m_geometryShader = effect->GetShader(ABI::ShaderType::SHADER_TYPE_GEOMETRY);

			block.m_vertexShaderDescriptor = effect->GetShaderInputDescriptor();
		}

		void DisableDepth(bool d)
		{
			block.m_nodepth = d;
		}

		void DisableDraw(bool r)
		{
			block.m_norender = r;
		}

		void EnableBlending(bool b)
		{
			block.m_blending = b;
		}

		void SetRenderTarget(const GraphicsRenderTarget* rt, uint32 arrayIndex = 0)
		{
			block.m_renderTarget = (rt) ? rt->hrt() : nullptr; 
			block.m_depthBuffer = (rt) ? rt->hdb() : nullptr;

			block.m_depthIndex = arrayIndex;
			block.m_renderIndex = arrayIndex;
		}

		void UpdateBuffer(GraphicsBuffer* buf, const MemoryBuffer& b) { m_bufferUpdateQueue[m_bufferUpdateCount++] = std::make_pair(buf, b); }
		void SetVertexTopology(VertexTopology vt) { block.m_vertexTopology = vt; }

		void SetEffectResources(uint32 i, const EffectResource* res)
		{
			if (res)
				block.m_srvs[i] = res->handle();
		}

		void SetEffectBuffers(uint32 i, const GraphicsBuffer* ebuf)
		{
			if (ebuf)
				block.m_shaderBuffers[i] = ebuf->handle();
		}

		void SetVertexBuffers(uint32 i, const GraphicsBuffer* vbuf)
		{
			if (vbuf)
				block.m_vertexBuffers[i] = vbuf->handle();
		}

		void SetIndexBuffer(const GraphicsBuffer* ibuf)
		{
			if (ibuf)
				block.m_indexBuffer = ibuf->handle();
		}

		void Draw(uint32 vertex_start, uint32 vertex_count)
		{
			block.m_vertexStart = vertex_start;
			block.m_vertexCount = vertex_count;
			block.m_indexed = false;
		}

		void DrawIndexed(uint32 index_start, uint32 index_count)
		{
			block.m_indexStart = index_start;
			block.m_indexCount = index_count;
			block.m_indexed = true;
		}
	};

	class C3E_GFX_API GraphicsQueue
	{
	private:

		struct Impl;
		Impl* pImpl = nullptr;

	public:

		GraphicsQueue(Graphics* renderer);
		~GraphicsQueue();

		//Enqueue a drawcall command for sorting - not threadsafe
		void Enqueue(GraphicsPacket& statepacket);
		//Dispatch sorted queue to graphics api backend for processing
		void Dispatch();

		void UpdateBuffer(GraphicsBuffer* buf, const void* mem, uint32 size);
		void UpdateBuffer(GraphicsBuffer* buf, const MemoryBuffer& b);
		//void CopyResource(EffectResource* dest, const EffectResource* src);

		void ClearRenderTarget(const GraphicsRenderTarget* rt, const Colour&);
	};
	
	class C3E_GFX_API Texture : public EffectResource
	{
	private:

		bool m_success = false;
		char m_filename[MAX_PATH];

	public:

		Texture(Graphics* renderer, ResourceHandle hres);
		Texture(Graphics* renderer, const char* filename);
		~Texture();

		bool good() const { return m_success; }
	};

	class C3E_GFX_API TextureCube : public EffectResource
	{
	private:

		bool m_success = false;
		char m_filename[MAX_PATH];

	public:

		TextureCube(Graphics* renderer, const char* filename);
		~TextureCube();

		bool good() const { return m_success; }
	};
}
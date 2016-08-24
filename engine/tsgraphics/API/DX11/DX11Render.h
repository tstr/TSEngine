/*
	Render API

	DirectX 11 implementation of Render API
*/

#pragma once

#include <vector>
#include <tsgraphics/renderapi.h>
#include "dx11base.h"

namespace ts
{
	namespace dx11
	{
		/////////////////////////////////////////////////////////////////////////////////////////////////
		//Main D3D11 class
		/////////////////////////////////////////////////////////////////////////////////////////////////

		class DX11RenderApi : public ts::IRenderApi
		{
		private:

			HWND m_hwnd;
			SRenderApiConfiguration m_config;

			ComPtr<IDXGISwapChain> m_dxgiSwapchain;
			ComPtr<ID3D11Device> m_device;
			ComPtr<ID3D11DeviceContext> m_immediateContext;

			ComPtr<ID3D11RenderTargetView> m_swapChainRenderTarget;

		public:

			//Constructor
			DX11RenderApi(const SRenderApiConfiguration& cfg);
			~DX11RenderApi();

			ComPtr<ID3D11Device> getDevice() const { return m_device; }

			//Resource creation methods

			ERenderStatus createResourceBuffer(ResourceProxy& rsc, const SResourceBufferData& data) override;
			ERenderStatus createResourceTexture(ResourceProxy& rsc, const STextureResourceData* data, const STextureResourceDescriptor& desc) override;

			ERenderStatus createTargetDepth(ResourceProxy& view, const ResourceProxy& rsc, const STextureViewDescriptor& desc) override;
			ERenderStatus createTargetRender(ResourceProxy& view, const ResourceProxy& rsc, const STextureViewDescriptor& desc) override;

			ERenderStatus createViewTextureCube(ResourceProxy& view, const ResourceProxy& rsc, const STextureViewDescriptor& desc) override;
			ERenderStatus createViewTexture2D(ResourceProxy& view, const ResourceProxy& rsc, const STextureViewDescriptor& desc) override;
			ERenderStatus createViewTexture3D(ResourceProxy& view, const ResourceProxy& rsc) override;

			ERenderStatus createShader(ResourceProxy& shader, const void* bytecode, uint32 bytecodesize, EShaderStage stage) override;
			ERenderStatus createShaderInputDescriptor(ResourceProxy& rsc, const ResourceProxy& vertexshader, const ShaderInputDescriptor* descs, uint32 descnum) override;

			IRenderContext* createRenderContext() override;
			void destroyRenderContext(IRenderContext* context) override;

			void setWindowMode(EWindowMode mode) override;
			void setWindowDimensions(uint32 w, uint32 h) override;

			void drawBegin(const Vector& vec) override;
			void drawEnd() override;
		};

		/////////////////////////////////////////////////////////////////////////////////////////////////
		//Render context class
		/////////////////////////////////////////////////////////////////////////////////////////////////

		class DX11RenderContext : public IRenderContext
		{
		private:

			DX11RenderApi* m_api;
			ComPtr<ID3D11DeviceContext> m_context;
			SRenderCommand m_prevCommand;

		public:

			DX11RenderContext(DX11RenderApi* api);
			~DX11RenderContext();

			void resourceBufferUpdate(const ResourceProxy& rsc, const void* memory) override;
			void resourceBufferCopy(const ResourceProxy& src, ResourceProxy& dest) override;
			void resourceTextureUpdate(uint32 index, ResourceProxy& rsc, const void* memory) override;
			void resourceTextureCopy(const ResourceProxy& src, ResourceProxy& dest) override;
			void resourceTextureResolve(const ResourceProxy& src, ResourceProxy& dest) override;

			void clearRenderTarget(const ResourceProxy& renderview, const Vector& vec) override;
			void clearDepthTarget(const ResourceProxy& depthview, float depth) override;

			void execute(const SRenderCommand& command) override;
		};

		/////////////////////////////////////////////////////////////////////////////////////////////////
		//Shader compiler
		/////////////////////////////////////////////////////////////////////////////////////////////////

		class DX11ShaderCompiler : public ts::IShaderCompiler
		{
		private:

			HMODULE m_d3dcompiler;

		public:

			DX11ShaderCompiler();
			~DX11ShaderCompiler();

			bool compile(const char* str, const SShaderCompileConfig& options, MemoryBuffer& bytecode) override;
			bool compileFromFile(const Path& file, const SShaderCompileConfig& options, MemoryBuffer& bytecode) override;
		
		};

		/////////////////////////////////////////////////////////////////////////////////////////////////

		class DX11RenderAdapterFactory : public ts::IRenderAdapterFactory
		{
		private:

			ComPtr<IDXGIFactory> m_dxgiFactory;
			std::vector<SRenderAdapterDesc> m_dxgiAdapterDescs;

		public:

			DX11RenderAdapterFactory();

			uint32 getAdapterCount() const override { return (uint32)m_dxgiAdapterDescs.size(); }

			bool enumAdapter(uint32 idx, SRenderAdapterDesc& desc) const override
			{
				if (idx >= getAdapterCount())
				{
					return false;
				}

				desc = m_dxgiAdapterDescs.at(idx);

				return true;
			}
		};

		/////////////////////////////////////////////////////////////////////////////////////////////////
		//Fundamental resource types
		/////////////////////////////////////////////////////////////////////////////////////////////////

		//Base class for dx11 implementation resources
		class IDX11RenderResource : public IRenderResource
		{
		protected:

			IRenderApi* m_api = nullptr;
			EResourceType m_type;

		public:

			IDX11RenderResource(DX11RenderApi* api, EResourceType type) :
				m_api((IRenderApi*)api),
				m_type(type)
			{}

			virtual ~IDX11RenderResource() {}

			EResourceType getType() const override { return m_type; }
			IRenderApi* getApi() const override { return m_api; }
			void release() const override { delete this; }
		};


		class DX11Buffer : public IDX11RenderResource
		{
		private:

			ComPtr<ID3D11Buffer> m_buffer;

		public:

			static DX11Buffer* upcast(IRenderResource* ptr) { return (DX11Buffer*)ptr; }

			DX11Buffer(DX11RenderApi* api, ComPtr<ID3D11Buffer> buf) :
				IDX11RenderResource(api, EResourceType::eResourceBuffer),
				m_buffer(buf)
			{}

			~DX11Buffer() {}

			ID3D11Buffer* get() const { return m_buffer.Get(); }
		};


		class DX11Texture : public IDX11RenderResource
		{
		private:

			ComPtr<ID3D11Resource> m_texture;
			STextureResourceDescriptor m_textureDesc;

		public:

			static DX11Texture* upcast(IRenderResource* ptr) { return (DX11Texture*)ptr; }

			DX11Texture(DX11RenderApi* api, ComPtr<ID3D11Resource> i, const STextureResourceDescriptor& desc) :
				IDX11RenderResource(api, eResourceTexture),
				m_texture(i),
				m_textureDesc(desc)
			{}

			~DX11Texture() {}

			void getDesc(STextureResourceDescriptor& desc) const
			{
				desc = m_textureDesc;
			}

			ID3D11Resource* get() const { return m_texture.Get(); }
		};


		class DX11View : public IDX11RenderResource
		{
		private:

			ComPtr<ID3D11View> m_view;
			STextureViewDescriptor m_viewDesc;

		public:

			DX11View(DX11RenderApi* api, ComPtr<ID3D11ShaderResourceView> v, const STextureViewDescriptor& desc) :
				IDX11RenderResource(api, eResourceViewTexture),
				m_view(v),
				m_viewDesc(desc)
			{}

			DX11View(DX11RenderApi* api, ComPtr<ID3D11DepthStencilView> v, const STextureViewDescriptor& desc) :
				IDX11RenderResource(api, eResourceViewDepth),
				m_view(v),
				m_viewDesc(desc)
			{}

			DX11View(DX11RenderApi* api, ComPtr<ID3D11RenderTargetView> v, const STextureViewDescriptor& desc) :
				IDX11RenderResource(api, eResourceViewRender),
				m_view(v),
				m_viewDesc(desc)
			{}

			static DX11View* upcast(IRenderResource* ptr) { return (DX11View*)ptr; }

			bool isRTV() const { return m_type == eResourceViewRender; }
			bool isSRV() const { return m_type == eResourceViewTexture; }
			bool isDTV() const { return m_type == eResourceViewDepth; }

			ID3D11View* get() const { return m_view.Get(); }

			void getDesc(STextureViewDescriptor& desc) const
			{
				desc = m_viewDesc;
			}
		};

		class DX11Shader : public IDX11RenderResource
		{
		private:

			ComPtr<ID3D11DeviceChild> m_shaderInterface;
			EShaderStage m_shaderStage;
			MemoryBuffer m_shaderBytecode;

		public:

			static DX11Shader* upcast(IRenderResource* ptr) { return (DX11Shader*)ptr; }

			DX11Shader(DX11RenderApi* api, ComPtr<ID3D11VertexShader> s, MemoryBuffer&& buf) :
				IDX11RenderResource(api, eResourceShader),
				m_shaderInterface((ID3D11DeviceChild*)s.Get()),
				m_shaderStage(eShaderStageVertex),
				m_shaderBytecode(buf)
			{}

			DX11Shader(DX11RenderApi* api, ComPtr<ID3D11PixelShader> s, MemoryBuffer&& buf) :
				IDX11RenderResource(api, eResourceShader),
				m_shaderInterface((ID3D11DeviceChild*)s.Get()),
				m_shaderStage(eShaderStagePixel),
				m_shaderBytecode(buf)
			{}

			DX11Shader(DX11RenderApi* api, ComPtr<ID3D11GeometryShader> s, MemoryBuffer&& buf) :
				IDX11RenderResource(api, eResourceShader),
				m_shaderInterface((ID3D11DeviceChild*)s.Get()),
				m_shaderStage(eShaderStageGeometry),
				m_shaderBytecode(buf)
			{}

			DX11Shader(DX11RenderApi* api, ComPtr<ID3D11HullShader> s, MemoryBuffer&& buf) :
				IDX11RenderResource(api, eResourceShader),
				m_shaderInterface((ID3D11DeviceChild*)s.Get()),
				m_shaderStage(eShaderStageHull),
				m_shaderBytecode(buf)
			{}

			DX11Shader(DX11RenderApi* api, ComPtr<ID3D11DomainShader> s, MemoryBuffer&& buf) :
				IDX11RenderResource(api, eResourceShader),
				m_shaderInterface((ID3D11DeviceChild*)s.Get()),
				m_shaderStage(eShaderStageDomain),
				m_shaderBytecode(buf)
			{}

			DX11Shader(DX11RenderApi* api, ComPtr<ID3D11ComputeShader> s, MemoryBuffer&& buf) :
				IDX11RenderResource(api, eResourceShader),
				m_shaderInterface((ID3D11DeviceChild*)s.Get()),
				m_shaderStage(eShaderStageCompute),
				m_shaderBytecode(buf)
			{}

			ID3D11DeviceChild* getShader() const { return m_shaderInterface.Get(); }
			EShaderStage getShaderType() const { return m_shaderStage; }

			void getShaderBytecode(void** bytecode, uint32& bytecodesize)
			{
				*bytecode = m_shaderBytecode.pointer();
				bytecodesize = (UINT)m_shaderBytecode.size();
			}
		};

		class DX11ShaderInputDescriptor : public IDX11RenderResource
		{
		private:

			ComPtr<ID3D11InputLayout> m_inputLayout;

		public:

			static DX11ShaderInputDescriptor* upcast(IRenderResource* rsc) { return (DX11ShaderInputDescriptor*)rsc; }

			DX11ShaderInputDescriptor(DX11RenderApi* api, ComPtr<ID3D11InputLayout> layout) :
				IDX11RenderResource(api, EResourceType::eResourceUnknown),
				m_inputLayout(layout)
			{}

			ID3D11InputLayout* getLayout() const { return m_inputLayout.Get(); }
		};

		/////////////////////////////////////////////////////////////////////////////////////////////////
	}
}
/*
	Render API

	DirectX 11 implementation of Render API
*/

#pragma once

#include <vector>
#include <atomic>
#include <tsgraphics/api/renderapi.h>

#include "base.h"

namespace ts
{
	class D3D11RenderContext;

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//Main D3D11 class
	/////////////////////////////////////////////////////////////////////////////////////////////////

	class D3D11Render : public ts::IRender
	{
	private:

		HWND m_hwnd;
		UINT m_apiFlags = 0;
		UINT m_swapChainFlags = 0;

		ComPtr<IDXGIFactory> m_dxgiFactory;
		ComPtr<IDXGIOutput> m_dxgiOutput;
		ComPtr<IDXGIAdapter> m_dxgiAdapter;
		ComPtr<IDXGISwapChain> m_dxgiSwapchain;
		ComPtr<ID3D11Device> m_device;
		ComPtr<ID3D11DeviceContext> m_immediateContext;

		ComPtr<ID3D11BlendState> m_blendState;
		ComPtr<ID3D11RasterizerState> m_rasterizerState;
		ComPtr<ID3D11DepthStencilState> m_depthStencilState;
				
		std::atomic<bool> m_drawActive;
		std::atomic<bool> m_displayNeedRebuild;
		SDisplayConfig m_cachedDisplayConfig;
		void tryRebuildDisplay();
		void doRebuildDisplay();
		
		//All allocated render contexts
		std::vector<D3D11RenderContext*> m_renderContexts;
		//Number of drawcalls per frame - for debugging
		std::atomic<uint32> m_drawCallCounter;

		bool getMultisampleQuality(DXGI_SAMPLE_DESC& sampledesc);

	public:
		
		D3D11Render(const SRenderApiConfig& cfg);
		~D3D11Render();

		//Internal methods
		ComPtr<ID3D11Device> getDevice() const { return m_device; }

		ComPtr<ID3D11BlendState> getBlendState() const { return m_blendState; }
		ComPtr<ID3D11RasterizerState> getRasterizerState() const { return m_rasterizerState; }
		ComPtr<ID3D11DepthStencilState> getDepthStencilState() const { return m_depthStencilState; }

		void incrementDrawCallCounter() { m_drawCallCounter++; }

		//Resource methods
		ERenderStatus createResourceBuffer(
			HBuffer& rsc,
			const SBufferResourceData& data
		) override;
		
		ERenderStatus createResourceTexture(
			HTexture& rsc,
			const STextureResourceData* data,
			const STextureResourceDesc& desc
		) override;
		
		ERenderStatus createShader(
			HShader& shader,
			const void* bytecode,
			uint32 bytecodesize,
			EShaderStage stage
		) override;
		
		ERenderStatus createTarget(
			HTarget& target,
			const HTexture* renderTexture,
			const uint32* renderTextureIndices,
			uint32 renderTextureCount,
			HTexture deptTextureProxy,
			uint32 deptTextureProxyIndex
		) override;
		
		void destroyBuffer(HBuffer buffer) override;
		void destroyTexture(HTexture texture) override;
		void destroyShader(HShader shader) override;
		void destroyTarget(HTarget target) override;
		
		//Command methods
		ERenderStatus createDrawCommand(HDrawCmd& cmd, const SDrawCommand& desc) override;
		void destroyDrawCommand(HDrawCmd cmd) override;
		
		//Render context
		void createContext(IRenderContext** context) override;
		void destroyContext(IRenderContext* context) override;
		
		//Window methods
		void setDisplayConfiguration(const SDisplayConfig& displayCfg) override;
		void getDisplayConfiguration(SDisplayConfig& displayCfg) override;
		void getDisplayTexture(HTexture& tex) override;

		//Statistics
		void getDrawStatistics(SRenderStatistics& stats) override;

		void drawBegin(const Vector& vec) override;
		void drawEnd(IRenderContext** contexts, uint32 numContexts) override;
	};
	
	/////////////////////////////////////////////////////////////////////////////////////////////////
}
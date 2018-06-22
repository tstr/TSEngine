/*
	Render API

	DirectX 11 implementation of Render API
*/

#pragma once

#include <vector>
#include <atomic>
#include <mutex>
#include <tsgraphics/Device.h>

#include "Base.h"
#include "HandleTarget.h"
#include "StateManager.h"

namespace ts
{
	class D3D11Context;

	/////////////////////////////////////////////////////////////////////////////////////////////////
	//Main D3D11 class
	/////////////////////////////////////////////////////////////////////////////////////////////////

	class D3D11 : public RenderDevice
	{
	public:
		
		D3D11(const RenderDeviceConfig& cfg);
		~D3D11();

		//Internal methods
		ComPtr<ID3D11Device> getDevice() const { return m_device; }

		ComPtr<ID3D11BlendState> getBlendState() const { return m_blendState; }
		ComPtr<ID3D11RasterizerState> getRasterizerState() const { return m_rasterizerState; }
		ComPtr<ID3D11DepthStencilState> getDepthStencilState() const { return m_depthStencilState; }

		void incrementDrawCallCounter() { m_drawCallCounter++; }

		RenderContext* getContext() override;
		void execute(RenderContext* context) override;

		//Display methods
		void setDisplayConfiguration(const DisplayConfig& displayCfg) override;
		void getDisplayConfiguration(DisplayConfig& displayCfg) override;
		ResourceHandle getDisplayTarget() override;

		//Query device
		void queryStats(RenderStats& stats) override;
		void queryInfo(DeviceInfo& info) override;

		//Resources
		ResourceHandle createEmptyResource(ResourceHandle recycle) override;
		ResourceHandle createResourceBuffer(const ResourceData& data, const BufferResourceInfo& info, ResourceHandle recycle) override;
		ResourceHandle createResourceImage(const ResourceData* data, const ImageResourceInfo& info, ResourceHandle recycle) override;
		ResourceSetHandle createResourceSet(const ResourceSetInfo& info, ResourceSetHandle recycle) override;
		ShaderHandle createShader(const ShaderCreateInfo& info) override;
		PipelineHandle createPipeline(ShaderHandle program, const PipelineCreateInfo& info) override;
		TargetHandle createTarget(const TargetCreateInfo& info, TargetHandle recycle) override;
		CommandHandle createCommand(const DrawCommandInfo& cmd, CommandHandle recycle) override;

		//Destroy device objects
		void destroy(ResourceHandle rsc) override;
		void destroy(ResourceSetHandle set) override;
		void destroy(ShaderHandle shader) override;
		void destroy(PipelineHandle state) override;
		void destroy(TargetHandle pass) override;
		void destroy(CommandHandle cmd) override;

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

		D3D11Target m_displayTarget;
		D3D11StateManager m_stateManager;

		std::atomic<bool> m_drawActive;
		std::mutex m_drawMutex;

		//Swapchain methods
		void rebuildSwapChain(DXGI_SWAP_CHAIN_DESC& scDesc);
		HRESULT translateSwapChainDesc(const DisplayConfig& displayCfg, DXGI_SWAP_CHAIN_DESC& scDesc);
		void initDisplayTarget();

		//All allocated render contexts
		std::vector<D3D11Context*> m_renderContexts;
		//Number of drawcalls per frame - for debugging
		std::atomic<uint32> m_drawCallCounter;

		bool getMultisampleQuality(DXGI_SAMPLE_DESC& sampledesc);

		ComPtr<ID3D11BlendState> m_blendState;
		ComPtr<ID3D11RasterizerState> m_rasterizerState;
		ComPtr<ID3D11DepthStencilState> m_depthStencilState;
	};
	
	/////////////////////////////////////////////////////////////////////////////////////////////////
}
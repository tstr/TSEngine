/*
	Render API

	DirectX 11 implementation of Render Driver
*/

#pragma once

#include <vector>
#include <atomic>

#include "Base.h"
#include "Context.h"
#include "HandleResource.h"
#include "StateManager.h"

namespace ts
{
	class Dx11Context;

	/*
	
		Main D3D11 driver class
	
	*/
	class Dx11 : public RenderDevice
	{
	public:
		
		Dx11(const RenderDeviceConfig& cfg);
		~Dx11();

		RenderContext* context() override { return &m_context; }
		void commit() override;

		//Display methods
		void setDisplayConfiguration(const DisplayConfig& displayCfg) override;
		void getDisplayConfiguration(DisplayConfig& displayCfg) override;
		ResourceHandle getDisplayTarget() override { return DxResource::downcast(&m_displayResourceProxy); }

		//Query device
		void queryStats(RenderStats& stats) override;
		void queryInfo(RenderDeviceInfo& info) override;

		//Resources
		RPtr<ResourceHandle> createEmptyResource(ResourceHandle recycle) override;
		RPtr<ResourceHandle> createResourceBuffer(const ResourceData& data, const BufferResourceInfo& info, ResourceHandle recycle) override;
		RPtr<ResourceHandle> createResourceImage(const ResourceData* data, const ImageResourceInfo& info, ResourceHandle recycle) override;
		//Resource set
		RPtr<ResourceSetHandle> createResourceSet(const ResourceSetCreateInfo& info, ResourceSetHandle recycle) override;
		//Pipeline state
		RPtr<ShaderHandle> createShader(const ShaderCreateInfo& info) override;
		RPtr<PipelineHandle> createPipeline(ShaderHandle program, const PipelineCreateInfo& info) override;
		//Output target
		RPtr<TargetHandle> createTarget(const TargetCreateInfo& info, TargetHandle recycle) override;
		//Commands
		RPtr<CommandHandle> createCommand(const DrawCommandInfo& cmd, CommandHandle recycle) override;

		//Destroy device objects
		void destroy(ResourceHandle rsc) override;
		void destroy(ResourceSetHandle set) override;
		void destroy(ShaderHandle shader) override;
		void destroy(PipelineHandle state) override;
		void destroy(TargetHandle pass) override;
		void destroy(CommandHandle cmd) override;

		//Internal methods
		ComPtr<ID3D11Device> getDevice() const { return m_device; }
		
		void incrementDrawCallCounter() { m_drawCallCounter++; }

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

		Dx11Context m_context;

		DxResource m_displayResourceProxy;
		DxStateManager m_stateManager;

		//Swapchain methods
		void rebuildSwapChain(DXGI_SWAP_CHAIN_DESC& scDesc);
		HRESULT translateSwapChainDesc(const DisplayConfig& displayCfg, DXGI_SWAP_CHAIN_DESC& scDesc);
		void updateDisplayResource();

		//Number of drawcalls per frame - for debugging
		std::atomic<uint32> m_drawCallCounter;

		bool getMultisampleQuality(DXGI_SAMPLE_DESC& sampledesc);
	};
	
	/////////////////////////////////////////////////////////////////////////////////////////////////
}
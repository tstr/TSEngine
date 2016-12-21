/*
	Render API

	D3D11Render display methods:

		In this context the term "display" means the same thing as a DXGI swap-chain.
*/

#include "render.h"

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Display rebuild methods
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D3D11Render::tryRebuildDisplay()
{
	//If display is marked for rebuild
	if (m_displayNeedRebuild.exchange(false))
	{
		doRebuildDisplay();
	}
}

void D3D11Render::doRebuildDisplay()
{
	//Copy cached display configuration
	auto cfg(m_cachedDisplayConfig);

	DXGI_SWAP_CHAIN_DESC desc;
	m_dxgiSwapchain->GetDesc(&desc);

	desc.Windowed = !cfg.fullscreen;
	
	if (cfg.multisampleCount)
	{
		desc.SampleDesc.Count = cfg.multisampleCount;
		getMultisampleQuality(desc.SampleDesc);
	}
	if (cfg.resolutionH || cfg.resolutionW)
	{
		desc.BufferDesc.Height = cfg.resolutionH;
		desc.BufferDesc.Width = cfg.resolutionW;
	}
	
	desc.Flags = m_swapChainFlags;

	//Release swapchain
	m_dxgiSwapchain.Reset();

	//Recreate swapchain
	tsassert(SUCCEEDED(m_dxgiFactory->CreateSwapChain((IUnknown*)m_device.Get(), &desc, m_dxgiSwapchain.GetAddressOf())));

	m_dxgiFactory->MakeWindowAssociation(
		m_hwnd,
		DXGI_MWA_NO_WINDOW_CHANGES
	);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Gets/sets the configuration of the display (swapchain)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D3D11Render::setDisplayConfiguration(const SDisplayConfig& displayCfg)
{
	//If the renderapi is currently in a drawing state then we must rebuild the display at a later time
	if (m_drawActive)
	{
		//If display is already marked for rebuild then override previous changes by first delaying the rebuild
		m_displayNeedRebuild.exchange(false);
		
		m_cachedDisplayConfig = displayCfg;
		
		//Mark display as ready for rebuild (again if it was previously marked)
		m_displayNeedRebuild.store(true);
	}
	else
	{
		doRebuildDisplay();
	}
}

//todo: make thread safe
void D3D11Render::getDisplayConfiguration(SDisplayConfig& displayCfg)
{
	DXGI_SWAP_CHAIN_DESC desc;
	HRESULT hr = m_dxgiSwapchain->GetDesc(&desc);

	if (SUCCEEDED(hr))
	{
		displayCfg.fullscreen = !desc.Windowed;
		displayCfg.multisampleCount = (uint8)desc.SampleDesc.Count;
		displayCfg.resolutionH = (uint16)desc.BufferDesc.Height;
		displayCfg.resolutionW = (uint16)desc.BufferDesc.Width;
	}
	else
	{
		tswarn("DXGISwapChain::GetDesc() failed with HRESULT %", hr);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D3D11Render::getDisplayTexture(HTexture& tex)
{
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

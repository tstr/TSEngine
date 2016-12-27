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
	//Release targets associated with swapchain
	m_displayTarget.reset();

	//Recreate swapchain
	tsassert(SUCCEEDED(m_dxgiFactory->CreateSwapChain((IUnknown*)m_device.Get(), &desc, m_dxgiSwapchain.GetAddressOf())));

	m_dxgiFactory->MakeWindowAssociation(
		m_hwnd,
		DXGI_MWA_NO_WINDOW_CHANGES
	);

	//Recreate render targets associated with display
	initDisplayTarget();
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
		m_cachedDisplayConfig = displayCfg;
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
//	Display target methods
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//creates a new 
void D3D11Render::initDisplayTarget()
{
	m_displayTarget.reset();
	
	ComPtr<ID3D11Texture2D> backbuffer;
	ComPtr<ID3D11Texture2D> depthbuffer;
	ComPtr<ID3D11RenderTargetView> rtv;
	ComPtr<ID3D11DepthStencilView> dsv;
	
	//Get ID3D11Texture2D resource from swapchain backbuffer
	HRESULT hr = m_dxgiSwapchain->GetBuffer(0, IID_OF(ID3D11Texture2D), (void**)backbuffer.GetAddressOf());
	tsassert(SUCCEEDED(hr));

	hr = m_device->CreateRenderTargetView(backbuffer.Get(), nullptr, rtv.GetAddressOf());
	tsassert(SUCCEEDED(hr));

	//Backbuffer description
	D3D11_TEXTURE2D_DESC bbdesc;
	backbuffer->GetDesc(&bbdesc);
	
	//Depth stencil description
	D3D11_TEXTURE2D_DESC dsdesc;
	dsdesc = bbdesc;
	dsdesc.ArraySize = 1;
	dsdesc.Format = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
	dsdesc.MipLevels = 1;
	dsdesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	hr = m_device->CreateTexture2D(&dsdesc, nullptr, depthbuffer.GetAddressOf());
	tsassert(SUCCEEDED(hr));
	
	hr = m_device->CreateDepthStencilView(depthbuffer.Get(), nullptr, dsv.GetAddressOf());
	tsassert(SUCCEEDED(hr));

	m_displayTarget = D3D11Target(rtv.GetAddressOf(), 1, dsv.Get());
}

void D3D11Render::getDisplayTarget(HTarget& target)
{
	target = D3D11Target::downcast(&m_displayTarget);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

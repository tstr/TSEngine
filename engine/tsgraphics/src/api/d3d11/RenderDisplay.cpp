/*
	Render API

	D3D11Render display methods:

		In this context the term "display" means the same thing as a DXGI swap-chain.
*/

#include "render.h"

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Gets/sets the configuration of the display (swapchain)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D3D11Render::setDisplayConfiguration(const SDisplayConfig& displayCfg)
{
	//Lock
	std::lock_guard<std::mutex> lk(m_drawMutex);

	DXGI_SWAP_CHAIN_DESC newDesc;
	DXGI_SWAP_CHAIN_DESC curDesc;

	HRESULT hr = S_OK;

	//Get current swap chain configuration
	tsassert(SUCCEEDED(m_dxgiSwapchain->GetDesc(&curDesc)));

	//Translate desired display configuration to a DXGI_SWAP_CHAIN_DESC
	tsassert(SUCCEEDED(translateSwapChainDesc(displayCfg, newDesc)));

	bool ms_differs = curDesc.SampleDesc.Count != newDesc.SampleDesc.Count;
	bool res_differs = (curDesc.BufferDesc.Width != newDesc.BufferDesc.Width) || (curDesc.BufferDesc.Height != newDesc.BufferDesc.Height);
	bool mode_differs = curDesc.Windowed != newDesc.Windowed;

	//If multisampling count differs
	if (ms_differs)
	{
		rebuildSwapChain(newDesc);
	}
	//If fullscreen state differs
	else if (mode_differs)
	{
		//Call resizetarget before entering fullscreen
		//if (FAILED(hr = m_dxgiSwapchain->ResizeTarget(&newDesc.BufferDesc)))
		//	tserror("unable to resize IDXGISwapChain target. Error: \"%\"", _com_error(hr).ErrorMessage());

		if (FAILED(hr = m_dxgiSwapchain->SetFullscreenState(!newDesc.Windowed, nullptr)))
			tserror("failed to change fullscreen mode. Error: \"%\"", _com_error(hr).ErrorMessage());
	}
	//If resolution state differs
	else if (res_differs)
	{
		//Release targets associated with swapchain
		m_displayTarget.reset();

		if (FAILED(
			hr = m_dxgiSwapchain->ResizeBuffers(
				2, //Number of back and front buffers
				newDesc.BufferDesc.Width,
				newDesc.BufferDesc.Height,
				newDesc.BufferDesc.Format,
				m_swapChainFlags
			)
		))
		{
			tserror("unable to resize IDXGISwapChain buffers. Error: \"%\"", _com_error(hr).ErrorMessage());
		}

		//Rebuild targets aassociated with swapchain
		initDisplayTarget();
	}
}

//todo: make thread safe
void D3D11Render::getDisplayConfiguration(SDisplayConfig& displayCfg)
{
	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(DXGI_SWAP_CHAIN_DESC));

	HRESULT hr = m_dxgiSwapchain->GetDesc(&desc);

	if (SUCCEEDED(hr))
	{
		displayCfg.fullscreen = !desc.Windowed;
		displayCfg.multisampleLevel = (uint8)desc.SampleDesc.Count;
		displayCfg.resolutionH = (uint16)desc.BufferDesc.Height;
		displayCfg.resolutionW = (uint16)desc.BufferDesc.Width;
	}
	else
	{
		tswarn("DXGISwapChain::GetDesc() failed with HRESULT %", hr);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D3D11Render::rebuildSwapChain(DXGI_SWAP_CHAIN_DESC& scDesc)
{
	//Release swapchain
	m_dxgiSwapchain.Reset();
	//Release targets associated with swapchain
	m_displayTarget.reset();

	//Recreate swapchain
	tsassert(SUCCEEDED(m_dxgiFactory->CreateSwapChain((IUnknown*)m_device.Get(), &scDesc, m_dxgiSwapchain.GetAddressOf())));

	m_dxgiFactory->MakeWindowAssociation(
		m_hwnd,
		DXGI_MWA_NO_WINDOW_CHANGES
	);

	//Recreate render targets associated with display
	initDisplayTarget();
}

HRESULT D3D11Render::translateSwapChainDesc(const SDisplayConfig& displayCfg, DXGI_SWAP_CHAIN_DESC& desc)
{
	HRESULT hr = m_dxgiSwapchain->GetDesc(&desc);

	desc.Windowed = !displayCfg.fullscreen;

	if (displayCfg.multisampleLevel > 0)
	{
		desc.SampleDesc.Count = displayCfg.multisampleLevel;
		getMultisampleQuality(desc.SampleDesc);
	}

	if (displayCfg.resolutionH > 0)
	{
		desc.BufferDesc.Height = displayCfg.resolutionH;
	}

	if (displayCfg.resolutionW > 0)
	{
		desc.BufferDesc.Width = displayCfg.resolutionW;
	}

	desc.Flags = m_swapChainFlags;

	return hr;
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

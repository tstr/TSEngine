/*
	Render API

	Driver display methods:

	In this context the term "display" means the same thing as a DXGI swap-chain.
*/

#include "render.h"

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Gets/sets the configuration of the display (swapchain)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Dx11::setDisplayConfiguration(const DisplayConfig& displayCfg)
{
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
		m_displayResourceProxy.reset();

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

		//Update resource proxy
		updateDisplayResource();
	}
}

//todo: make thread safe
void Dx11::getDisplayConfiguration(DisplayConfig& displayCfg)
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

void Dx11::rebuildSwapChain(DXGI_SWAP_CHAIN_DESC& scDesc)
{
	//Release swapchain
	m_dxgiSwapchain.Reset();
	//Release targets associated with swapchain
	m_displayResourceProxy.reset();

	//Recreate swapchain
	tsassert(SUCCEEDED(m_dxgiFactory->CreateSwapChain((IUnknown*)m_device.Get(), &scDesc, m_dxgiSwapchain.GetAddressOf())));

	m_dxgiFactory->MakeWindowAssociation(
		m_hwnd,
		DXGI_MWA_NO_WINDOW_CHANGES
	);

	//Recreate render targets associated with display
	updateDisplayResource();
}

HRESULT Dx11::translateSwapChainDesc(const DisplayConfig& displayCfg, DXGI_SWAP_CHAIN_DESC& desc)
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

//creates a new 
void Dx11::updateDisplayResource()
{
	ComPtr<ID3D11Texture2D> backbuffer;
	HRESULT hr = m_dxgiSwapchain->GetBuffer(0, IID_OF(ID3D11Texture2D), (void**)backbuffer.GetAddressOf());
	tsassert(SUCCEEDED(hr));

	m_displayResourceProxy = DxResource(backbuffer);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

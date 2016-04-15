/*
	DirectX-11 Render target source
*/

#include "pch.h"
#include "DX11RenderTarget.h"
#include <C3E\core\strings.h>

using namespace std;
using namespace C3E;
using namespace C3E::DX11;

//////////////////////////////////////////////////////////////////////////////////////////////

DX11RenderTarget::DX11RenderTarget(
	ID3D11Device* device,
	uint32 w, uint32 h,
	DXGI_FORMAT format,
	DXGI_SAMPLE_DESC sampling,
	bool cubemap,
	bool mipmaps
)
{
	m_device = ComPtr<ID3D11Device>(device);

	m_sampling = sampling;
	m_height = h;
	m_width = w;
	m_format = format;
	m_is_cubemap = cubemap;
	m_use_mipmaps = mipmaps;

	C3E_ASSERT(GenerateRenderTarget());
}

DX11RenderTarget::DX11RenderTarget(IDXGISwapChain* _swapchain)
{
	m_renderTargetViews.resize(1);

	m_dxgiSwapchain = ComPtr<IDXGISwapChain>(_swapchain);

	if (FAILED(m_dxgiSwapchain->GetDevice(IID_OF(ID3D11Device), (void**)m_device.GetAddressOf())))
	{
		throw exception();
	}

	DXGI_SWAP_CHAIN_DESC desc;
	m_dxgiSwapchain->GetDesc(&desc);
	m_sampling = desc.SampleDesc;
	m_width = desc.BufferDesc.Width;
	m_height = desc.BufferDesc.Height;

	ComPtr<ID3D11Texture2D> backbuffer;
	m_dxgiSwapchain->GetBuffer(0, IID_OF(ID3D11Texture2D), (void**)backbuffer.GetAddressOf());

	if (FAILED(m_device->CreateRenderTargetView(backbuffer.Get(), nullptr, m_renderTargetViews[0].GetAddressOf())))
	{
		throw exception();
	}

	SetDebugObjectName(m_renderTargetViews[0].Get(), "D3D11:RenderTarget-SwapChain-View");
}

DX11RenderTarget::~DX11RenderTarget() {}

//////////////////////////////////////////////////////////////////////////////////////////////

bool DX11RenderTarget::ResizeView(uint32 w, uint32 h)
{
	m_height = h;
	m_width = w;

	if (m_dxgiSwapchain.Get())
	{
		//Release old resources
		RenderTargetViewReset();
		ShaderViewReset();

		m_renderTargetViews.resize(1);

		w = max(1u, w);
		h = max(1u, h);

		DXGI_SWAP_CHAIN_DESC desc;
		m_dxgiSwapchain->GetDesc(&desc);

		//Resize swapchain - DXGI_FORMAT_R8G8B8A8_UNORM
		if (FAILED(m_dxgiSwapchain->ResizeBuffers(2, w, h, DXGI_FORMAT_UNKNOWN, desc.Flags)))
			return false;

		//Temporary Backbuffer Texture
		ComPtr<ID3D11Texture2D> backbuffer;

		// New back-buffer and render-target view
		m_dxgiSwapchain->GetBuffer(0, IID_OF(ID3D11Texture2D), (void**)backbuffer.GetAddressOf());
		C3E_ASSERT(SUCCEEDED(m_device->CreateRenderTargetView(backbuffer.Get(), nullptr, m_renderTargetViews[0].GetAddressOf())));
	}
	else
	{
		GenerateRenderTarget();
	}

	return true;
}

bool DX11RenderTarget::GenerateRenderTarget()
{
	RenderTargetViewReset();
	ShaderViewReset();

	D3D11_TEXTURE2D_DESC texDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));

	HRESULT hr = S_OK;

	uint32 arraysize = (m_is_cubemap) ? 6 : 1;

	// Initialize the render target texture description.
	ZeroMemory(&texDesc, sizeof(D3D11_TEXTURE2D_DESC));

	// Setup the render target texture description.
	texDesc.Width = m_width;
	texDesc.Height = m_height;
	texDesc.MipLevels = (m_use_mipmaps) ? GetNumMipLevels(m_width, m_height) : 1;
	texDesc.ArraySize = arraysize;
	texDesc.Format = m_format;
	//texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	texDesc.SampleDesc = m_sampling;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags |= (m_is_cubemap) ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
	texDesc.MiscFlags |= (m_use_mipmaps) ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;

	// Create the render target texture.
	hr = m_device->CreateTexture2D(&texDesc, nullptr, m_renderTargetTexture.GetAddressOf());
	if (FAILED(hr))
	{
		return false;
	}

	m_renderTargetViews.resize(arraysize);

	if (arraysize > 1)
	{
		for (uint32 i = 0; i < arraysize; i++)
		{
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
			ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));

			rtvDesc.Format = m_format;
			rtvDesc.ViewDimension = (m_sampling.Count > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			
			rtvDesc.Texture2DArray.ArraySize = 1;
			rtvDesc.Texture2DArray.FirstArraySlice = i;// D3D11CalcSubresource(0, i, texDesc.MipLevels);
			rtvDesc.Texture2DArray.MipSlice = 0;
			
			if (FAILED(m_device->CreateRenderTargetView(m_renderTargetTexture.Get(), &rtvDesc, m_renderTargetViews[i].GetAddressOf())))
				return false;
		}
	}
	else
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));

		// Setup the description of the render target view.
		rtvDesc.Format = texDesc.Format;
		rtvDesc.ViewDimension = (m_sampling.Count > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		// Create the render target view.
		hr = m_device->CreateRenderTargetView(m_renderTargetTexture.Get(), &rtvDesc, m_renderTargetViews[0].GetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

	}

	// Setup the description of the shader resource view.
	srvDesc.Format = texDesc.Format;

	if (m_is_cubemap)
	{
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	}
	else
	{
		srvDesc.ViewDimension = (m_sampling.Count > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
	}

	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = (m_use_mipmaps) ? GetNumMipLevels(m_width, m_height) : 1;
	srvDesc.Texture2D.MipLevels = 1; //Mip mapping doesnt work at the moment for render targets so ignore for now
	m_resourceViews.resize(1);

	// Create the shader resource view.
	hr = m_device->CreateShaderResourceView(m_renderTargetTexture.Get(), &srvDesc, m_resourceViews[0].GetAddressOf());
	if (FAILED(hr))
	{
		return false;
	}

	SetDebugObjectName(m_resourceViews[0].Get(), "D3D11:RenderTarget-ResourceView");


	string debugname = "D3D11:RenderTarget-";
	if (m_is_cubemap)
	{
		debugname += "cubemap-";
	}
	for (int i = 0; i < m_renderTargetViews.size(); i++)
	{
		SetDebugObjectName(m_renderTargetViews[i].Get(), (debugname + to_string(i)).c_str());
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////

DX11DepthBuffer::DX11DepthBuffer(
	ID3D11Device* device,
	uint32 w, uint32 h,
	DXGI_FORMAT format,
	DXGI_SAMPLE_DESC sampling,
	bool cubemap,
	int flags
) : m_device(device)
{
	m_format = format;
	m_width = w;
	m_height = h;
	m_sampling = sampling;
	m_flags = flags;
	m_is_cubemap = cubemap;

	GenerateDepthStencil();
}

bool DX11DepthBuffer::ResizeView(uint32 w, uint32 h)
{
	m_width = w;
	m_height = h;

	return GenerateDepthStencil();
}

bool DX11DepthBuffer::GenerateDepthStencil()
{
	m_depthStencilViews.clear();
	m_resourceViews.clear();

	ShaderViewReset();

	//Actual depth buffer
	ComPtr<ID3D11Texture2D> depthStencilResource;

	uint32 arraysize = (m_is_cubemap) ? 6 : 1;

	m_depthStencilViews.resize(arraysize);
	m_resourceViews.resize(1);

	D3D11_TEXTURE2D_DESC dtd;
	ZeroMemory(&dtd, sizeof(D3D11_TEXTURE2D_DESC));

	dtd.Width = m_width;
	dtd.Height = m_height;
	dtd.MipLevels = 1;
	dtd.ArraySize = arraysize;
	//dtd.Format = DXGI_FORMAT_R24G8_TYPELESS;
	dtd.Format = DXGI_FORMAT_R32_TYPELESS;
	dtd.SampleDesc = m_sampling;
	dtd.Usage = D3D11_USAGE_DEFAULT;
	dtd.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	dtd.CPUAccessFlags = 0;
	dtd.MiscFlags |= (m_is_cubemap) ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;

	//Depth stencil texture
	if (FAILED(m_device->CreateTexture2D(&dtd, NULL, depthStencilResource.GetAddressOf())))
		return false;

	if (arraysize > 1)
	{
		for (uint32 i = 0; i < arraysize; i++)
		{
			//Depth buffer - depth stencil view
			D3D11_DEPTH_STENCIL_VIEW_DESC dvd;
			ZeroMemory(&dvd, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
			dvd.Format = DXGI_FORMAT_D32_FLOAT;
			dvd.ViewDimension = (m_sampling.Count > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY : D3D11_DSV_DIMENSION_TEXTURE2DARRAY;

			dvd.Texture2DArray.ArraySize = 1;
			dvd.Texture2DArray.FirstArraySlice = i;
			dvd.Texture2DArray.MipSlice = 0;

			if (FAILED(m_device->CreateDepthStencilView(depthStencilResource.Get(), &dvd, m_depthStencilViews[i].GetAddressOf())))
				return false;
		}
	}
	else
	{
		//Depth buffer - depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC dvd;
		ZeroMemory(&dvd, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		dvd.Format = DXGI_FORMAT_D32_FLOAT;
		dvd.ViewDimension = (m_sampling.Count > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;

		if (FAILED(m_device->CreateDepthStencilView(depthStencilResource.Get(), &dvd, m_depthStencilViews[0].GetAddressOf())))
			return false;
	}

	//Depth buffer - shader accessible resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC srv;
	ZeroMemory(&srv, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srv.Format = DXGI_FORMAT_R32_FLOAT;

	if (m_is_cubemap)
	{
		srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	}
	else
	{
		srv.ViewDimension = (m_sampling.Count > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
	}

	srv.Texture2D.MostDetailedMip = 0;
	srv.Texture2D.MipLevels = -1;

	if (FAILED(m_device->CreateShaderResourceView(depthStencilResource.Get(), &srv, m_resourceViews[0].GetAddressOf())))
		return false;

	SetDebugObjectName(m_resourceViews[0].Get(), "D3D11:DepthStencilResourceView");


	string debugname = "D3D11:DepthStencilView-";
	if (m_is_cubemap)
	{
		debugname += "cubemap-";
	}

	for (int i = 0; i < m_depthStencilViews.size(); i++)
	{
		SetDebugObjectName(m_depthStencilViews[i].Get(), (debugname + to_string(i)).c_str());
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////////////////////////

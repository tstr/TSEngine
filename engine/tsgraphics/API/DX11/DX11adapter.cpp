/*
	DirectX 11 Rendering API main header
*/

#pragma once

#include "DX11render.h"

#include <tscore/debug/assert.h>

using namespace ts;
using namespace ts::dx11;

/////////////////////////////////////////////////////////////////////////////////////////////////

DX11RenderAdapterFactory::DX11RenderAdapterFactory()
{
	ComPtr<IDXGIAdapter> dxgiAdapter;

	HRESULT hr = S_OK;

	hr = CreateDXGIFactory(IID_OF(IDXGIFactory), (void**)m_dxgiFactory.GetAddressOf());
	tsassert(SUCCEEDED(hr));

	hr = S_OK;
	UINT i = 0;
	while (true)
	{
		ComPtr<IDXGIAdapter> adapter;
		hr = m_dxgiFactory->EnumAdapters(i, adapter.GetAddressOf());

		if (hr == DXGI_ERROR_NOT_FOUND)
		{
			break;
		}
		
		DXGI_ADAPTER_DESC desc;
		SRenderAdapterDesc adapterDesc;
		adapter->GetDesc(&desc);

		_bstr_t str = desc.Description;
		adapterDesc.adapterName = (const char*)str;
		adapterDesc.gpuVideoMemory = desc.DedicatedVideoMemory;
		adapterDesc.gpuSystemMemory = desc.DedicatedSystemMemory;
		adapterDesc.sharedSystemMemory = desc.SharedSystemMemory;

		m_dxgiAdapterDescs.push_back(adapterDesc);

		i++;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////s
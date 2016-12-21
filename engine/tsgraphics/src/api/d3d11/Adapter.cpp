/*
	Render API
	
	D3D11 adapter factory
*/

#pragma once

#include "base.h"
#include <tscore/debug/assert.h>

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////

class D3D11AdapterFactory : public IAdapterFactory
{
private:

	ComPtr<IDXGIFactory> m_dxgiFactory;
	std::vector<SRenderAdapterDesc> m_dxgiAdapterDescs;

public:

	D3D11AdapterFactory()
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
namespace ts
{
	namespace abi
	{
		extern "C"
		{
			int createAdapterFactory(IAdapterFactory** adapterFactory)
			{
				*adapterFactory = new D3D11AdapterFactory();
				return 0;
			}

			void destroyAdapterFactory(IAdapterFactory* adapterFactory)
			{
				if (auto ptr = dynamic_cast<D3D11AdapterFactory*>(adapterFactory))
				{
					delete ptr;
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Render API

	D3D11Render texture resource handling methods
*/

#include "render.h"
#include "helpers.h"
#include <vector>

using namespace ts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ERenderStatus D3D11Render::createResourceTexture(
	HTexture& rsc,
	const STextureResourceData* data,
	const STextureResourceDesc& desc
)
{
	DXGI_FORMAT format = TextureFormatToDXGIFormat(desc.texformat);
	D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	D3D11_CPU_ACCESS_FLAG access = D3D11_CPU_ACCESS_READ;
	
	/*
	//todo: add options for other resource usages
	switch ((EResourceUsage)desc.rscusage)
	{
	case EResourceUsage::UsageDefault: { usage = D3D11_USAGE_DEFAULT; break; }
	case EResourceUsage::UsageDynamic: { usage = D3D11_USAGE_DYNAMIC; break; }
	case EResourceUsage::UsageStaging: { usage = D3D11_USAGE_STAGING; break; }
	case EResourceUsage::UsageStatic: { usage = D3D11_USAGE_IMMUTABLE; break; }
	}
	*/

	//Force default usage
	if (desc.useMips)
	{
		usage = D3D11_USAGE_DEFAULT;
	}

	if (desc.arraySize > 1)
	{
		usage = D3D11_USAGE_DEFAULT;
	}

	if (usage == D3D11_USAGE_DEFAULT)
	{
		access = D3D11_CPU_ACCESS_FLAG(0);
	}
	else if (usage == D3D11_USAGE_DYNAMIC)
	{
		access = D3D11_CPU_ACCESS_WRITE;
	}
	else
	{
		access = D3D11_CPU_ACCESS_READ;
	}

	uint32 miplevels = (desc.useMips) ? getNumMipLevels(desc.width, desc.height) : 1;
	uint32 miscFlags = (desc.textype == ETextureResourceType::eTypeTextureCube) ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
	uint32 arraySize = (desc.textype == ETextureResourceType::eTypeTextureCube) ? 6 : 1; //Do not allow texture arrays for now

	vector<D3D11_SUBRESOURCE_DATA> subresources(arraySize);

	if (data)
	{
		for (uint32 i = 0; i < arraySize; i++)
		{
			subresources[i].pSysMem = data[i].memory;
			subresources[i].SysMemPitch = data[i].memoryByteWidth;
			subresources[i].SysMemSlicePitch = data[i].memoryByteDepth;
		}
	}

	D3D11_SUBRESOURCE_DATA* pSubresource = (desc.useMips || !data) ? nullptr : &subresources[0];

	if (desc.useMips)
	{
		uint32 fmtSupport;
		m_device->CheckFormatSupport(format, &fmtSupport);
		tsassert(fmtSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN);
	}

	//Create a temporary texture resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.Format = format;
	sd.Texture1D.MipLevels = miplevels;

	HRESULT hr = S_OK;

	ComPtr<ID3D11Resource> resource;

	UINT bindFlags = 0;

	if (desc.texmask & eTextureMaskShaderResource)
	{
		bindFlags |= D3D11_BIND_SHADER_RESOURCE;
	}
	if (desc.texmask & eTextureMaskRenderTarget)
	{
		bindFlags |= D3D11_BIND_RENDER_TARGET;
	}
	if (desc.texmask & eTextureMaskDepthTarget)
	{
		if (bindFlags & D3D11_BIND_RENDER_TARGET)
		{
			tswarn("A texture resource with the D3D11_BIND_RENDER_TARGET flag cannot have the D3D11_BIND_DEPTH_STENCIL flag set");
			return eInvalidParameter;
		}

		bindFlags |= D3D11_BIND_DEPTH_STENCIL;
	}

	//Create texture resource
	switch (desc.textype)
	{
	case (ETextureResourceType::eTypeTexture1D):
	{
		D3D11_TEXTURE1D_DESC dtd;
		ZeroMemory(&dtd, sizeof(dtd));

		dtd.BindFlags = bindFlags;
		dtd.Width = desc.width;
		dtd.ArraySize = arraySize;
		dtd.MipLevels = miplevels;
		dtd.Format = format;
		dtd.MiscFlags = miscFlags;
		dtd.Usage = usage;
		dtd.CPUAccessFlags = access;

		if (desc.useMips)
		{
			dtd.BindFlags |= D3D11_BIND_RENDER_TARGET;
			dtd.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}

		hr = m_device->CreateTexture1D(&dtd, pSubresource, (ID3D11Texture1D**)resource.GetAddressOf());
		sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;

		break;
	}
	case (ETextureResourceType::eTypeTextureCube):
	case (ETextureResourceType::eTypeTexture2D):
	{
		D3D11_TEXTURE2D_DESC dtd;
		ZeroMemory(&dtd, sizeof(dtd));

		dtd.BindFlags = bindFlags;
		dtd.Width = desc.width;
		dtd.Height = desc.height;
		dtd.Format = format;
		dtd.MiscFlags = miscFlags;
		dtd.ArraySize = arraySize;
		dtd.Usage = usage;
		dtd.CPUAccessFlags = access;
		dtd.MipLevels = miplevels;
		dtd.SampleDesc.Count = desc.multisampling.count;
		getMultisampleQuality(dtd.SampleDesc);

		if (desc.useMips)
		{
			dtd.BindFlags |= D3D11_BIND_RENDER_TARGET;
			dtd.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}

		hr = m_device->CreateTexture2D(&dtd, pSubresource, (ID3D11Texture2D**)resource.GetAddressOf());
		sd.ViewDimension = (desc.textype == eTypeTextureCube) ? D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2D;

		break;
	}
	case (ETextureResourceType::eTypeTexture3D):
	{
		D3D11_TEXTURE3D_DESC dtd;
		ZeroMemory(&dtd, sizeof(dtd));

		dtd.BindFlags = bindFlags;
		dtd.Width = desc.width;
		dtd.Height = desc.height;
		dtd.Depth = desc.depth;
		dtd.Format = format;
		dtd.MipLevels = miplevels;
		dtd.MiscFlags = miscFlags;
		dtd.Usage = usage;
		dtd.CPUAccessFlags = access;

		if (desc.useMips)
		{
			dtd.BindFlags |= D3D11_BIND_RENDER_TARGET;
			dtd.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}

		hr = m_device->CreateTexture3D(&dtd, pSubresource, (ID3D11Texture3D**)resource.GetAddressOf());
		sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;

		break;
	}
	}

	if (FAILED(hr))
	{
		return RenderStatusFromHRESULT(hr);
	}

	if (desc.useMips)
	{
		//lock_guard<mutex>lk(m_rtv_lock);

		//Create a temporary texture resource view
		ComPtr<ID3D11ShaderResourceView> tempView;

		hr = m_device->CreateShaderResourceView(resource.Get(), &sd, tempView.GetAddressOf());

		for (int i = 0; (size_t)i < subresources.size(); i++)
		{
			m_immediateContext->UpdateSubresource(resource.Get(), 0, nullptr, subresources[i].pSysMem, subresources[i].SysMemPitch, subresources[i].SysMemSlicePitch);
		}

		m_immediateContext->GenerateMips(tempView.Get());
	}

	if (FAILED(hr))
	{
		return RenderStatusFromHRESULT(hr);
	}
	
	auto r = resource.Detach();
	rsc = (HTexture)reinterpret_cast<HTexture&>(r);

	return eOk;
}

void D3D11Render::destroyTexture(HTexture texture)
{
	if (auto u = reinterpret_cast<IUnknown*>(texture))
	{
		u->Release();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

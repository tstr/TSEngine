/*
	Render API

	D3D11Render texture resource handling methods
*/

#include "render.h"
#include "helpers.h"
#include "HandleTexture.h"
#include <vector>

using namespace ts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ERenderStatus D3D11Render::createResourceTexture(
	HTexture& texRsc,
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
	
	vector<D3D11_SUBRESOURCE_DATA> subresources(desc.arraySize);

	if (data)
	{
		for (uint32 i = 0; i < desc.arraySize; i++)
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
			dtd.ArraySize = desc.arraySize;
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
			dtd.ArraySize = desc.arraySize;
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
	
	texRsc = D3D11Texture::downcast(new D3D11Texture(resource.Get()));

	return eOk;
}

void D3D11Render::destroyTexture(HTexture texture)
{
	if (D3D11Texture* t = D3D11Texture::upcast(texture))
	{
		delete t;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ComPtr<ID3D11ShaderResourceView> D3D11Texture::getView(uint32 arrayIndex, uint32 arrayCount, ETextureResourceType type)
{
	STextureView view;
	view.arrayIndex = arrayIndex;
	view.arrayCount = arrayCount;
	view.type = type;

	ComPtr<ID3D11Device> device;
	m_tex->GetDevice(device.GetAddressOf());

	auto it = find(m_texViewCache.begin(), m_texViewCache.end(), view);

	if (it != m_texViewCache.end())
	{
		return it->view;
	}
	else
	{
		ID3D11Resource* rsc = m_tex.Get();
		ComPtr<ID3D11ShaderResourceView> srv;

		switch (type)
		{
			case eTypeTexture1D:
			{
				D3D11_TEXTURE1D_DESC desc;
				if (auto tex = static_cast<ID3D11Texture1D*>(rsc))
				{
					tex->GetDesc(&desc);

					D3D11_SHADER_RESOURCE_VIEW_DESC viewdesc;
					ZeroMemory(&viewdesc, sizeof(viewdesc));
					viewdesc.Format = desc.Format;
					viewdesc.ViewDimension = (desc.ArraySize > 1) ? D3D11_SRV_DIMENSION_TEXTURE1DARRAY : D3D11_SRV_DIMENSION_TEXTURE1D;
					viewdesc.Texture1DArray.MipLevels = desc.MipLevels;
					viewdesc.Texture1DArray.ArraySize = arrayCount;
					viewdesc.Texture1DArray.FirstArraySlice = arrayIndex;
					viewdesc.Texture1DArray.MostDetailedMip = 0;

					device->CreateShaderResourceView(tex, &viewdesc, srv.GetAddressOf());
				}

				break;
			}

			case eTypeTexture2D:
			{
				D3D11_TEXTURE2D_DESC desc;
				if (auto tex = static_cast<ID3D11Texture2D*>(rsc))
				{
					tex->GetDesc(&desc);

					D3D11_SHADER_RESOURCE_VIEW_DESC viewdesc;
					ZeroMemory(&viewdesc, sizeof(viewdesc));
					viewdesc.Format = desc.Format;
					viewdesc.ViewDimension = (desc.ArraySize > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DARRAY : D3D11_SRV_DIMENSION_TEXTURE2D;
					viewdesc.Texture2DArray.MipLevels = desc.MipLevels;
					viewdesc.Texture2DArray.ArraySize = arrayCount;
					viewdesc.Texture2DArray.FirstArraySlice = arrayIndex;
					viewdesc.Texture2DArray.MostDetailedMip = 0;

					device->CreateShaderResourceView(tex, &viewdesc, srv.GetAddressOf());
				}

				break;
			}

			case eTypeTexture3D:
			{
				D3D11_TEXTURE3D_DESC desc;
				if (auto tex = static_cast<ID3D11Texture3D*>(rsc))
				{
					tex->GetDesc(&desc);

					D3D11_SHADER_RESOURCE_VIEW_DESC viewdesc;
					ZeroMemory(&viewdesc, sizeof(viewdesc));
					viewdesc.Format = desc.Format;
					viewdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
					viewdesc.Texture3D.MipLevels = desc.MipLevels;
					viewdesc.Texture3D.MostDetailedMip = 0;

					device->CreateShaderResourceView(tex, &viewdesc, srv.GetAddressOf());
				}

				break;
			}

			case eTypeTextureCube:
			{
				D3D11_TEXTURE2D_DESC desc;
				if (auto tex = static_cast<ID3D11Texture2D*>(rsc))
				{
					tex->GetDesc(&desc);

					D3D11_SHADER_RESOURCE_VIEW_DESC viewdesc;
					ZeroMemory(&viewdesc, sizeof(viewdesc));
					viewdesc.Format = desc.Format;
					viewdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
					viewdesc.Texture2DArray.MipLevels = desc.MipLevels;
					viewdesc.Texture2DArray.ArraySize = arrayCount;
					viewdesc.Texture2DArray.FirstArraySlice = arrayIndex;
					viewdesc.Texture2DArray.MostDetailedMip = 0;

					device->CreateShaderResourceView(tex, &viewdesc, srv.GetAddressOf());
				}

				break;
			}
		}

		view.view = srv;
		m_texViewCache.push_back(view);
		return srv;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

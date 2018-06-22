/*
	Render API

	Resource object

	Can be an image or buffer
*/

#include "render.h"
#include "helpers.h"
#include "HandleResource.h"
#include <vector>

using namespace ts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Image creation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ResourceHandle D3D11::createResourceImage(const ResourceData* data, const ImageResourceInfo& info, ResourceHandle recycle)
{
	DXGI_FORMAT format = ImageFormatToDXGIFormat(info.format);
	D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	D3D11_CPU_ACCESS_FLAG access = D3D11_CPU_ACCESS_READ;

	//Force default usage
	if (info.useMips)
	{
		usage = D3D11_USAGE_DEFAULT;
	}

	if (info.length > 1)
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

	uint32 miplevels = (info.useMips) ? getNumMipLevels(info.width, info.height) : 1;
	uint32 miscFlags = (info.type == ImageType::CUBE) ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
	
	vector<D3D11_SUBRESOURCE_DATA> subresources(info.length);

	if (data)
	{
		for (uint32 i = 0; i < info.length; i++)
		{
			subresources[i].pSysMem = data[i].memory;
			subresources[i].SysMemPitch = data[i].memoryByteWidth;
			subresources[i].SysMemSlicePitch = data[i].memoryByteDepth;
		}
	}

	D3D11_SUBRESOURCE_DATA* pSubresource = (info.useMips || !data) ? nullptr : &subresources[0];

	if (info.useMips)
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

	if (info.mask & (uint8)ImageTypeMask::SRV)
	{
		bindFlags |= D3D11_BIND_SHADER_RESOURCE;
	}
	if (info.mask & (uint8)ImageTypeMask::RTV)
	{
		bindFlags |= D3D11_BIND_RENDER_TARGET;
	}
	if (info.mask & (uint8)ImageTypeMask::DSV)
	{
		if (bindFlags & D3D11_BIND_RENDER_TARGET)
		{
			tswarn("A texture resource with the D3D11_BIND_RENDER_TARGET flag cannot have the D3D11_BIND_DEPTH_STENCIL flag set");
			return D3D11Resource::downcast(0);
		}

		bindFlags |= D3D11_BIND_DEPTH_STENCIL;
	}

	//Create texture resource
	switch (info.type)
	{
		case (ImageType::_1D):
		{
			D3D11_TEXTURE1D_DESC dtd;
			ZeroMemory(&dtd, sizeof(dtd));

			dtd.BindFlags = bindFlags;
			dtd.Width = info.width;
			dtd.ArraySize = info.length;
			dtd.MipLevels = miplevels;
			dtd.Format = format;
			dtd.MiscFlags = miscFlags;
			dtd.Usage = usage;
			dtd.CPUAccessFlags = access;

			if (info.useMips)
			{
				dtd.BindFlags |= D3D11_BIND_RENDER_TARGET;
				dtd.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
			}

			hr = m_device->CreateTexture1D(&dtd, pSubresource, (ID3D11Texture1D**)resource.GetAddressOf());
			sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;

			break;
		}
		case (ImageType::CUBE):
		case (ImageType::_2D):
		{
			D3D11_TEXTURE2D_DESC dtd;
			ZeroMemory(&dtd, sizeof(dtd));

			dtd.BindFlags = bindFlags;
			dtd.Width = info.width;
			dtd.Height = info.height;
			dtd.Format = format;
			dtd.MiscFlags = miscFlags;
			dtd.ArraySize = info.length;
			dtd.Usage = usage;
			dtd.CPUAccessFlags = access;
			dtd.MipLevels = miplevels;
			dtd.SampleDesc.Count = info.msCount;
			getMultisampleQuality(dtd.SampleDesc);

			if (info.useMips)
			{
				dtd.BindFlags |= D3D11_BIND_RENDER_TARGET;
				dtd.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
			}

			hr = m_device->CreateTexture2D(&dtd, pSubresource, (ID3D11Texture2D**)resource.GetAddressOf());
			sd.ViewDimension = (info.type == ImageType::CUBE) ? D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2D;

			break;
		}
		case (ImageType::_3D):
		{
			D3D11_TEXTURE3D_DESC dtd;
			ZeroMemory(&dtd, sizeof(dtd));

			dtd.BindFlags = bindFlags;
			dtd.Width = info.width;
			dtd.Height = info.height;
			dtd.Depth = info.length;
			dtd.Format = format;
			dtd.MipLevels = miplevels;
			dtd.MiscFlags = miscFlags;
			dtd.Usage = usage;
			dtd.CPUAccessFlags = access;

			if (info.useMips)
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
		return D3D11Resource::downcast(0);
	}

	if (info.useMips)
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
		return D3D11Resource::downcast(nullptr);
	}

	if (recycle != (ResourceHandle)0)
	{
		auto rsc = D3D11Resource::upcast(recycle);
		rsc->reset();
		return D3D11Resource::downcast(new(rsc) D3D11Resource(resource.Get(), true));
	}
	else
	{
		return D3D11Resource::downcast(new D3D11Resource(resource.Get(), true));
	}
}

void D3D11::destroy(ResourceHandle hrsc)
{
	auto rsc = D3D11Resource::upcast(hrsc);

	if (rsc)
	{
		delete rsc;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ID3D11ShaderResourceView* D3D11Resource::getSRV(uint32 arrayIndex, uint32 arrayCount, ImageType type)
{
	if (!isImage()) return nullptr;

	SRV view;
	view.arrayIndex = arrayIndex;
	view.arrayCount = arrayCount;
	view.type = type;

	ComPtr<ID3D11Device> device;
	m_rsc->GetDevice(device.GetAddressOf());

	auto it = m_srvCache.find(view);

	if (it != m_srvCache.end())
	{
		return it->second.Get();
	}
	else
	{
		ID3D11Resource* rsc = m_rsc.Get();
		ComPtr<ID3D11ShaderResourceView> srv;

		switch (type)
		{
			case ImageType::_1D:
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

			case ImageType::_2D:
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

			case ImageType::_3D:
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

			case ImageType::CUBE:
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

		m_srvCache.insert(make_pair(view, srv));
		return srv.Get();
	}
}

ID3D11RenderTargetView* D3D11Resource::getRTV(uint32 arrayIndex)
{
	if (!isImage()) return nullptr;

	ComPtr<ID3D11Device> device;
	m_rsc->GetDevice(device.GetAddressOf());

	auto it = m_rtvCache.find(arrayIndex);

	if (it != m_rtvCache.end())
	{
		return it->second.Get();
	}
	else
	{
		//kind of dangerous
		if (auto tex = static_cast<ID3D11Texture2D*>(m_rsc.Get()))
		{
			D3D11_TEXTURE2D_DESC desc;
			tex->GetDesc(&desc);

			if (desc.BindFlags & D3D11_BIND_RENDER_TARGET)
			{
				bool multisampled = desc.SampleDesc.Count > 1;

				D3D11_RENDER_TARGET_VIEW_DESC viewdesc;
				ZeroMemory(&viewdesc, sizeof(viewdesc));

				if (desc.ArraySize > 1)
					viewdesc.ViewDimension = (multisampled) ? D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				else
					viewdesc.ViewDimension = (multisampled) ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;

				viewdesc.Format = desc.Format;
				viewdesc.Texture2DArray.FirstArraySlice = arrayIndex;
				viewdesc.Texture2DArray.ArraySize = 1;
				viewdesc.Texture2DArray.MipSlice = 0;

				ComPtr<ID3D11RenderTargetView> rtv;
				if (FAILED(device->CreateRenderTargetView(m_rsc.Get(), &viewdesc, rtv.GetAddressOf())))
				{
					return nullptr;
				}

				m_rtvCache.insert(make_pair(arrayIndex, rtv));
				return rtv.Get();
			}
		}

		return nullptr;
	}
}

ID3D11DepthStencilView* D3D11Resource::getDSV(uint32 arrayIndex)
{
	if (!isImage()) return nullptr;

	ComPtr<ID3D11Device> device;
	m_rsc->GetDevice(device.GetAddressOf());

	auto it = m_dsvCache.find(arrayIndex);

	if (it != m_dsvCache.end())
	{
		return it->second.Get();
	}
	else
	{
		if (ID3D11Texture2D* tex = static_cast<ID3D11Texture2D*>(m_rsc.Get()))
		{
			D3D11_TEXTURE2D_DESC desc;
			tex->GetDesc(&desc);

			if (desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
			{
				bool multisampled = desc.SampleDesc.Count > 1;

				D3D11_DEPTH_STENCIL_VIEW_DESC viewdesc;
				ZeroMemory(&viewdesc, sizeof(viewdesc));

				if (desc.ArraySize > 1)
					viewdesc.ViewDimension = (multisampled) ? D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY : D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				else
					viewdesc.ViewDimension = (multisampled) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;

				viewdesc.Format = desc.Format;
				viewdesc.Texture2DArray.FirstArraySlice = arrayIndex;
				viewdesc.Texture2DArray.ArraySize = 1;
				viewdesc.Texture2DArray.MipSlice = 0;

				ComPtr<ID3D11DepthStencilView> dsv;
				if (FAILED(device->CreateDepthStencilView(tex, &viewdesc, dsv.GetAddressOf())))
				{
					return nullptr;
				}

				m_dsvCache.insert(make_pair(arrayIndex, dsv));
				return dsv.Get();
			}
		}

		return nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer creation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ResourceHandle D3D11::createResourceBuffer(const ResourceData& data, const BufferResourceInfo& info, ResourceHandle recycle)
{
	ID3D11Buffer* buffer = nullptr;

	D3D11_SUBRESOURCE_DATA subdata;
	D3D11_BUFFER_DESC subdesc;

	ZeroMemory(&subdata, sizeof(D3D11_SUBRESOURCE_DATA));
	ZeroMemory(&subdesc, sizeof(D3D11_BUFFER_DESC));

	//For now keep resource usage as default
	subdesc.Usage = D3D11_USAGE_DEFAULT;

	switch (info.type)
	{
	case BufferType::VERTEX: { subdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; break; }
	case BufferType::INDEX : { subdesc.BindFlags = D3D11_BIND_INDEX_BUFFER; break; }
	case BufferType::CONSTANTS: { subdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; break; }
	}

	//Only dynamic resources are allowed direct access to buffer memory
	if (subdesc.Usage == D3D11_USAGE_DYNAMIC)
	{
		subdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	subdesc.MiscFlags = 0;
	subdesc.ByteWidth = info.size;

	subdata.pSysMem = data.memory;
	subdata.SysMemPitch = data.memoryByteWidth;
	subdata.SysMemSlicePitch = data.memoryByteDepth;

	HRESULT hr = m_device->CreateBuffer(&subdesc, &subdata, &buffer);

	if (FAILED(hr))
	{
		return D3D11Resource::downcast(nullptr);
	}
	else
	{
		if (recycle != (ResourceHandle)0)
		{
			auto rsc = D3D11Resource::upcast(recycle);
			rsc->reset();
			return D3D11Resource::downcast(new(rsc) D3D11Resource(buffer, false));
		}
		else
		{
			return D3D11Resource::downcast(new D3D11Resource(buffer, false));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Render API

	D3D11Render target handling methods
*/

#include "Render.h"
#include "HandleTarget.h"
#include "HandleTexture.h"

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ERenderStatus D3D11Render::createTarget(
	HTarget& target,
	const STargetDesc& targetDesc
)
{
	ComPtr<ID3D11RenderTargetView> renderTargets[STargetDesc::eMaxRenderTextures];
	ComPtr<ID3D11DepthStencilView> depthStencil;

	//Create RenderTargetViews for each HTexture
	for (uint32 i = 0; i < STargetDesc::eMaxRenderTextures; i++)
	{
		if (D3D11Texture* h = D3D11Texture::upcast(targetDesc.renderTextures[i]))
		{
			//Obtain texture2D interface from HTexture
			if (ID3D11Texture2D* tex = static_cast<ID3D11Texture2D*>(h->getResource().Get()))
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
					viewdesc.Texture2DArray.FirstArraySlice = targetDesc.renderTextures[i];
					viewdesc.Texture2DArray.ArraySize = 1;
					viewdesc.Texture2DArray.MipSlice = 0;

					ComPtr<ID3D11RenderTargetView> rtv;
					if (FAILED(m_device->CreateRenderTargetView(tex, &viewdesc, rtv.GetAddressOf())))
					{
						return eFail;
					}

					renderTargets[i] = rtv;
				}
				else
				{
					return eInvalidResource;
				}
			}
		}
	}

	if (D3D11Texture* h = D3D11Texture::upcast(targetDesc.depthTexture))
	{
		//Obtain texture2D interface from depth HTexture
		if (ID3D11Texture2D* tex = static_cast<ID3D11Texture2D*>(h->getResource().Get()))
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
				viewdesc.Texture2DArray.FirstArraySlice = targetDesc.depthTextureIndex;
				viewdesc.Texture2DArray.ArraySize = 1;
				viewdesc.Texture2DArray.MipSlice = 0;

				if (FAILED(m_device->CreateDepthStencilView(tex, &viewdesc, depthStencil.GetAddressOf())))
				{
					return eFail;
				}
			}
			else
			{
				return eInvalidResource;
			}
		}
	}

	D3D11Target* t = new D3D11Target(renderTargets[0].GetAddressOf(), STargetDesc::eMaxRenderTextures, depthStencil.Get());
	target = D3D11Target::downcast(t);
	
	return eOk;
}

void D3D11Render::destroyTarget(HTarget target)
{
	if (auto t = D3D11Target::upcast(target))
	{
		delete t;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

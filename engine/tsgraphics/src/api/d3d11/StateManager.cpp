/*
	Render API
	
	D3D11StateManager class implementation
*/

#include "statemanager.h"
#include "helpers.h"

#include <tuple>

using namespace ts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	State entry comparison methods
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<>
bool D3D11StateManager::StateEntry<SDepthState, ID3D11DepthStencilState>::operator==(const StateEntry& rhs)
{
	return tie(desc.enableDepth, desc.enableStencil) == tie(rhs.desc.enableDepth, rhs.desc.enableStencil);
}

template<>
bool D3D11StateManager::StateEntry<SRasterState, ID3D11RasterizerState>::operator==(const StateEntry& rhs)
{
	return tie(
		desc.enableScissor,
		desc.cullMode,
		desc.fillMode
	) == tie(
		rhs.desc.enableScissor,
		rhs.desc.cullMode,
		rhs.desc.fillMode
	);
}

template<>
bool D3D11StateManager::StateEntry<SBlendState, ID3D11BlendState>::operator==(const StateEntry& rhs)
{
	return (desc.enable == rhs.desc.enable);
}

template<>
bool D3D11StateManager::StateEntry<STextureSampler, ID3D11SamplerState>::operator==(const StateEntry& rhs)
{
	return tie(
		desc.enabled,
		desc.addressU,
		desc.addressV,
		desc.addressW,
		desc.borderColour,
		desc.filtering
	) == tie(
		rhs.desc.enabled,
		rhs.desc.addressU,
		rhs.desc.addressV,
		rhs.desc.addressW,
		rhs.desc.borderColour,
		rhs.desc.filtering
	);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT D3D11StateManager::demandDepthState(const SDepthState & desc, ID3D11DepthStencilState** state)
{
	if (m_cacheDepthState.find(desc, state))
	{
		return S_OK;
	}
	else
	{
		D3D11_DEPTH_STENCIL_DESC depthDesc;
		ZeroMemory(&depthDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

		depthDesc.DepthEnable = desc.enableDepth;
		depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthDesc.StencilEnable = desc.enableStencil;
		depthDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		depthDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
		depthDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthDesc.FrontFace.StencilFailOp = depthDesc.FrontFace.StencilDepthFailOp = depthDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthDesc.BackFace = depthDesc.FrontFace;
		
		HRESULT hr = m_device->CreateDepthStencilState(&depthDesc, state);
		if (SUCCEEDED(hr))
		{
			m_cacheDepthState.insert(desc, *state);
		}

		return hr;
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT D3D11StateManager::demandRasterizerState(const SRasterState & desc, ID3D11RasterizerState** state)
{
	if (m_cacheRasterState.find(desc, state))
	{
		return S_OK;
	}
	else
	{
		D3D11_RASTERIZER_DESC rstrDesc;
		ZeroMemory(&rstrDesc, sizeof(rstrDesc));

		rstrDesc.FillMode = D3D11_FILL_SOLID;

		switch (desc.fillMode)
		{
		case eFillSolid:
			rstrDesc.FillMode = D3D11_FILL_SOLID;
			break;
		case eFillWireframe:
			rstrDesc.FillMode = D3D11_FILL_WIREFRAME;
			break;
		}

		switch (desc.cullMode)
		{
		case eCullNone:
			rstrDesc.CullMode = D3D11_CULL_NONE;
			break;
		case eCullBack:
			rstrDesc.CullMode = D3D11_CULL_BACK;
			break;
		case eCullFront:
			rstrDesc.CullMode = D3D11_CULL_FRONT;
			break;
		}

		rstrDesc.MultisampleEnable = true;
		rstrDesc.ScissorEnable = desc.enableScissor;
		rstrDesc.DepthClipEnable = true;
		
		HRESULT hr = m_device->CreateRasterizerState(&rstrDesc, state);
		if (SUCCEEDED(hr))
		{
			m_cacheRasterState.insert(desc, *state);
		}

		return hr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT D3D11StateManager::demandBlendState(const SBlendState & desc, ID3D11BlendState** state)
{
	if (m_cacheBlendState.find(desc, state))
	{
		return S_OK;
	}
	else
	{
		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(blendDesc));

		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0].BlendEnable = desc.enable;
		//dest.rgb = src.rgb * src.a + dest.rgb * (1 - src.a)
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		//dest.a = 1 - (1 - src.a) * (1 - dest.a)
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		
		HRESULT hr = m_device->CreateBlendState(&blendDesc, state);
		if (SUCCEEDED(hr))
		{
			m_cacheBlendState.insert(desc, *state);
		}

		return hr;
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline D3D11_TEXTURE_ADDRESS_MODE getAddressMode(ETextureAddressMode mode)
{
	switch (mode)
	{
		case (eTextureAddressClamp) : return D3D11_TEXTURE_ADDRESS_CLAMP;
		case (eTextureAddressMirror): return D3D11_TEXTURE_ADDRESS_MIRROR;
		case (eTextureAddressWrap)  : return D3D11_TEXTURE_ADDRESS_WRAP;
		case (eTextureAddressBorder): return D3D11_TEXTURE_ADDRESS_BORDER;
	}

	return D3D11_TEXTURE_ADDRESS_MODE(0);
}

HRESULT D3D11StateManager::demandSamplerState(const STextureSampler & desc, ID3D11SamplerState** state)
{
	if (m_cacheSamplerState.find(desc, state))
	{
		return S_OK;
	}
	else
	{
		D3D11_SAMPLER_DESC sampleDesc;
		ZeroMemory(&sampleDesc, sizeof(D3D11_SAMPLER_DESC));

		sampleDesc.AddressU = getAddressMode(desc.addressU);
		sampleDesc.AddressV = getAddressMode(desc.addressV);
		sampleDesc.AddressW = getAddressMode(desc.addressW);
		sampleDesc.BorderColor[0] = (float)desc.borderColour.R() / 255.0f;
		sampleDesc.BorderColor[1] = (float)desc.borderColour.G() / 255.0f;
		sampleDesc.BorderColor[2] = (float)desc.borderColour.B() / 255.0f;
		sampleDesc.BorderColor[3] = (float)desc.borderColour.A() / 255.0f;

		switch (desc.filtering)
		{
		case eTextureFilterPoint:
			sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			break;
		case eTextureFilterBilinear:
			sampleDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			break;
		case eTextureFilterTrilinear:
			sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			break;
		case eTextureFilterAnisotropic2x:
			sampleDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			sampleDesc.MaxAnisotropy = 2;
			break;
		case eTextureFilterAnisotropic4x:
			sampleDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			sampleDesc.MaxAnisotropy = 4;
			break;
		case eTextureFilterAnisotropic8x:
			sampleDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			sampleDesc.MaxAnisotropy = 8;
			break;
		case eTextureFilterAnisotropic16x:
			sampleDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			sampleDesc.MaxAnisotropy = 16;
			break;
		}

		sampleDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampleDesc.MinLOD = -FLT_MAX;
		sampleDesc.MaxLOD = FLT_MAX;
		sampleDesc.MipLODBias = 0.0f;

		HRESULT hr = m_device->CreateSamplerState(&sampleDesc, state);
		if (SUCCEEDED(hr))
		{
			m_cacheSamplerState.insert(desc, *state);
		}

		return hr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

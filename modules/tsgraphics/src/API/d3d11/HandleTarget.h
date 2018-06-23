/*
	Render API

	D3D11Target class
*/

#pragma once

#include "render.h"
#include "helpers.h"
#include "handle.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	class D3D11Target : public Handle<D3D11Target, TargetHandle>
	{
	private:

		ComPtr<ID3D11RenderTargetView> m_renderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
		uint32 m_renderTargetsCount = 0;
		ComPtr<ID3D11DepthStencilView> m_depthStencil;

	public:

		D3D11Target() {}
		
		D3D11Target(ID3D11RenderTargetView** rtvs, uint32 rtvCount, ID3D11DepthStencilView* dsv)
		{
			for (uint32 i = 0; i < rtvCount; i++)
				m_renderTargets[i] = ComPtr<ID3D11RenderTargetView>(rtvs[i]);

			m_renderTargetsCount = rtvCount;

			m_depthStencil = ComPtr<ID3D11DepthStencilView>(dsv);
		}

		~D3D11Target() { reset(); }

		void clearRenderTargets(ID3D11DeviceContext* context, const Vector& vec)
		{
			for (uint32 i = 0; i < m_renderTargetsCount; i++)
				context->ClearRenderTargetView(m_renderTargets[i].Get(), (const float*)&vec);
		}

		void clearDepthStencil(ID3D11DeviceContext* context, float depth)
		{
			context->ClearDepthStencilView(m_depthStencil.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, 0);
		}
		
		//Binds all RTV's and DSV's in this target to a device context
		void bind(ID3D11DeviceContext* context)
		{
			context->OMSetRenderTargets(m_renderTargetsCount, (ID3D11RenderTargetView**)m_renderTargets, m_depthStencil.Get());
		}

		//Releases all RTV's and DSV's in this target
		void reset()
		{
			for (uint32 i = 0; i < m_renderTargetsCount; i++)
				m_renderTargets[i].Reset();

			m_depthStencil.Reset();

			m_renderTargetsCount = 0;
		}
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

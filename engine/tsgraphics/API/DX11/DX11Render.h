/*
	Render API

	DirectX 11 implementation of Render API
*/

#pragma once

#include <vector>
#include <tsgraphics/renderapi.h>
#include "dx11base.h"

namespace ts
{
	namespace dx11
	{
		class DX11RenderApi : public ts::IRenderApi
		{
		private:

			HWND m_hwnd;
			SRenderApiConfiguration m_config;

			ComPtr<IDXGISwapChain> m_dxgiSwapchain;
			ComPtr<ID3D11Device> m_device;
			ComPtr<ID3D11DeviceContext> m_immediateContext;

			ComPtr<ID3D11RenderTargetView> m_swapChainRenderTarget;

		public:

			DX11RenderApi(const SRenderApiConfiguration& cfg);
			~DX11RenderApi();

			void setWindowMode(EWindowMode mode) override;
			
			void drawBegin(const Vector& vec) override;
			void drawEnd() override;
		};


		class DX11RenderAdapterFactory : public ts::IRenderAdapterFactory
		{
		private:

			ComPtr<IDXGIFactory> m_dxgiFactory;
			std::vector<SRenderAdapterDesc> m_dxgiAdapterDescs;

		public:

			DX11RenderAdapterFactory();

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
	}
}
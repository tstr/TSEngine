/*
	Render API

	DirectX 11 implementation of Render API
*/

#pragma once

#include <tsgraphics/renderapi.h>
#include "dx11base.h"

namespace ts
{
	namespace dx11
	{
		class DX11RenderApi : public ts::IRenderApi
		{
		private:

			ComPtr<IDXGISwapChain> m_dxgiSwapchain;
			ComPtr<ID3D11Device> m_device;
			ComPtr<ID3D11DeviceContext> m_immediateContext;

			ComPtr<ID3D11RenderTargetView> m_swapChainRenderTarget;

		public:

			DX11RenderApi(const SRenderApiConfiguration& cfg);
			~DX11RenderApi();

			void drawBegin(const Vector& vec) override;
			void drawEnd() override;
		};
	}
}
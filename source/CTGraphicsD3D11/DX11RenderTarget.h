/*
	DirectX-11 Render target api
*/

#pragma once

#include "DX11Base.h"
#include "DX11Utils.h"

using namespace std;

namespace CT
{
	namespace DX11
	{
		enum EDX11ViewFlags
		{
			MIP_AUTOGEN = 1
		};

		class DX11View
		{
		protected:

			//Shader resource view
			vector<ComPtr<ID3D11ShaderResourceView>> m_resourceViews;

			void ShaderViewReset()
			{
				if (!m_resourceViews.empty())
				{
					m_resourceViews.clear();
				}
			}

		public:

			ID3D11ShaderResourceView* GetTextureView(uint32 index)
			{
				return m_resourceViews[index].Get();
			}
		};

		class DX11RenderTarget : public DX11View
		{
		private:

			ComPtr<IDXGISwapChain> m_dxgiSwapchain;

			uint32 m_width = 0;
			uint32 m_height = 0;
			DXGI_FORMAT m_format;
			DXGI_SAMPLE_DESC m_sampling;
			bool m_is_cubemap = false;
			bool m_use_mipmaps = false;

			ComPtr<ID3D11Device> m_device;
			vector<ComPtr<ID3D11RenderTargetView>> m_renderTargetViews; //Render target view array

			ComPtr<ID3D11Texture2D> m_renderTargetTexture;
			//ComPtr<ID3D11Texture2D> m_renderTargetTextureResolved;

			bool GenerateRenderTarget();

			void RenderTargetViewReset()
			{
				if (!m_renderTargetViews.empty())
				{
					m_renderTargetViews.clear();
				}
			}

		public:

			DX11RenderTarget() {}

			DX11RenderTarget(
				ID3D11Device* device,
				uint32 w, uint32 h,
				DXGI_FORMAT format,
				DXGI_SAMPLE_DESC sampling,
				bool cubemap,
				bool mipmaps
			);

			DX11RenderTarget(IDXGISwapChain* swapchain);
			
			~DX11RenderTarget();

			bool ResizeView(uint32 w, uint32 h);

			ID3D11RenderTargetView* GetRenderTargetView(uint32 index) const { return m_renderTargetViews[index].Get(); }

			void GetViewportStruct(D3D11_VIEWPORT& viewport) const
			{
				viewport.Height = (float)m_height;
				viewport.Width = (float)m_width;
				viewport.MinDepth = 0;
				viewport.MaxDepth = 1;
				viewport.TopLeftX = 0;
				viewport.TopLeftY = 0;
			}

			bool isCubeMap() const { return m_is_cubemap; }
		};

		class DX11DepthBuffer : public DX11View
		{
		private:

			uint32 m_width = 0;
			uint32 m_height = 0;
			DXGI_FORMAT m_format;
			DXGI_SAMPLE_DESC m_sampling;
			bool m_is_cubemap = false;

			int m_flags = 0;

			ComPtr<ID3D11Device> m_device;
			vector<ComPtr<ID3D11DepthStencilView>> m_depthStencilViews; //Depth stencil view array

			bool GenerateDepthStencil();

		public:

			DX11DepthBuffer() {}

			bool ResizeView(uint32 w, uint32 h);

			bool isCubeMap() const { return m_is_cubemap; }

			DX11DepthBuffer(
				ID3D11Device* device,
				uint32 w, uint32 h,
				DXGI_FORMAT format,
				DXGI_SAMPLE_DESC sampling,
				bool cubemap,
				int flags
			);

			ID3D11DepthStencilView* GetDepthStencilView(uint32 index) const { return m_depthStencilViews[index].Get(); }
		};
	}
}
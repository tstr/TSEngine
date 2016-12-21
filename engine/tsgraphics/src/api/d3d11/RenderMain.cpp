/*
	Render API

	D3D11Render main methods
*/

#include "render.h"
#include "context.h"

#include <tscore/debug/log.h>
#include <vector>

//libraries to link
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace ts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Init
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
D3D11Render::D3D11Render(const SRenderApiConfig& cfg)
{
	m_hwnd = reinterpret_cast<HWND>(cfg.windowHandle);
	tsassert(IsWindow(m_hwnd));

	HRESULT hr = S_OK;

	m_apiFlags = cfg.flags;

	//////////////////////////////////////////////////////////////////////////////////////////////
	//initialize direct3D
	//////////////////////////////////////////////////////////////////////////////////////////////

	//Create swap chain
	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	//Swap chain description
	scd.BufferCount = 1; // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = m_hwnd;

	scd.SampleDesc.Count = cfg.display.multisampleCount;
	scd.SampleDesc.Quality = 0;
	scd.Windowed = !cfg.display.fullscreen;
	scd.BufferDesc.Width = cfg.display.resolutionW;
	scd.BufferDesc.Height = cfg.display.resolutionH;

	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scd.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;

	m_swapChainFlags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	scd.Flags = m_swapChainFlags;

	UINT flags = 0;
	//UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT; //for D2D and D3D interop

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	D3D_FEATURE_LEVEL featureLevel;

	if (cfg.flags & ERenderApiFlags::eFlagDebug)
	{
		flags |= D3D11_CREATE_DEVICE_DEBUG;
	}

	try
	{
		//Create DXGI Factory
		hr = CreateDXGIFactory(IID_OF(IDXGIFactory), (void**)m_dxgiFactory.GetAddressOf());

		if (FAILED(hr))
		{
			throw _com_error(hr);
		}

		hr = m_dxgiFactory->EnumAdapters(cfg.adapterIndex, m_dxgiAdapter.GetAddressOf());
		if (FAILED(hr)) throw _com_error(hr);

		//Create D3D11 Device and Device Context
		hr = D3D11CreateDevice(
			m_dxgiAdapter.Get(),
			D3D_DRIVER_TYPE_UNKNOWN,//D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			flags,
			featureLevels,
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION,
			m_device.GetAddressOf(),
			&featureLevel,
			m_immediateContext.GetAddressOf()
		);
		
		if (FAILED(hr))
		{
			throw _com_error(hr);
		}

		getMultisampleQuality(scd.SampleDesc);

		//Create DXGI Swap Chain
		m_dxgiFactory->CreateSwapChain(
			(IUnknown*)m_device.Get(),
			&scd,
			m_dxgiSwapchain.GetAddressOf()
		);

		if (FAILED(hr))
		{
			throw _com_error(hr);
		}

		m_dxgiFactory->MakeWindowAssociation(
			m_hwnd,
			DXGI_MWA_NO_WINDOW_CHANGES
		);

		if (FAILED(hr))
		{
			throw _com_error(hr);
		}
	}
	catch (_com_error& e)
	{
		std::stringstream stream;
		stream << std::hex << "D3D11CreateDeviceAndSwapChain failure. HRESULT (0x" << hr << "): " << e.ErrorMessage();
		tserror(stream.str());

		return;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////

	//Create the new render target view
	ComPtr<ID3D11Texture2D> backbuffer;
	hr = m_dxgiSwapchain->GetBuffer(0, IID_OF(ID3D11Texture2D), (void**)backbuffer.GetAddressOf());
	tsassert(SUCCEEDED(hr));
	hr = m_device->CreateRenderTargetView(backbuffer.Get(), nullptr, m_swapChainRenderTarget.GetAddressOf());
	tsassert(SUCCEEDED(hr));

	//States
	{
		D3D11_BLEND_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.AlphaToCoverageEnable = false;
		desc.RenderTarget[0].BlendEnable = true;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		m_device->CreateBlendState(&desc, m_blendState.GetAddressOf());
	}

	{
		D3D11_DEPTH_STENCIL_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.DepthEnable = false;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		desc.StencilEnable = false;
		desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.BackFace = desc.FrontFace;
		m_device->CreateDepthStencilState(&desc, m_depthStencilState.GetAddressOf());
	}

	{
		D3D11_RASTERIZER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_NONE;
		desc.ScissorEnable = true;
		desc.DepthClipEnable = true;
		m_device->CreateRasterizerState(&desc, m_rasterizerState.GetAddressOf());
	}

	//Initialize values
	m_displayNeedRebuild.store(false);
	m_drawActive.store(false);

	//////////////////////////////////////////////////////////////////////////////////////////////
}

D3D11Render::~D3D11Render()
{
	//If the swapchain is in fullscreen mode then exit before releasing the swapchain
	if (m_dxgiSwapchain.Get())
	{
		m_dxgiSwapchain->SetFullscreenState(false, nullptr);
	}

	if (m_apiFlags & ERenderApiFlags::eFlagReportObjects)
	{
		ComPtr<ID3D11Debug> debug;
		m_device.As(&debug);
		debug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Context methods
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D3D11Render::createContext(IRenderContext** context)
{
	auto rc = new D3D11RenderContext(this);
	*context = rc;
	m_renderContexts.push_back(rc);
}

void D3D11Render::destroyContext(IRenderContext* context)
{
	//upcast
	if (auto ptr = dynamic_cast<D3D11RenderContext*>(context))
	{
		auto it = find(m_renderContexts.begin(), m_renderContexts.end(), ptr);
		m_renderContexts.erase(it);
		delete ptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Pipeline methods
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D3D11Render::drawBegin(const Vector& vec)
{
	//Sets drawing status to active
	//If status was already marked as active then show error
	tsassert(!m_drawActive.exchange(true));
}

void D3D11Render::drawEnd(IRenderContext** contexts, uint32 numContexts)
{
	for (int i = 0; i < numContexts; i++)
	{
		if (auto rcon = dynamic_cast<D3D11RenderContext*>(contexts[i]))
		{
			if (auto cmdlist = rcon->getCommandList().Get())
			{
				m_immediateContext->ExecuteCommandList(cmdlist, false);
				rcon->resetCommandList(); //releases all outstanding references to swapchain
			}
		}
	}

	//Handle rebuilding of swapchain if changes were marked
	//MUST BE CALLED AT THIS POINT
	tryRebuildDisplay();

	//Sets drawing status to inactive
	//If status was already marked as inactive then show error
	tsassert(m_drawActive.exchange(false));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	namespace abi
	{
		extern "C"
		{
			int createRenderApi(IRender** api, const SRenderApiConfig& cfg)
			{
				*api = new D3D11Render(cfg);
				return 0;
			}

			void destroyRenderApi(IRender* api)
			{
				if (auto ptr = dynamic_cast<ts::D3D11Render*>(api))
				{
					delete ptr;
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
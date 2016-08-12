/*
	Render API

	DirectX 11 implementation of Render API source
*/

#include "DX11Render.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

using namespace ts;
using namespace ts::dx11;

/////////////////////////////////////////////////////////////////////////////////////////////////

//This global variable forces systems using Nvidia Optimus to enable the high performance gpu
extern "C"
{
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

DX11RenderApi::DX11RenderApi(const SRenderApiConfiguration& cfg)
{
	HWND hwnd = reinterpret_cast<HWND>(cfg.windowHandle);
	tsassert(IsWindow(hwnd));

	HRESULT hr = S_OK;

	//////////////////////////////////////////////////////////////////////////////////////////////
	//DXGI
	//////////////////////////////////////////////////////////////////////////////////////////////

	ComPtr<IDXGIFactory> dxgiFactory;
	ComPtr<IDXGIAdapter> dxgiAdapter;

	hr = CreateDXGIFactory(IID_OF(IDXGIFactory), (void**)dxgiFactory.GetAddressOf());
	tsassert(SUCCEEDED(hr));

	hr = S_OK;
	UINT i = 0;
	while (true)
	{
		ComPtr<IDXGIAdapter> adapter;
		hr = dxgiFactory->EnumAdapters(i, adapter.GetAddressOf());

		if (hr == DXGI_ERROR_NOT_FOUND)
		{
			break;
		}
		DXGI_ADAPTER_DESC adapterDesc;
		adapter->GetDesc(&adapterDesc);
		_bstr_t str = adapterDesc.Description;
		tsinfo("DXGI adapter: %", (const char*)str);

		i++;
	}

	hr = dxgiFactory->EnumAdapters(cfg.adapterIndex, dxgiAdapter.GetAddressOf());
	
	//dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

	//////////////////////////////////////////////////////////////////////////////////////////////
	//initialize direct3D
	//////////////////////////////////////////////////////////////////////////////////////////////

	UINT msaaCount = 1;
	UINT msaaQuality = 0;
	msaaQuality = D3D11_STANDARD_MULTISAMPLE_PATTERN;

	//Create swap chain
	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	//Swap chain description
	scd.BufferCount = 1; // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hwnd;

	scd.SampleDesc.Count = msaaCount;
	scd.SampleDesc.Quality = msaaQuality;
	scd.Windowed = !cfg.windowFullscreen;
	scd.BufferDesc.Width = cfg.resolutionWidth;
	scd.BufferDesc.Height = cfg.resolutionHeight;

	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//scd.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD;

	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//VSync
	{
		scd.BufferDesc.RefreshRate.Numerator = GetDeviceCaps(GetDC(hwnd), VREFRESH);
		scd.BufferDesc.RefreshRate.Denominator = 1;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////

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

	//if (settings.debug)
	{
		flags |= D3D11_CREATE_DEVICE_DEBUG;
	}

	try
	{
		hr = D3D11CreateDeviceAndSwapChain(
			dxgiAdapter.Get(),
			D3D_DRIVER_TYPE_UNKNOWN,//D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			flags,
			featureLevels,
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION,
			&scd,
			m_dxgiSwapchain.GetAddressOf(),
			m_device.GetAddressOf(),
			&featureLevel,
			m_immediateContext.GetAddressOf()
		);
		
		if (FAILED(hr))
		{
			throw _com_error(hr);
		}

	}
	catch (_com_error& e)
	{
		std::stringstream stream;
		stream << std::hex << "D3D11CreateDeviceAndSwapChain failure. HRESULT (0x" << hr << "): " << (const char*)e.ErrorMessage();
		tserror(stream.str());

		return;
	}


	//For the moment just clear the backbuffer and present

	ComPtr<ID3D11Texture2D> backbuffer;
	hr = m_dxgiSwapchain->GetBuffer(0, IID_OF(ID3D11Texture2D), (void**)backbuffer.GetAddressOf());
	tsassert(SUCCEEDED(hr));
	hr = m_device->CreateRenderTargetView(backbuffer.Get(), nullptr, m_swapChainRenderTarget.GetAddressOf());
	tsassert(SUCCEEDED(hr));


}

DX11RenderApi::~DX11RenderApi()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////

void DX11RenderApi::drawBegin(const Vector& vec)
{
	const float colour[] = { vec.x(), vec.y(), vec.z(), 1.0f };
	m_immediateContext->ClearRenderTargetView(m_swapChainRenderTarget.Get(), colour);
}

void DX11RenderApi::drawEnd()
{
	m_dxgiSwapchain->Present(0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
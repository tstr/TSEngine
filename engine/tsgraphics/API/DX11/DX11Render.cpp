/*
	Render API

	DirectX 11 implementation of Render API source
*/

#include "DX11Render.h"

#include <tscore/debug/log.h>

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
	m_hwnd = reinterpret_cast<HWND>(cfg.windowHandle);
	tsassert(IsWindow(m_hwnd));

	HRESULT hr = S_OK;

	//////////////////////////////////////////////////////////////////////////////////////////////
	//DXGI
	//////////////////////////////////////////////////////////////////////////////////////////////

	ComPtr<IDXGIFactory> dxgiFactory;
	ComPtr<IDXGIAdapter> dxgiAdapter;

	hr = CreateDXGIFactory(IID_OF(IDXGIFactory), (void**)dxgiFactory.GetAddressOf());
	tsassert(SUCCEEDED(hr));

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
	scd.OutputWindow = m_hwnd;

	scd.SampleDesc.Count = msaaCount;
	scd.SampleDesc.Quality = msaaQuality;
	scd.Windowed = cfg.windowMode != EWindowMode::eWindowFullscreen;
	scd.BufferDesc.Width = cfg.resolutionWidth;
	scd.BufferDesc.Height = cfg.resolutionHeight;

	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//scd.SwapEffect = DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD;

	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//VSync
	{
		scd.BufferDesc.RefreshRate.Numerator = GetDeviceCaps(GetDC(m_hwnd), VREFRESH);
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

	if (cfg.flags & ERenderApiFlags::eFlagDebug)
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
		stream << std::hex << "D3D11CreateDeviceAndSwapChain failure. HRESULT (0x" << hr << "): " << e.ErrorMessage();
		tserror(stream.str());

		return;
	}

	setWindowMode(cfg.windowMode);

	ComPtr<ID3D11Texture2D> backbuffer;
	hr = m_dxgiSwapchain->GetBuffer(0, IID_OF(ID3D11Texture2D), (void**)backbuffer.GetAddressOf());
	tsassert(SUCCEEDED(hr));
	hr = m_device->CreateRenderTargetView(backbuffer.Get(), nullptr, m_swapChainRenderTarget.GetAddressOf());
	tsassert(SUCCEEDED(hr));

	//Save a copy of the configuration
	m_config = cfg;
}

DX11RenderApi::~DX11RenderApi()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////

void DX11RenderApi::setWindowMode(EWindowMode mode)
{
	if (mode == m_config.windowMode)
		return;

	switch (mode)
	{
	case (EWindowMode::eWindowDefault):

		if (m_config.windowMode == eWindowFullscreen)
		{
			//Exit windowed mode
			HRESULT hr = m_dxgiSwapchain->SetFullscreenState(false, nullptr);
			if (FAILED(hr))
				tswarn("failed to exit fullscreen mode");
		}
		else
		{
			//Exit borderless mode
			SetWindowLongPtr(m_hwnd, GWL_EXSTYLE, WS_EX_LEFT);
			SetWindowLongPtr(m_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);

			SetWindowPos(m_hwnd, HWND_NOTOPMOST, 0, 0, m_config.resolutionWidth, m_config.resolutionHeight, SWP_SHOWWINDOW);
			ShowWindow(m_hwnd, SW_RESTORE);
		}

		break;

	case (EWindowMode::eWindowBorderless):

		if (m_config.windowMode == eWindowFullscreen)
		{
			HRESULT hr = m_dxgiSwapchain->SetFullscreenState(false, nullptr);
			if (FAILED(hr))
				tswarn("failed to exit fullscreen mode");
		}
		{
			//Set borderless mode
			DEVMODE dev;
			ZeroMemory(&dev, sizeof(DEVMODE));

			int width = GetSystemMetrics(SM_CXSCREEN),
				height = GetSystemMetrics(SM_CYSCREEN);

			EnumDisplaySettings(NULL, 0, &dev);

			HDC context = GetWindowDC(m_hwnd);
			int colourBits = GetDeviceCaps(context, BITSPIXEL);
			int refreshRate = GetDeviceCaps(context, VREFRESH);

			dev.dmPelsWidth = width;
			dev.dmPelsHeight = height;
			dev.dmBitsPerPel = colourBits;
			dev.dmDisplayFrequency = refreshRate;
			dev.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

			//todo: fix
			LONG result = ChangeDisplaySettingsA(&dev, CDS_FULLSCREEN);// == DISP_CHANGE_SUCCESSFUL);
			//tserror("ChangeDisplaySettings returned %", result);

			SetWindowLongPtr(m_hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
			SetWindowPos(m_hwnd, HWND_TOP, 0, 0, width, height, 0);
			BringWindowToTop(m_hwnd);
		}

		break;

	case (EWindowMode::eWindowFullscreen):

		HRESULT hr = m_dxgiSwapchain->SetFullscreenState(true, nullptr);
		if (FAILED(hr))
			tswarn("failed to enter fullscreen mode");

		break;
	}

	m_config.windowMode = mode;
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
/*
	Render API

	D3D11 driver main methods
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
Dx11::Dx11(const RenderDeviceConfig& cfg) :
	m_displayResourceProxy(nullptr)
{
	m_hwnd = reinterpret_cast<HWND>(cfg.windowHandle);
	tsassert(IsWindow(m_hwnd));

	HRESULT hr = S_OK;

	//initialize members
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

	scd.SampleDesc.Count = cfg.display.multisampleLevel;
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

	if (cfg.flags & RenderDeviceConfig::DEBUG)
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

	/*
	//m_dxgiAdapter->EnumOutputs();
	m_dxgiSwapchain->GetContainingOutput(m_dxgiOutput.ReleaseAndGetAddressOf());

	DXGI_MODE_DESC modeDescs[128];
	UINT modeCount = ARRAYSIZE(modeDescs);

	m_dxgiOutput->GetDisplayModeList(scd.BufferDesc.Format, 3, &modeCount, modeDescs);

	for (int i = 0; i < modeCount; i++)
	{
		const DXGI_MODE_DESC& m = modeDescs[i];

		OutputDebugStringA("===========================================\n");
		OutputDebugStringA(format("Resolution: %x%\n", m.Width, m.Height).c_str());
		OutputDebugStringA(format("Refresh Rata: %/%\n", m.RefreshRate.Denominator, m.RefreshRate.Numerator).c_str());
		OutputDebugStringA(format("Format: %\n", m.Format).c_str());
		OutputDebugStringA(format("Scaling: %\n", m.Scaling).c_str());
		OutputDebugStringA(format("Scanline Ordering: %\n", m.ScanlineOrdering).c_str());
	}
	*/

	//Setup resource proxies for this swapchain
	updateDisplayResource();

	m_stateManager = DxStateManager(m_device.Get());
	m_context = Dx11Context(this);

	//////////////////////////////////////////////////////////////////////////////////////////////
}

Dx11::~Dx11()
{
	//If the swapchain is in fullscreen mode then exit before releasing the swapchain
	if (m_dxgiSwapchain.Get())
	{
		m_dxgiSwapchain->SetFullscreenState(false, nullptr);
	}

	if (m_apiFlags & RenderDeviceConfig::DEBUG_REPORT)
	{
		ComPtr<ID3D11Debug> debug;
		m_device.As(&debug);
		debug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Dx11::commit()
{	
	if (ID3D11CommandList* cmdlist = m_context.getCommandList())
	{
		m_immediateContext->ExecuteCommandList(cmdlist, false);
		m_context.resetCommandList(); //Every command list must be released every frame
	}

	//Send queued commands to the GPU and present swapchain backbuffer
	m_dxgiSwapchain->Present(0, 0);
	//Reset draw call counter each frame
	m_drawCallCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Dx11::queryStats(RenderStats& stats)
{
	stats.drawcalls = m_drawCallCounter.load();
}

void Dx11::queryInfo(RenderDeviceInfo& info)
{
	DXGI_ADAPTER_DESC desc;
	m_dxgiAdapter->GetDesc(&desc);

	_bstr_t str = desc.Description;
	info.adapterName = (const char*)str;
	info.gpuVideoMemory = desc.DedicatedVideoMemory;
	info.gpuSystemMemory = desc.DedicatedSystemMemory;
	info.sharedSystemMemory = desc.SharedSystemMemory;
}

//helper function
bool Dx11::getMultisampleQuality(DXGI_SAMPLE_DESC& sampledesc)
{
	tsassert(m_device.Get());
	tsassert(SUCCEEDED(m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, sampledesc.Count, &sampledesc.Quality)));
	sampledesc.Quality--;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C"
{
	RenderDevice* createDX11device(const RenderDeviceConfig& config)
	{
		return new Dx11(config);
	}

	void destroyDX11device(RenderDevice* device)
	{
		if (auto d = dynamic_cast<Dx11*>(device))
		{
			delete d;
		}
	}
}

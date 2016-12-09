/*
	Render API

	DirectX 11 implementation of Render API source
*/

#include "DX11Render.h"
#include "DX11helpers.h"

#include <tscore/debug/log.h>

#include <vector>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

using namespace std;
using namespace ts;
using namespace ts::dx11;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DX11RenderApi::DX11RenderApi(const SRenderApiConfiguration& cfg)
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

	scd.SampleDesc.Count = cfg.multisampleCount;
	scd.SampleDesc.Quality = 0;
	scd.Windowed = !cfg.fullscreen;
	scd.BufferDesc.Width = cfg.resolutionWidth;
	scd.BufferDesc.Height = cfg.resolutionHeight;

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

		//Get the first output for now
		hr = m_dxgiAdapter->EnumOutputs(0, m_dxgiOutput.GetAddressOf());
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
	m_cachedRes = 0;
	m_cachedSampling = 0;

	//////////////////////////////////////////////////////////////////////////////////////////////
}

DX11RenderApi::~DX11RenderApi()
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

bool DX11RenderApi::getMultisampleQuality(DXGI_SAMPLE_DESC& sampledesc)
{
	tsassert(m_device.Get());
	tsassert(SUCCEEDED(m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, sampledesc.Count, &sampledesc.Quality)));
	sampledesc.Quality--;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DX11RenderApi::setDisplayResolution(uint32 width, uint32 height)
{
	if (width && height)
		m_cachedRes.store(MAKELPARAM((uint16)width, (uint16)height));
	else
		tswarn("Invalid resolution (w:%)(h:%)", width, height);
}

void DX11RenderApi::setDisplayMultisampleCount(uint32 samplecount)
{
	if (samplecount)
		m_cachedSampling.store(samplecount);
	else
		tswarn("Invalid multisample count %", samplecount);
}

void DX11RenderApi::setDisplayFullscreenState(bool fullscreen)
{
	HRESULT hr = S_OK;

	DXGI_SWAP_CHAIN_DESC desc;
	m_dxgiSwapchain->GetDesc(&desc);

	//Zero out refresh rate values because DXGI calculates them automatically
	desc.BufferDesc.RefreshRate.Denominator = 0;
	desc.BufferDesc.RefreshRate.Numerator = 0;

	//Enter
	if (fullscreen)
	{
		//Call resizetarget before entering fullscreen
		if (FAILED(hr = m_dxgiSwapchain->ResizeTarget(&desc.BufferDesc)))
			tserror("unable to resize IDXGISwapChain target. Error: \"%\"", _com_error(hr).ErrorMessage());

		if (FAILED(hr = m_dxgiSwapchain->SetFullscreenState(true, nullptr)))
			tserror("failed to enter fullscreen mode. Error: \"%\"", _com_error(hr).ErrorMessage());

		//Call resize target again in order to prevent issues with the refresh rate
		if (FAILED(hr = m_dxgiSwapchain->ResizeTarget(&desc.BufferDesc)))
			tserror("unable to resize IDXGISwapChain target. Error: \"%\"", _com_error(hr).ErrorMessage());
	}
	//Exit
	else
	{
		if (FAILED(hr = m_dxgiSwapchain->SetFullscreenState(false, nullptr)))
			tserror("failed to exit fullscreen mode. Error: \"%\"", _com_error(hr).ErrorMessage());
	}
} 

bool DX11RenderApi::getDisplayFullscreenState() const
{
	HRESULT hr = S_OK;
	BOOL screenstate = FALSE;

	if (FAILED(hr =m_dxgiSwapchain->GetFullscreenState(&screenstate, nullptr)))
		tswarn("failed to get fullscreen mode. Error: \"%\"", _com_error(hr).ErrorMessage());

	return (screenstate != FALSE);
}

void DX11RenderApi::getDisplayRenderTarget(ResourceProxy& target)
{
	target.reset(new DX11View(this, this->m_swapChainRenderTarget.Get(), STextureViewDescriptor()));
}

void DX11RenderApi::getDrawStatistics(SRenderStatistics& stats)
{
	stats.drawcalls = m_drawCallCounter.load();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DX11RenderApi::drawBegin(const Vector& vec)
{
	//Changing states (resolution/msaa/fullscreenstate) must be done outside of rendering but on the rendering thread so we execute these state changes before rendering
	{
		HRESULT hr = S_OK;

		auto res = m_cachedRes.exchange(0);
		auto sample = m_cachedSampling.exchange(0); //Multisample count
		uint32 w = LOWORD(res); //New resolution width
		uint32 h = HIWORD(res); //New resolution height

		//Get current fullscreen state
		BOOL screenstate = FALSE;
		if (FAILED(hr = m_dxgiSwapchain->GetFullscreenState(&screenstate, nullptr)))
			tswarn("failed to get fullscreen mode. Error: \"%\"", _com_error(hr).ErrorMessage());

		bool res_changed = (w != 0) || (h != 0);
		bool sample_changed = (sample != 0);

		if (res_changed || sample_changed)
		{
			w = max(1u, w);
			h = max(1u, h);

			//Reset deferred contexts
			for (auto& rc : m_renderContexts)
			{
				rc->resetCommandList();
			}

			//Destroy the old render target view
			m_swapChainRenderTarget.Reset();

			//If the multisampling count value has changed then rebuild the swap chain
			if (sample_changed)
			{
				//If the current state of the swapchain is fullscreen then we must exit in order to rebuild the swapchain
				if (screenstate)
					hr = m_dxgiSwapchain->SetFullscreenState(false, nullptr);

				DXGI_SWAP_CHAIN_DESC desc;
				m_dxgiSwapchain->GetDesc(&desc);

				//Because we are rebuilding the swap chain we can update the resolution/fullscreen state here instead of in separate calls to ResizeBuffers/SetFullscreenState
				desc.Windowed = !screenstate;

				if (res_changed)
				{
					desc.BufferDesc.Width = w;
					desc.BufferDesc.Height = h;
				}

				desc.SampleDesc.Count = sample;
				getMultisampleQuality(desc.SampleDesc);

				desc.Flags = m_swapChainFlags;

				//Release swapchain
				m_dxgiSwapchain.Reset();

				//Recreate swapchain
				tsassert(SUCCEEDED(m_dxgiFactory->CreateSwapChain((IUnknown*)m_device.Get(), &desc, m_dxgiSwapchain.GetAddressOf())));

				m_dxgiFactory->MakeWindowAssociation(
					m_hwnd,
					DXGI_MWA_NO_WINDOW_CHANGES
				);
			}
			else if (res_changed)
			{
				//Only resize if width/height differ
				DXGI_SWAP_CHAIN_DESC desc;
				m_dxgiSwapchain->GetDesc(&desc);

				//Resize swapchain - DXGI_FORMAT_R8G8B8A8_UNORM
				//if (FAILED(m_dxgiSwapchain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, desc.Flags)))
				if (FAILED(m_dxgiSwapchain->ResizeBuffers(2, w, h, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH)))
					tswarn("unable to resize IDXGISwapChain");
			}

			//Create the new render target view
			ComPtr<ID3D11Texture2D> backbuffer;
			HRESULT hr = m_dxgiSwapchain->GetBuffer(0, IID_OF(ID3D11Texture2D), (void**)backbuffer.GetAddressOf());
			tsassert(SUCCEEDED(hr));
			hr = m_device->CreateRenderTargetView(backbuffer.Get(), nullptr, m_swapChainRenderTarget.GetAddressOf());
			tsassert(SUCCEEDED(hr));
		}
	}

	//Clear the backbuffer
	const float colour[] = { vec.x(), vec.y(), vec.z(), 1.0f };
	m_immediateContext->ClearRenderTargetView(m_swapChainRenderTarget.Get(), colour);
}

void DX11RenderApi::drawEnd()
{
	m_dxgiSwapchain->Present(0, 0);

	//reset the counter
	m_drawCallCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Resource creation methods
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ERenderStatus DX11RenderApi::createResourceBuffer(ResourceProxy& rsc, const SBufferResourceData& data)
{
	ComPtr<ID3D11Buffer> buffer;

	D3D11_SUBRESOURCE_DATA subdata;
	D3D11_BUFFER_DESC subdesc;

	ZeroMemory(&subdata, sizeof(D3D11_SUBRESOURCE_DATA));
	ZeroMemory(&subdesc, sizeof(D3D11_BUFFER_DESC));

	//For now keep resource usage as default
	subdesc.Usage = D3D11_USAGE_DEFAULT;

	switch (data.usage)
	{
		case EBufferType::eBufferTypeVertex: { subdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; break; }
		case EBufferType::eBufferTypeIndex: { subdesc.BindFlags = D3D11_BIND_INDEX_BUFFER; break; }
		case EBufferType::eBufferTypeUniform: { subdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; break; }
	}
	
	//Only dynamic resources are allowed direct access to buffer memory
	if (subdesc.Usage == D3D11_USAGE_DYNAMIC)
	{
		subdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	subdesc.MiscFlags = 0;
	subdesc.ByteWidth = data.size;

	subdata.pSysMem = data.memory;
	subdata.SysMemPitch = 0;
	subdata.SysMemSlicePitch = 0;

	HRESULT hr = m_device->CreateBuffer(&subdesc, &subdata, buffer.GetAddressOf());
	
	if (FAILED(hr))
	{
		return RenderStatusFromHRESULT(hr);
	}

	rsc.reset(new DX11Buffer(this, buffer));

	return eOk;
}


ERenderStatus DX11RenderApi::createResourceTexture(ResourceProxy& rsc, const STextureResourceData* data, const STextureResourceDescriptor& desc)
{
	DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	D3D11_USAGE usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	D3D11_CPU_ACCESS_FLAG access = D3D11_CPU_ACCESS_READ;

	format = TextureFormatToDXGIFormat(desc.texformat);

	/*
	//todo: add options for other resource usages
	switch ((EResourceUsage)desc.rscusage)
	{
		case EResourceUsage::UsageDefault: { usage = D3D11_USAGE_DEFAULT; break; }
		case EResourceUsage::UsageDynamic: { usage = D3D11_USAGE_DYNAMIC; break; }
		case EResourceUsage::UsageStaging: { usage = D3D11_USAGE_STAGING; break; }
		case EResourceUsage::UsageStatic: { usage = D3D11_USAGE_IMMUTABLE; break; }
	}
	*/

	//Force default usage
	if (desc.useMips)
	{
		usage = D3D11_USAGE_DEFAULT;
	}

	if (desc.arraySize > 1)
	{
		usage = D3D11_USAGE_DEFAULT;
	}

	if (usage == D3D11_USAGE_DEFAULT)
	{
		access = D3D11_CPU_ACCESS_FLAG(0);
	}
	else if (usage == D3D11_USAGE_DYNAMIC)
	{
		access = D3D11_CPU_ACCESS_WRITE;
	}
	else
	{
		access = D3D11_CPU_ACCESS_READ;
	}
	
	uint32 miplevels = (desc.useMips) ? getNumMipLevels(desc.width, desc.height) : 1;
	uint32 miscFlags = (desc.textype == ETextureResourceType::eTypeTextureCube) ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;
	uint32 arraySize = (desc.textype == ETextureResourceType::eTypeTextureCube) ? 6 : 1; //Do not allow texture arrays for now

	vector<D3D11_SUBRESOURCE_DATA> subresources(arraySize);

	if (data)
	{
		for (uint32 i = 0; i < arraySize; i++)
		{
			subresources[i].pSysMem = data[i].memory;
			subresources[i].SysMemPitch = data[i].memoryByteWidth;
			subresources[i].SysMemSlicePitch = data[i].memoryByteDepth;
		}
	}

	D3D11_SUBRESOURCE_DATA* pSubresource = (desc.useMips || !data) ? nullptr : &subresources[0];

	if (desc.useMips)
	{
		uint32 fmtSupport;
		m_device->CheckFormatSupport(format, &fmtSupport);
		tsassert(fmtSupport & D3D11_FORMAT_SUPPORT_MIP_AUTOGEN);
	}

	//Create a temporary texture resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.Format = format;
	sd.Texture1D.MipLevels = miplevels;

	HRESULT hr = S_OK;

	ComPtr<ID3D11Resource> resource;

	UINT bindFlags = 0;

	if (desc.texmask & eTextureMaskShaderResource)
	{
		bindFlags |= D3D11_BIND_SHADER_RESOURCE;
	}
	if (desc.texmask & eTextureMaskRenderTarget)
	{
		bindFlags |= D3D11_BIND_RENDER_TARGET;
	}
	if (desc.texmask & eTextureMaskDepthTarget)
	{
		if (bindFlags & D3D11_BIND_RENDER_TARGET)
		{
			tswarn("A texture resource with the D3D11_BIND_RENDER_TARGET flag cannot have the D3D11_BIND_DEPTH_STENCIL flag set");
			return eInvalidParameter;
		}

		bindFlags |= D3D11_BIND_DEPTH_STENCIL;
	}

	//Create texture resource
	switch (desc.textype)
	{
	case (ETextureResourceType::eTypeTexture1D):
	{
		D3D11_TEXTURE1D_DESC dtd;
		ZeroMemory(&dtd, sizeof(dtd));

		dtd.BindFlags = bindFlags;
		dtd.Width = desc.width;
		dtd.ArraySize = arraySize;
		dtd.MipLevels = miplevels;
		dtd.Format = format;
		dtd.MiscFlags = miscFlags;
		dtd.Usage = usage;
		dtd.CPUAccessFlags = access;

		if (desc.useMips)
		{
			dtd.BindFlags |= D3D11_BIND_RENDER_TARGET;
			dtd.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}

		hr = m_device->CreateTexture1D(&dtd, pSubresource, (ID3D11Texture1D**)resource.GetAddressOf());
		sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;

		break;
	}
	case (ETextureResourceType::eTypeTextureCube):
	case (ETextureResourceType::eTypeTexture2D):
	{
		D3D11_TEXTURE2D_DESC dtd;
		ZeroMemory(&dtd, sizeof(dtd));

		dtd.BindFlags = bindFlags;
		dtd.Width = desc.width;
		dtd.Height = desc.height;
		dtd.Format = format;
		dtd.MiscFlags = miscFlags;
		dtd.ArraySize = arraySize;
		dtd.Usage = usage;
		dtd.CPUAccessFlags = access;
		dtd.MipLevels = miplevels;
		dtd.SampleDesc.Count = desc.multisampling.count;
		getMultisampleQuality(dtd.SampleDesc);

		if (desc.useMips)
		{
			dtd.BindFlags |= D3D11_BIND_RENDER_TARGET;
			dtd.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}

		hr = m_device->CreateTexture2D(&dtd, pSubresource, (ID3D11Texture2D**)resource.GetAddressOf());
		sd.ViewDimension = (desc.textype == eTypeTextureCube) ? D3D11_SRV_DIMENSION_TEXTURECUBE : D3D11_SRV_DIMENSION_TEXTURE2D;

		break;
	}
	case (ETextureResourceType::eTypeTexture3D):
	{
		D3D11_TEXTURE3D_DESC dtd;
		ZeroMemory(&dtd, sizeof(dtd));

		dtd.BindFlags = bindFlags;
		dtd.Width = desc.width;
		dtd.Height = desc.height;
		dtd.Depth = desc.depth;
		dtd.Format = format;
		dtd.MipLevels = miplevels;
		dtd.MiscFlags = miscFlags;
		dtd.Usage = usage;
		dtd.CPUAccessFlags = access;

		if (desc.useMips)
		{
			dtd.BindFlags |= D3D11_BIND_RENDER_TARGET;
			dtd.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}

		hr = m_device->CreateTexture3D(&dtd, pSubresource, (ID3D11Texture3D**)resource.GetAddressOf());
		sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;

		break;
	}
	}

	if (FAILED(hr))
	{
		return RenderStatusFromHRESULT(hr);
	}

	if (desc.useMips)
	{
		//lock_guard<mutex>lk(m_rtv_lock);

		//Create a temporary texture resource view
		ComPtr<ID3D11ShaderResourceView> tempView;

		hr = m_device->CreateShaderResourceView(resource.Get(), &sd, tempView.GetAddressOf());

		for (int i = 0; (size_t)i < subresources.size(); i++)
		{
			m_immediateContext->UpdateSubresource(resource.Get(), 0, nullptr, subresources[i].pSysMem, subresources[i].SysMemPitch, subresources[i].SysMemSlicePitch);
		}

		m_immediateContext->GenerateMips(tempView.Get());
	}

	if (FAILED(hr))
	{
		return RenderStatusFromHRESULT(hr);
	}

	rsc.reset(new DX11Texture(this, resource, desc));

	return eOk;
}

inline D3D11_TEXTURE_ADDRESS_MODE getAddressMode(ETextureAddressMode mode)
{
	switch (mode)
	{
	case (eTextureAddressClamp): return D3D11_TEXTURE_ADDRESS_CLAMP;
	case (eTextureAddressMirror): return D3D11_TEXTURE_ADDRESS_MIRROR;
	case (eTextureAddressWrap): return D3D11_TEXTURE_ADDRESS_WRAP;
	}

	return D3D11_TEXTURE_ADDRESS_MODE(0);
}

ERenderStatus DX11RenderApi::createTextureSampler(ResourceProxy& rsc, const STextureSamplerDescriptor& desc)
{
	D3D11_SAMPLER_DESC sd;
	ZeroMemory(&sd, sizeof(D3D11_SAMPLER_DESC));
	
	sd.AddressU = getAddressMode(desc.addressU);
	sd.AddressV = getAddressMode(desc.addressV);
	sd.AddressW = getAddressMode(desc.addressW);

	switch (desc.filtering)
	{
	case eTextureFilterPoint:
		sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		break;
	case eTextureFilterBilinear:
		sd.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		break;
	case eTextureFilterTrilinear:
		sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		break;
	case eTextureFilterAnisotropic2x:
		sd.Filter = D3D11_FILTER_ANISOTROPIC;
		sd.MaxAnisotropy = 2;
		break;
	case eTextureFilterAnisotropic4x:
		sd.Filter = D3D11_FILTER_ANISOTROPIC;
		sd.MaxAnisotropy = 4;
		break;
	case eTextureFilterAnisotropic8x:
		sd.Filter = D3D11_FILTER_ANISOTROPIC;
		sd.MaxAnisotropy = 8;
		break;
	case eTextureFilterAnisotropic16x:
		sd.Filter = D3D11_FILTER_ANISOTROPIC;
		sd.MaxAnisotropy = 16;
		break;
	}
	
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.MinLOD = -FLT_MAX;
	sd.MaxLOD = FLT_MAX;
	sd.MipLODBias = 0.0f;

	ComPtr<ID3D11SamplerState> sampler;
	HRESULT hr = m_device->CreateSamplerState(&sd, sampler.GetAddressOf());
	
	if (FAILED(hr))
	{
		return RenderStatusFromHRESULT(hr);
	}

	rsc.reset(new DX11TextureSampler(this, sampler.Get()));

	return eOk;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Resource view creation methods
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ERenderStatus DX11RenderApi::createViewDepthTarget(ResourceProxy& view, const ResourceProxy& rsc, const STextureViewDescriptor& desc)
{
	if (rsc.getType() != EResourceType::eResourceTexture)
		return eInvalidResource;

	auto tex = DX11Texture::upcast(rsc.get());

	STextureResourceDescriptor texdesc;
	tex->getDesc(texdesc);

	//Return failure if the texture resource was not created as a depth stencil
	if (~texdesc.texmask & eTextureMaskDepthTarget)
		return eInvalidResource;

	bool multisampled = texdesc.multisampling.count > 1;

	D3D11_DEPTH_STENCIL_VIEW_DESC viewdesc;
	ZeroMemory(&viewdesc, sizeof(viewdesc));

	viewdesc.Format = TextureFormatToDXGIFormat(texdesc.texformat);
	viewdesc.ViewDimension = (multisampled) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
	viewdesc.Texture2DArray.FirstArraySlice = desc.arrayIndex;
	viewdesc.Texture2DArray.ArraySize = 1;

	ComPtr<ID3D11DepthStencilView> dsv;
	ComPtr<ID3D11Resource> dt(tex->get());
	HRESULT hr = m_device->CreateDepthStencilView(dt.Get(), &viewdesc, dsv.GetAddressOf());

	setObjectDebugName(dsv.Get(), format("depthstencil-idx-%", desc.arrayIndex).c_str());

	if (FAILED(hr))
	{
		return RenderStatusFromHRESULT(hr);
	}

	view.reset(new DX11View(this, dsv.Get(), desc));

	return eOk;
}

ERenderStatus DX11RenderApi::createViewRenderTarget(ResourceProxy& view, const ResourceProxy& rsc, const STextureViewDescriptor& desc)
{
	if (rsc.getType() != EResourceType::eResourceTexture)
		return eInvalidResource;

	auto tex = DX11Texture::upcast(rsc.get());

	STextureResourceDescriptor texdesc;
	tex->getDesc(texdesc);

	//Return failure if the texture resource was not created as a render target
	if (~texdesc.texmask & eTextureMaskRenderTarget)
		return eInvalidResource;

	bool multisampled = texdesc.multisampling.count > 1;

	D3D11_RENDER_TARGET_VIEW_DESC viewdesc;
	ZeroMemory(&viewdesc, sizeof(viewdesc));
	
	viewdesc.Format = TextureFormatToDXGIFormat(texdesc.texformat);
	viewdesc.ViewDimension = (multisampled) ? D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	viewdesc.Texture2DArray.FirstArraySlice = desc.arrayIndex;
	viewdesc.Texture2DArray.ArraySize = 1;
	viewdesc.Texture2DArray.MipSlice = 0;

	ComPtr<ID3D11RenderTargetView> rtv;
	ComPtr<ID3D11Resource> rt(tex->get());
	HRESULT hr = m_device->CreateRenderTargetView(rt.Get(), &viewdesc, rtv.GetAddressOf());

	setObjectDebugName(rtv.Get(), format("rendertarget-idx-%", desc.arrayIndex).c_str());

	if (FAILED(hr))
	{
		return RenderStatusFromHRESULT(hr);
	}

	view.reset(new DX11View(this, rtv.Get(), desc));

	return eOk;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ERenderStatus DX11RenderApi::createViewTextureCube(ResourceProxy& view, const ResourceProxy& rsc, const STextureViewDescriptor& desc)
{
	if (rsc.getType() != EResourceType::eResourceTexture)
		return eInvalidResource;

	auto tex = DX11Texture::upcast(rsc.get());

	STextureResourceDescriptor texdesc;
	tex->getDesc(texdesc);

	uint32 miplevels = (texdesc.useMips) ? getNumMipLevels(texdesc.width, texdesc.height) : 1;

	if (~texdesc.texmask & eTextureMaskShaderResource)
		return eInvalidResource;

	bool multisampled = texdesc.multisampling.count > 1;

	D3D11_SHADER_RESOURCE_VIEW_DESC viewdesc;
	ZeroMemory(&viewdesc, sizeof(viewdesc));

	viewdesc.Format = TextureFormatToDXGIFormat(texdesc.texformat);
	viewdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE; //todo sort out multisampling
	viewdesc.TextureCube.MipLevels = miplevels;
	viewdesc.TextureCube.MostDetailedMip = 0;

	ComPtr<ID3D11ShaderResourceView> srv;
	ComPtr<ID3D11Resource> t(tex->get());
	HRESULT hr = m_device->CreateShaderResourceView(t.Get(), &viewdesc, srv.GetAddressOf());

	setObjectDebugName(srv.Get(), "cubemapView");

	if (FAILED(hr))
	{
		return RenderStatusFromHRESULT(hr);
	}

	view.reset(new DX11View(this, srv.Get(), desc));

	return eOk;
}

ERenderStatus DX11RenderApi::createViewTexture2D(ResourceProxy& view, const ResourceProxy& rsc, const STextureViewDescriptor& desc)
{
	if (rsc.getType() != EResourceType::eResourceTexture)
		return eInvalidResource;

	auto tex = DX11Texture::upcast(rsc.get());

	STextureResourceDescriptor texdesc;
	tex->getDesc(texdesc);

	uint32 miplevels = (texdesc.useMips) ? getNumMipLevels(texdesc.width, texdesc.height) : 1;

	if (~texdesc.texmask & eTextureMaskShaderResource)
		return eInvalidResource;

	bool multisampled = texdesc.multisampling.count > 1;

	D3D11_SHADER_RESOURCE_VIEW_DESC viewdesc;
	ZeroMemory(&viewdesc, sizeof(viewdesc));

	viewdesc.Format = TextureFormatToDXGIFormat(texdesc.texformat);
	viewdesc.ViewDimension = (multisampled) ? D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY : D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	viewdesc.Texture2DArray.MipLevels = miplevels;
	viewdesc.Texture2DArray.MostDetailedMip = 0;
	viewdesc.Texture2DArray.ArraySize = 1;
	viewdesc.Texture2DArray.FirstArraySlice = desc.arrayIndex;

	ComPtr<ID3D11ShaderResourceView> srv;
	ComPtr<ID3D11Resource> t(tex->get());
	HRESULT hr = m_device->CreateShaderResourceView(t.Get(), &viewdesc, srv.GetAddressOf());

	setObjectDebugName(srv.Get(), format("texture2DView-idx-%-sz-%", desc.arrayIndex, desc.arrayCount).c_str());

	if (FAILED(hr))
	{
		return RenderStatusFromHRESULT(hr);
	}

	view.reset(new DX11View(this, srv.Get(), desc));

	return eOk;
}

ERenderStatus DX11RenderApi::createViewTexture3D(ResourceProxy& view, const ResourceProxy& rsc)
{
	if (rsc.getType() != EResourceType::eResourceTexture)
		return eInvalidResource;

	auto tex3D = DX11Texture::upcast(rsc.get());

	STextureResourceDescriptor texdesc;
	tex3D->getDesc(texdesc);

	uint32 miplevels = getNumMipLevels(texdesc.width, texdesc.height);

	if (~texdesc.texmask & eTextureMaskShaderResource)
		return eInvalidResource;

	bool multisampled = texdesc.multisampling.count > 1;

	D3D11_SHADER_RESOURCE_VIEW_DESC viewdesc;
	ZeroMemory(&viewdesc, sizeof(viewdesc));

	viewdesc.Format = TextureFormatToDXGIFormat(texdesc.texformat);
	viewdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	viewdesc.Texture3D.MipLevels = miplevels;
	viewdesc.Texture3D.MostDetailedMip = 0;

	ComPtr<ID3D11ShaderResourceView> srv;
	ComPtr<ID3D11Resource> t(tex3D->get());
	HRESULT hr = m_device->CreateShaderResourceView(t.Get(), &viewdesc, srv.GetAddressOf());

	setObjectDebugName(srv.Get(), "texture3DView");

	if (FAILED(hr))
	{
		return RenderStatusFromHRESULT(hr);
	}

	view.reset(new DX11View(this, srv.Get(), STextureViewDescriptor()));

	return eOk;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Shader creation methods
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ERenderStatus DX11RenderApi::createShader(ResourceProxy& shader, const void* bytecode, uint32 bytecodesize, EShaderStage stage)
{

	switch (stage)
	{
		case (EShaderStage::eShaderStageVertex):
		{
			ComPtr<ID3D11VertexShader> vertexshader;
			HRESULT hr = m_device->CreateVertexShader(bytecode, bytecodesize, nullptr, vertexshader.GetAddressOf());
			if (FAILED(hr)) return RenderStatusFromHRESULT(hr);
			shader.reset(new DX11Shader(this, vertexshader.Get(), MemoryBuffer(bytecode, bytecodesize)));
			return eOk;
		}

		case (EShaderStage::eShaderStagePixel):
		{
			ComPtr<ID3D11PixelShader> pixelshader;
			HRESULT hr = m_device->CreatePixelShader(bytecode, bytecodesize, nullptr, pixelshader.GetAddressOf());

			if (FAILED(hr)) return RenderStatusFromHRESULT(hr);

			shader.reset(new DX11Shader(this, pixelshader.Get(), MemoryBuffer(bytecode, bytecodesize)));
			return eOk;
		}

		case (EShaderStage::eShaderStageGeometry):
		{
			ComPtr<ID3D11GeometryShader> geometryshader;
			HRESULT hr = m_device->CreateGeometryShader(bytecode, bytecodesize, nullptr, geometryshader.GetAddressOf());

			if (FAILED(hr)) return RenderStatusFromHRESULT(hr);

			shader.reset(new DX11Shader(this, geometryshader.Get(), MemoryBuffer(bytecode, bytecodesize)));
			return eOk;
		}

		case (EShaderStage::eShaderStageHull):
		{
			ComPtr<ID3D11HullShader> hullshader;
			HRESULT hr = m_device->CreateHullShader(bytecode, bytecodesize, nullptr, hullshader.GetAddressOf());
			if (FAILED(hr)) return RenderStatusFromHRESULT(hr);

			shader.reset(new DX11Shader(this, hullshader.Get(), MemoryBuffer(bytecode, bytecodesize)));
			return eOk;
		}

		case (EShaderStage::eShaderStageDomain):
		{
			ComPtr<ID3D11DomainShader> domainshader;
			HRESULT hr = m_device->CreateDomainShader(bytecode, bytecodesize, nullptr, domainshader.GetAddressOf());

			if (FAILED(hr)) return RenderStatusFromHRESULT(hr);

			shader.reset(new DX11Shader(this, domainshader, MemoryBuffer(bytecode, bytecodesize)));
			return eOk;
		}

		case (EShaderStage::eShaderStageCompute):
		{
			ComPtr<ID3D11ComputeShader> computeshader;
			HRESULT hr = m_device->CreateComputeShader(bytecode, bytecodesize, nullptr, computeshader.GetAddressOf());

			if (FAILED(hr)) return RenderStatusFromHRESULT(hr);

			shader.reset(new DX11Shader(this, computeshader, MemoryBuffer(bytecode, bytecodesize)));
			return eOk;
		}

		default:
			return eInvalidParameter;
	}


	return eFail;
}

ERenderStatus DX11RenderApi::createShaderInputDescriptor(ResourceProxy& rsc, const ResourceProxy& vertexshader, const SShaderInputDescriptor* sids, uint32 sidnum)
{
	ComPtr<ID3D11InputLayout> inputlayout;

	auto vshader = DX11Shader::upcast(vertexshader.get());

	if (sidnum < 1)
		return eFail;

	void* bytecode = nullptr;
	uint32 bytecodesize = 0;
	vshader->getShaderBytecode(&bytecode, bytecodesize);

	if (bytecode == nullptr || !bytecodesize)
		return eInvalidShaderByteCode;

	vector<D3D11_INPUT_ELEMENT_DESC> desc(sidnum);
	desc.resize(sidnum);

	for (uint32 i = 0; i < sidnum; i++)
	{
		desc[i].AlignedByteOffset = sids[i].byteOffset;
		desc[i].InputSlot = sids[i].bufferSlot;
		desc[i].SemanticName = sids[i].semanticName;
		
		desc[i].SemanticIndex = 0;
		desc[i].InstanceDataStepRate = 0;

		if (sids[i].channel == EShaderInputChannel::eInputPerVertex)
			desc[i].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		else if (sids[i].channel == EShaderInputChannel::eInputPerInstance)
			desc[i].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_INSTANCE_DATA;

		//todo: matrix
		switch (sids[i].type)
		{
			case eShaderInputFloat: { desc[i].Format = DXGI_FORMAT_R32_FLOAT; break; }
			case eShaderInputFloat2: { desc[i].Format = DXGI_FORMAT_R32G32_FLOAT; break; }
			case eShaderInputFloat3: { desc[i].Format = DXGI_FORMAT_R32G32B32_FLOAT; break; }
			case eShaderInputFloat4: { desc[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break; }
			case eShaderInputInt32: { desc[i].Format = DXGI_FORMAT_R32_SINT; break; }
			case eShaderInputUint32: { desc[i].Format = DXGI_FORMAT_R32_UINT; break; }
			case eShaderInputRGB: {}
			case eShaderInputRGBA: { desc[i].Format = DXGI_FORMAT_R8G8B8A8_UNORM; break; }
			default: { desc[i].Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN; }
		}

	}

	HRESULT hr = m_device->CreateInputLayout(&desc[0], sidnum, bytecode, bytecodesize, inputlayout.GetAddressOf());

	if (FAILED(hr))
		return RenderStatusFromHRESULT(hr);

	rsc.reset(new DX11ShaderInputDescriptor(this, inputlayout));

	return eOk;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DX11RenderApi::createContext(IRenderContext** context)
{
	auto rc = new DX11RenderContext(this);
	*context = rc;
	m_renderContexts.push_back(rc);
}

void DX11RenderApi::destroyContext(IRenderContext* context)
{
	//upcast
	if (auto ptr = dynamic_cast<DX11RenderContext*>(context))
	{
		auto it = find(m_renderContexts.begin(), m_renderContexts.end(), ptr);
		m_renderContexts.erase(it);
		delete ptr;
	}
}

void DX11RenderApi::executeContext(IRenderContext* context)
{
	//upcast
	if (auto rcon = dynamic_cast<DX11RenderContext*>(context))
	{
		m_immediateContext->ExecuteCommandList(rcon->getCommandList().Get(), false);
		rcon->resetCommandList();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
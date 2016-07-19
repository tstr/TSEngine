/*
	Graphics source - renderer
	
	- manages lifetime of graphics resources and control of rendering pipeline
*/

#include "pch.h"

#include <CTxt\win32.h>
#include <CT\gfx\graphics.h>
#include <CT\gfx\abi\graphicsabi.h>

#include "libraryloader.h"

#include <string>
#include <unordered_map>

using namespace std;
using namespace CT;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Graphics::Impl
{
	GraphicsBackend m_backendloader;
	ABI::IRenderApi* m_api = nullptr;
	API m_apiEnum = API::API_NULL;


	Impl(Graphics* r, API id, const Graphics::Configuration& cfg);
	~Impl();
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

//decltype(ABI::CompileEffectCache)* f_compile = nullptr;

Graphics::Graphics(API id, const Graphics::Configuration& cfg) :
	pImpl(new Impl(this, id, cfg))
{
	/*
	ofstream out("ShaderUtils.ccfx", ios::binary);
	ifstream in("ShaderUtils.fx");

	ABI::ShaderTarget targetstages[] =
	{
		{ "VS", ABI::ShaderType::Vertex },
		{ "PS", ABI::ShaderType::Pixel }
	};

	CT_ASSERT(pImpl->m_backendloader.f_compile(in, out, (const ABI::ShaderTarget*)&targetstages, 2, true));

	out.close();
	*/
}

Graphics::~Graphics()
{
	if (pImpl)
		delete pImpl;
}

Graphics::Impl::Impl(Graphics* r, API id, const Graphics::Configuration& cfg) :
	m_backendloader(id)
{
	m_api = m_backendloader.f_create(cfg);
	m_apiEnum = id;
}

Graphics::Impl::~Impl()
{
	m_backendloader.f_destroy(m_api);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

void VECTOR_CALL Graphics::DrawBegin(Colour backbuffercolour)
{
	pImpl->m_api->DrawBegin(backbuffercolour);
}

void Graphics::DrawEnd()
{
	pImpl->m_api->DrawEnd();
}

ABI::IRenderApi* Graphics::api() const
{
	return pImpl->m_api;
}

Graphics::API Graphics::apiEnum() const
{
	return pImpl->m_apiEnum;
}

void Graphics::SetViewport(const Graphics::Viewport& vp)
{
	pImpl->m_api->SetViewport(vp);
}

void Graphics::GetViewport(Graphics::Viewport& vp)
{
	pImpl->m_api->GetViewport(vp);
}

void Graphics::QueryApi(Graphics::ApiInfo& i) const
{

}

void Graphics::QueryDevice(uint32 slot, Graphics::DeviceInfo& i) const
{

}

void Graphics::QueryDebugStatistics(DebugInfo& info) const
{
	api()->QueryDebugStatistics(info);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

GraphicsBuffer::GraphicsBuffer(
	Graphics* renderer,
	const void* mem,
	uint32 memsize,
	EResourceType buffer_type,
	EResourceUsage buffer_usage
	) : m_renderer(renderer)
{
	auto api = (ABI::IRenderApi*)m_renderer->api();
	m_hRsc = api->CreateBuffer(mem, memsize, buffer_type, buffer_usage);
}

GraphicsBuffer::~GraphicsBuffer()
{
	auto api = (ABI::IRenderApi*)m_renderer->api();
	api->DestroyBuffer(m_hRsc);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

class GraphicsRenderTarget::RenderTargetResource : public EffectResource
{
public:

	RenderTargetResource(Graphics* renderer, ResourceHandle rsc) :
		EffectResource(renderer)
	{
		m_rsc = rsc;
	}
};

GraphicsRenderTarget::GraphicsRenderTarget(Graphics* renderer, uint32 w, uint32 h, ETextureFormat format, Multisampling ms, bool cubemap) :
m_renderer(renderer)
{
	CT_ASSERT(m_renderer);
	
	{
		ABI::RenderTargetDescriptor desc;
		desc.height = h;
		desc.width = w;
		desc.isCubemap = cubemap;
		desc.useMips = false;
		desc.texformat = format;
		desc.sampling.count = ms.count;
		desc.sampling.quality = ms.quality;
		m_rtHandle = m_renderer->api()->CreateRenderTarget(desc);
	}

	{
		ABI::DepthBufferDescriptor desc;
		desc.height = h;
		desc.width = w;
		desc.isCubemap = cubemap;
		desc.sampling.count = ms.count;
		desc.sampling.quality = ms.quality;
		m_dbHandle = m_renderer->api()->CreateDepthBuffer(desc);
	}

	this->m_colourTexture = new GraphicsRenderTarget::RenderTargetResource(m_renderer, m_renderer->api()->GetViewTexture(m_rtHandle));
	this->m_depthTexture = new GraphicsRenderTarget::RenderTargetResource(m_renderer, m_renderer->api()->GetViewTexture(m_dbHandle));
}

GraphicsRenderTarget::~GraphicsRenderTarget()
{
	if (m_rtHandle)
		m_renderer->api()->DestroyRenderTarget(m_rtHandle);

	if (m_dbHandle)
		m_renderer->api()->DestroyDepthBuffer(m_dbHandle);

	if (m_colourTexture)
		delete (RenderTargetResource*)m_colourTexture;

	if (m_depthTexture)
		delete (RenderTargetResource*)m_depthTexture;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
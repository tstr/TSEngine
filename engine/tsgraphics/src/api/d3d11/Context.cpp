/*
	Render API

	D3D11 implementation of rendering context
*/

#include "context.h"
#include "helpers.h"
#include "target.h"

using namespace std;
using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Constructor
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

D3D11RenderContext::D3D11RenderContext(D3D11Render* api) :
	m_api(api)
{
	tsassert(api);
	auto device = m_api->getDevice();
	tsassert(SUCCEEDED(device->CreateDeferredContext(0, m_context.GetAddressOf())));
}

D3D11RenderContext::~D3D11RenderContext()
{

}

void D3D11RenderContext::finish()
{
	m_contextCommandList.Reset();

	//Store current state in a command list
	m_context->FinishCommandList(false, m_contextCommandList.GetAddressOf());
}

void D3D11RenderContext::resetCommandList()
{
	m_contextCommandList.Reset();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Resource updating
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D3D11RenderContext::clearRenderTarget(HTarget h, const Vector& vec)
{
	D3D11Target* target = D3D11Target::upcast(h);
	target->clearRenderTargets(m_context.Get(), vec);
}

void D3D11RenderContext::clearDepthTarget(HTarget h, float depth)
{
	D3D11Target* target = D3D11Target::upcast(h);
	target->clearDepthStencil(m_context.Get(), depth);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D3D11RenderContext::bufferUpdate(HBuffer rsc, const void* memory)
{
	auto buffer = reinterpret_cast<ID3D11Buffer*>(rsc);

	if (buffer)
	{
		m_context->UpdateSubresource(buffer, 0, nullptr, memory, 0, 0);
	}
	else
	{
		tswarn("unable to update buffer");
	}
}

void D3D11RenderContext::bufferCopy(HBuffer src, HBuffer dest)
{
	auto buffersrc = reinterpret_cast<ID3D11Buffer*>(src);
	auto bufferdest = reinterpret_cast<ID3D11Buffer*>(dest);

	if (buffersrc && bufferdest)
	{
		m_context->CopyResource(bufferdest, buffersrc);
	}
	else
	{
		tswarn("unable to copy buffers");
	}
}

void D3D11RenderContext::textureUpdate(HTexture rsc, uint32 index, const void* memory)
{
	auto texture = reinterpret_cast<ID3D11Resource*>(rsc);

	if (texture)
	{
		m_context->UpdateSubresource(texture, index, nullptr, memory, 0, 0);
	}
	else
	{
		tswarn("unable to update texture");
	}
}

void D3D11RenderContext::textureCopy(HTexture src, HTexture dest)
{
	auto texsrc = reinterpret_cast<ID3D11Resource*>(src);
	auto texdest = reinterpret_cast<ID3D11Resource*>(dest);

	if (texsrc && texdest)
	{
		m_context->CopyResource(texdest, texsrc);
	}
	else
	{
		tswarn("unable to copy textures");
	}
}

void D3D11RenderContext::textureResolve(HTexture src, HTexture dest)
{
	auto texsrc = reinterpret_cast<ID3D11Resource*>(src);
	auto texdest = reinterpret_cast<ID3D11Resource*>(dest);

	if (texsrc && texdest)
	{
		m_context->ResolveSubresource(texdest, 0, texsrc, 0, DXGI_FORMAT_UNKNOWN);
	}
	else
	{
		tswarn("unable to resolve textures");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
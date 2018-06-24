/*
	Render API

	D3D11 implementation of rendering context
*/

#include "Render.h"
#include "Context.h"
#include "Helpers.h"
#include "HandleResource.h"
#include "HandleTarget.h"

using namespace std;
using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  Constructor
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

D3D11Context::D3D11Context(D3D11* driver) :
	m_driver(driver)
{
	tsassert(m_driver);
	tsassert(SUCCEEDED(m_driver->getDevice()->CreateDeferredContext(0, m_context.GetAddressOf())));
}

D3D11Context::~D3D11Context()
{

}

void D3D11Context::finish()
{
	m_contextCommandList.Reset();

	//Store current state in a command list
	m_context->FinishCommandList(false, m_contextCommandList.GetAddressOf());
}

void D3D11Context::resetCommandList()
{
	m_contextCommandList.Reset();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Resource updating
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D3D11Context::clearColourTarget(TargetHandle h, uint32 colour)
{
	if (D3D11Target* target = D3D11Target::upcast(h))
	{
		target->clearRenderTargets(m_context.Get(), RGBA(colour));
	}
}

void D3D11Context::clearDepthTarget(TargetHandle h, float depth)
{
	if (D3D11Target* target = D3D11Target::upcast(h))
	{
		target->clearDepthStencil(m_context.Get(), depth);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D3D11Context::resourceUpdate(ResourceHandle rsc, const void* memory, uint32 index)
{
	D3D11Resource* pRsc = D3D11Resource::upcast(rsc);

	if (pRsc)
	{
		m_context->UpdateSubresource(pRsc->asResource(), index, nullptr, memory, 0, 0);
	}
	else
	{
		tswarn("unable to update buffer");
	}
}

void D3D11Context::resourceCopy(ResourceHandle src, ResourceHandle dest)
{
	auto pSrc = D3D11Resource::upcast(src);
	auto pDest = D3D11Resource::upcast(dest);

	if (pSrc && pDest)
	{
		m_context->CopyResource(pDest->asResource(), pSrc->asResource());
	}
	else
	{
		tswarn("unable to copy buffers");
	}
}

void D3D11Context::imageResolve(ResourceHandle src, ResourceHandle dest, uint32 index)
{
	auto pSrc = D3D11Resource::upcast(src);
	auto pDest = D3D11Resource::upcast(dest);

	if (pSrc && pDest && pSrc->isImage() && pDest->isImage())
	{
		m_context->ResolveSubresource(pDest->asResource(), index, pSrc->asResource(), index, DXGI_FORMAT_UNKNOWN);
	}
	else
	{
		tswarn("unable to resolve textures");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
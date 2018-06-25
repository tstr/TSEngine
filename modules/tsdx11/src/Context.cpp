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

Dx11Context::Dx11Context(Dx11* driver) :
	m_driver(driver)
{
	tsassert(m_driver);
	tsassert(SUCCEEDED(m_driver->getDevice()->CreateDeferredContext(0, m_context.GetAddressOf())));
}

Dx11Context::~Dx11Context()
{

}

void Dx11Context::finish()
{
	m_contextCommandList.Reset();

	//Store current state in a command list
	m_context->FinishCommandList(false, m_contextCommandList.GetAddressOf());
}

void Dx11Context::resetCommandList()
{
	m_contextCommandList.Reset();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Resource updating
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Dx11Context::clearColourTarget(TargetHandle h, uint32 colour)
{
	if (DxTarget* target = DxTarget::upcast(h))
	{
		target->clearRenderTargets(m_context.Get(), RGBA(colour));
	}
}

void Dx11Context::clearDepthTarget(TargetHandle h, float depth)
{
	if (DxTarget* target = DxTarget::upcast(h))
	{
		target->clearDepthStencil(m_context.Get(), depth);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Dx11Context::resourceUpdate(ResourceHandle rsc, const void* memory, uint32 index)
{
	DxResource* pRsc = DxResource::upcast(rsc);

	if (pRsc)
	{
		m_context->UpdateSubresource(pRsc->asResource(), index, nullptr, memory, 0, 0);
	}
	else
	{
		tswarn("unable to update buffer");
	}
}

void Dx11Context::resourceCopy(ResourceHandle src, ResourceHandle dest)
{
	auto pSrc = DxResource::upcast(src);
	auto pDest = DxResource::upcast(dest);

	if (pSrc && pDest)
	{
		m_context->CopyResource(pDest->asResource(), pSrc->asResource());
	}
	else
	{
		tswarn("unable to copy buffers");
	}
}

void Dx11Context::imageResolve(ResourceHandle src, ResourceHandle dest, uint32 index)
{
	auto pSrc = DxResource::upcast(src);
	auto pDest = DxResource::upcast(dest);

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
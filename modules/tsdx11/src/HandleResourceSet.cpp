/*
	Render API

	Resource Set object
*/

#include "Render.h"
#include "HandleResourceSet.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

RPtr<ResourceSetHandle> Dx11::createResourceSet(const ResourceSetCreateInfo& info, ResourceSetHandle recycle)
{
	UPtr<DxResourceSet> newSet(new DxResourceSet());
	DxResourceSet* set = (recycle == (ResourceSetHandle)0) ? newSet.release() : DxResourceSet::upcast(recycle);
	set->reset();

	HRESULT hr = set->create(info);

	if (FAILED(hr))
	{
		delete set;
		return RPtr<ResourceSetHandle>();
	}

	return RPtr<ResourceSetHandle>(this, DxResourceSet::downcast(set));
}

void Dx11::destroy(ResourceSetHandle handle)
{
	if (auto t = DxResourceSet::upcast(handle))
	{
		delete t;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

HRESULT DxResourceSet::create(const ResourceSetCreateInfo& info)
{
	reset();

	//Get constant buffers
	m_constantBuffers.reserve(info.constantBuffersCount);
	for (size_t i = 0; i < info.constantBuffersCount; i++)
	{
		m_constantBuffers.push_back(CBV(DxResource::upcast(info.constantBuffers[i])));
	}
	
	//Get vertex buffers
	m_vertexBuffers.reserve(info.vertexBufferCount);
	for (size_t i = 0; i < info.vertexBufferCount; i++)
	{
		m_vertexBuffers.push_back(VBV(info.vertexBuffers[i]));
	}

	//Get resource views
	m_srvs.reserve(info.resourceCount);
	for (size_t i = 0; i < info.resourceCount; i++)
	{
		m_srvs.push_back(SRV(info.resources[i]));
	}

	m_indexBuffer = DxResource::upcast(info.indexBuffer);

	return S_OK;
}

void DxResourceSet::bind(ID3D11DeviceContext* context)
{
	//Bind SRV
	UINT i = 0;
	for (const SRV& srv : m_srvs)
	{
		auto s = srv.getView();
		context->PSSetShaderResources(i, 1, &s);
		context->HSSetShaderResources(i, 1, &s);
		i++;
	}
	
	//Bind constant buffers
	i = 0;
	for (const CBV& cbv : m_constantBuffers)
	{
		ID3D11Buffer* buf = (cbv == nullptr) ? nullptr : cbv->asBuffer();
		context->VSSetConstantBuffers(i, 1, &buf);
		context->GSSetConstantBuffers(i, 1, &buf);
		context->DSSetConstantBuffers(i, 1, &buf);
		context->HSSetConstantBuffers(i, 1, &buf);
		context->PSSetConstantBuffers(i, 1, &buf);
		i++;
	}

	//Bind vertex buffers
	i = 0;
	for (const VBV& vbv : m_vertexBuffers)
	{
		auto v = vbv.getBuffer();
		context->IASetVertexBuffers(i, 1, &v, &vbv.stride, &vbv.offset);
		i++;
	}

	//Bind index buffer
	auto ib = (m_indexBuffer == nullptr) ? nullptr : m_indexBuffer->asBuffer();
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

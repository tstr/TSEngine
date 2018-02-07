/*
	Render API

	D3D11Render buffer resource handling methods
*/

#include "render.h"
#include "helpers.h"

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ERenderStatus D3D11Render::createResourceBuffer(
	HBuffer& rsc,
	const SBufferResourceData& data
)
{
	ID3D11Buffer* buffer = nullptr;

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
		case EBufferType::eBufferTypeConstant: { subdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; break; }
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

	HRESULT hr = m_device->CreateBuffer(&subdesc, &subdata, &buffer);

	if (FAILED(hr))
	{
		return RenderStatusFromHRESULT(hr);
	}

	rsc = (HBuffer)reinterpret_cast<HBuffer&>(buffer);
	
	return eOk;
}

void D3D11Render::destroyBuffer(HBuffer buffer)
{
	if (auto u = reinterpret_cast<IUnknown*>(buffer))
	{
		u->Release();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

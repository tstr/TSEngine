/*
	Render API

	Shader handling methods
*/

#include "render.h"
#include "helpers.h"
#include "handleshader.h"

#include <vector>

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

RPtr<ShaderHandle> D3D11::createShader(const ShaderCreateInfo& info)
{
	UPtr<D3D11Shader> program;

	//Has stage
	auto has = [&info](ShaderStage stage)
	{
		return info.stages[(size_t)stage].bytecode != nullptr;
	};

	if (has(ShaderStage::VERTEX))
	{
		const ShaderBytecode& bc = info.stages[(size_t)ShaderStage::VERTEX];
		HRESULT hr = m_device->CreateVertexShader(bc.bytecode, bc.size, nullptr, program->vertex.GetAddressOf());
		if (FAILED(hr)) return RPtr<ShaderHandle>();

		//Save vertex bytecode for use when building input layouts
		program->vertexBytecode = MemoryBuffer(bc.bytecode, bc.size);
	}

	if (has(ShaderStage::GEOMETRY))
	{
		const ShaderBytecode& bc = info.stages[(size_t)ShaderStage::GEOMETRY];
		HRESULT hr = m_device->CreateGeometryShader(bc.bytecode, bc.size, nullptr, program->geometry.GetAddressOf());
		if (FAILED(hr)) return RPtr<ShaderHandle>();
	}

	if (has(ShaderStage::TESSCTRL))
	{
		const ShaderBytecode& bc = info.stages[(size_t)ShaderStage::TESSCTRL];
		HRESULT hr = m_device->CreateDomainShader(bc.bytecode, bc.size, nullptr, program->domain.GetAddressOf());
		if (FAILED(hr)) return RPtr<ShaderHandle>();
	}

	if (has(ShaderStage::TESSEVAL))
	{
		const ShaderBytecode& bc = info.stages[(size_t)ShaderStage::TESSEVAL];
		HRESULT hr = m_device->CreateHullShader(bc.bytecode, bc.size, nullptr, program->hull.GetAddressOf());
		if (FAILED(hr)) return RPtr<ShaderHandle>();
	}

	if (has(ShaderStage::PIXEL))
	{
		const ShaderBytecode& bc = info.stages[(size_t)ShaderStage::PIXEL];
		HRESULT hr = m_device->CreatePixelShader(bc.bytecode, bc.size, nullptr, program->pixel.GetAddressOf());
		if (FAILED(hr)) return RPtr<ShaderHandle>();
	}

	if (has(ShaderStage::COMPUTE))
	{
		const ShaderBytecode& bc = info.stages[(size_t)ShaderStage::COMPUTE];
		HRESULT hr = m_device->CreateComputeShader(bc.bytecode, bc.size, nullptr, program->compute.GetAddressOf());
		if (FAILED(hr)) return RPtr<ShaderHandle>();
	}

	return RPtr<ShaderHandle>(this, D3D11Shader::downcast(program.release()));
}

void D3D11::destroy(ShaderHandle shader)
{
	if (auto s = D3D11Shader::upcast(shader))
	{
		delete s;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void D3D11Shader::bind(ID3D11DeviceContext* context)
{
	context->VSSetShader(vertex.Get(), nullptr, 0);
	context->PSSetShader(pixel.Get(), nullptr, 0);
	context->GSSetShader(geometry.Get(), nullptr, 0);
	context->DSSetShader(domain.Get(), nullptr, 0);
	context->HSSetShader(hull.Get(), nullptr, 0);
}

ComPtr<ID3D11InputLayout> D3D11Shader::createInputLayout(const VertexAttribute* attributeList, size_t attributeCount)
{
	if (vertex.Get() == nullptr || vertexBytecode.pointer() == nullptr)
	{
		return nullptr;
	}

	std::vector<D3D11_INPUT_ELEMENT_DESC> elementList;
	elementList.reserve(attributeCount);

	for (size_t i = 0; i < attributeCount; i++)
	{
		D3D11_INPUT_ELEMENT_DESC element;

		const VertexAttribute& attrib = attributeList[i];

		element.AlignedByteOffset = attrib.byteOffset;
		element.InputSlot = attrib.bufferSlot;
		element.SemanticName = attrib.semanticName;
		element.SemanticIndex = 0;
		element.InstanceDataStepRate = 0;

		if (attrib.channel == VertexAttributeChannel::VERTEX)
		{
			element.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		}
		else if (attrib.channel == VertexAttributeChannel::INSTANCE)
		{
			element.InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_INSTANCE_DATA;
			element.InstanceDataStepRate = 1;
		}

		//matrix attribute requires multiple input components
		if (attrib.type == VertexAttributeType::MATRIX)
		{
			for (int j = 0; j < 4; j++)
			{
				element.AlignedByteOffset = attrib.byteOffset + (j * sizeof(Vector));
				element.SemanticIndex = j;

				elementList.push_back(element);
			}
		}
		else
		{
			switch (attrib.type)
			{
				case VertexAttributeType::FLOAT: { element.Format = DXGI_FORMAT_R32_FLOAT; break; }
				case VertexAttributeType::FLOAT2: { element.Format = DXGI_FORMAT_R32G32_FLOAT; break; }
				case VertexAttributeType::FLOAT3: { element.Format = DXGI_FORMAT_R32G32B32_FLOAT; break; }
				case VertexAttributeType::FLOAT4: { element.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break; }
				case VertexAttributeType::INT32: { element.Format = DXGI_FORMAT_R32_SINT; break; }
				case VertexAttributeType::UINT32: { element.Format = DXGI_FORMAT_R32_UINT; break; }
				case VertexAttributeType::RGB: {}
				case VertexAttributeType::RGBA: { element.Format = DXGI_FORMAT_R8G8B8A8_UNORM; break; }
				default: { element.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN; }
			}

			elementList.push_back(element);
		}
	}

	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3D11Device> device;
	vertex->GetDevice(device.GetAddressOf());

	HRESULT hr = device->CreateInputLayout(
		elementList.data(),
		(UINT)elementList.size(),
		vertexBytecode.pointer(),
		vertexBytecode.size(),
		inputLayout.GetAddressOf()
	);

	if (FAILED(hr))
	{
		return nullptr;
	}

	return inputLayout;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Render API

	Shader handling methods
*/

#include "render.h"
#include "helpers.h"
#include "handleshader.h"

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ShaderHandle D3D11::createShader(const ShaderCreateInfo& info)
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
		if (FAILED(hr)) return D3D11Shader::downcast(nullptr);
	}

	if (has(ShaderStage::GEOMETRY))
	{
		const ShaderBytecode& bc = info.stages[(size_t)ShaderStage::GEOMETRY];
		HRESULT hr = m_device->CreateGeometryShader(bc.bytecode, bc.size, nullptr, program->geometry.GetAddressOf());
		if (FAILED(hr)) return D3D11Shader::downcast(nullptr);
	}

	if (has(ShaderStage::TESSCTRL))
	{
		const ShaderBytecode& bc = info.stages[(size_t)ShaderStage::TESSCTRL];
		HRESULT hr = m_device->CreateDomainShader(bc.bytecode, bc.size, nullptr, program->domain.GetAddressOf());
		if (FAILED(hr)) return D3D11Shader::downcast(nullptr);
	}

	if (has(ShaderStage::TESSEVAL))
	{
		const ShaderBytecode& bc = info.stages[(size_t)ShaderStage::TESSEVAL];
		HRESULT hr = m_device->CreateHullShader(bc.bytecode, bc.size, nullptr, program->hull.GetAddressOf());
		if (FAILED(hr)) return D3D11Shader::downcast(nullptr);
	}

	if (has(ShaderStage::PIXEL))
	{
		const ShaderBytecode& bc = info.stages[(size_t)ShaderStage::PIXEL];
		HRESULT hr = m_device->CreatePixelShader(bc.bytecode, bc.size, nullptr, program->pixel.GetAddressOf());
		if (FAILED(hr)) return D3D11Shader::downcast(nullptr);
	}

	if (has(ShaderStage::COMPUTE))
	{
		const ShaderBytecode& bc = info.stages[(size_t)ShaderStage::COMPUTE];
		HRESULT hr = m_device->CreateComputeShader(bc.bytecode, bc.size, nullptr, program->compute.GetAddressOf());
		if (FAILED(hr)) return D3D11Shader::downcast(nullptr);
	}
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

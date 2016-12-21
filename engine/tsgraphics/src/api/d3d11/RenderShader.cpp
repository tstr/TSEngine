/*
	Render API

	D3D11Render shader handling methods
*/

#include "render.h"
#include "helpers.h"
#include "shader.h"

using namespace ts;

inline HShader downcast(D3D11Shader* shader)
{
	return (HShader)reinterpret_cast<HShader&>(shader);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ERenderStatus D3D11Render::createShader(
	HShader& shader,
	const void* bytecode,
	uint32 bytecodesize,
	EShaderStage stage
)
{
	switch (stage)
	{
		case (EShaderStage::eShaderStageVertex):
		{
			ComPtr<ID3D11VertexShader> vertexshader;
			HRESULT hr = m_device->CreateVertexShader(bytecode, bytecodesize, nullptr, vertexshader.GetAddressOf());
			if (FAILED(hr)) return RenderStatusFromHRESULT(hr);
		
			shader = downcast(new D3D11Shader(vertexshader.Get(), MemoryBuffer(bytecode, bytecodesize)));
			return eOk;
		}

		case (EShaderStage::eShaderStagePixel):
		{
			ComPtr<ID3D11PixelShader> pixelshader;
			HRESULT hr = m_device->CreatePixelShader(bytecode, bytecodesize, nullptr, pixelshader.GetAddressOf());

			if (FAILED(hr)) return RenderStatusFromHRESULT(hr);

			shader = downcast(new D3D11Shader(pixelshader.Get(), MemoryBuffer(bytecode, bytecodesize)));
			return eOk;
		}

		case (EShaderStage::eShaderStageGeometry):
		{
			ComPtr<ID3D11GeometryShader> geometryshader;
			HRESULT hr = m_device->CreateGeometryShader(bytecode, bytecodesize, nullptr, geometryshader.GetAddressOf());

			if (FAILED(hr)) return RenderStatusFromHRESULT(hr);

			shader = downcast(new D3D11Shader(geometryshader.Get(), MemoryBuffer(bytecode, bytecodesize)));
			return eOk;
		}

		case (EShaderStage::eShaderStageHull):
		{
			ComPtr<ID3D11HullShader> hullshader;
			HRESULT hr = m_device->CreateHullShader(bytecode, bytecodesize, nullptr, hullshader.GetAddressOf());
			if (FAILED(hr)) return RenderStatusFromHRESULT(hr);

			shader = downcast(new D3D11Shader(hullshader.Get(), MemoryBuffer(bytecode, bytecodesize)));
			return eOk;
		}

		case (EShaderStage::eShaderStageDomain):
		{
			ComPtr<ID3D11DomainShader> domainshader;
			HRESULT hr = m_device->CreateDomainShader(bytecode, bytecodesize, nullptr, domainshader.GetAddressOf());

			if (FAILED(hr)) return RenderStatusFromHRESULT(hr);

			shader = downcast(new D3D11Shader(domainshader.Get(), MemoryBuffer(bytecode, bytecodesize)));
			return eOk;
		}

		case (EShaderStage::eShaderStageCompute):
		{
			ComPtr<ID3D11ComputeShader> computeshader;
			HRESULT hr = m_device->CreateComputeShader(bytecode, bytecodesize, nullptr, computeshader.GetAddressOf());

			if (FAILED(hr)) return RenderStatusFromHRESULT(hr);

			shader = downcast(new D3D11Shader(computeshader.Get(), MemoryBuffer(bytecode, bytecodesize)));
			return eOk;
		}

		default:
			return eInvalidParameter;
	}
}

void D3D11Render::destroyShader(HShader shader)
{
	if (auto s = reinterpret_cast<D3D11Shader*>(shader))
	{
		delete s;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

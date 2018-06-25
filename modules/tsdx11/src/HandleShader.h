/*
	Render API

	Shader program class
*/

#pragma once

#include "render.h"
#include "handle.h"

namespace ts
{
	struct DxShader : public Handle<DxShader, ShaderHandle>
	{
		//Shader stages
		ComPtr<ID3D11VertexShader> vertex;
		ComPtr<ID3D11GeometryShader> geometry;
		ComPtr<ID3D11DomainShader> domain;
		ComPtr<ID3D11HullShader> hull;
		ComPtr<ID3D11PixelShader> pixel;
		ComPtr<ID3D11ComputeShader> compute;

		MemoryBuffer vertexBytecode;

		//Bind shader program stages
		void bind(ID3D11DeviceContext* context);

		//Create an input layout for this particular shader program
		ComPtr<ID3D11InputLayout> createInputLayout(const VertexAttribute* attributeList, size_t attributeCount);
	};
}

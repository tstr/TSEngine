/*
	Render API

	Pipeline object
*/

#pragma once

#include "Base.h"
#include "Handle.h"
#include "HandleShader.h"
#include "StateManager.h"

#include <vector>

namespace ts
{
	class D3D11Pipeline : public Handle<D3D11Pipeline, PipelineHandle>
	{
	public:

		D3D11Pipeline(D3D11StateManager& states, ShaderHandle program, const PipelineCreateInfo& info);
		~D3D11Pipeline() {}
		
		void bind(ID3D11DeviceContext* context);

	private:

		D3D11Shader* m_program;

		ComPtr<ID3D11RasterizerState> m_rasterState;
		ComPtr<ID3D11DepthStencilState> m_depthState;
		ComPtr<ID3D11BlendState> m_blendState;
		std::vector<ComPtr<ID3D11SamplerState>> m_samplers;

		ComPtr<ID3D11InputLayout> m_inputLayout;

		D3D11_PRIMITIVE_TOPOLOGY m_topology;

	};
}
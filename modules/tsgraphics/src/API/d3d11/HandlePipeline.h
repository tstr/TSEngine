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
	class DxPipeline : public Handle<DxPipeline, PipelineHandle>
	{
	public:

		DxPipeline(DxStateManager& states, ShaderHandle program, const PipelineCreateInfo& info);
		~DxPipeline() {}
		
		void bind(ID3D11DeviceContext* context);

	private:

		DxShader* m_program;

		ComPtr<ID3D11RasterizerState> m_rasterState;
		ComPtr<ID3D11DepthStencilState> m_depthState;
		ComPtr<ID3D11BlendState> m_blendState;
		std::vector<ComPtr<ID3D11SamplerState>> m_samplers;

		ComPtr<ID3D11InputLayout> m_inputLayout;

		D3D11_PRIMITIVE_TOPOLOGY m_topology;

	};
}
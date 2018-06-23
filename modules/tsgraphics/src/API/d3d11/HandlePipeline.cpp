/*
	Render API

	Pipeline object
*/

#include "Render.h"
#include "HandlePipeline.h"

using namespace ts;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

RPtr<PipelineHandle> D3D11::createPipeline(ShaderHandle program, const PipelineCreateInfo& info)
{
	return RPtr<PipelineHandle>(this, D3D11Pipeline::downcast(new D3D11Pipeline(m_stateManager, program, info)));
}

void D3D11::destroy(PipelineHandle hpipe)
{
	if (auto p = D3D11Pipeline::upcast(hpipe))
	{
		delete p;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

D3D11Pipeline::D3D11Pipeline(D3D11StateManager& states, ShaderHandle program, const PipelineCreateInfo& info)
{
	ID3D11Device* device = states.getDevice();

	m_program = D3D11Shader::upcast(program);
	tsassert(m_program != nullptr);

	//Resolve states
	states.demandBlendState(info.blend, m_blendState.GetAddressOf());
	states.demandDepthState(info.depth, m_depthState.GetAddressOf());
	states.demandRasterizerState(info.raster, m_rasterState.GetAddressOf());

	//Resolve sampler states
	m_samplers.reserve(info.samplerCount);

	for (size_t i = 0; i < info.samplerCount; i++)
	{
		ComPtr<ID3D11SamplerState> sampler;
		states.demandSamplerState(info.samplers[i], sampler.GetAddressOf());
		m_samplers.push_back(sampler);
	}

	//Get primitive topology
	switch (info.topology)
	{
		case (VertexTopology::TRIANGELIST): { m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break; }
		case (VertexTopology::TRIANGESTRIP): { m_topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break; }
		case (VertexTopology::LINELIST): { m_topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST; break; }
		case (VertexTopology::LINESTRIP): { m_topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP; break; }
		case (VertexTopology::POINTLIST): { m_topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST; break; }
		case (VertexTopology::PATCHLIST2): { m_topology = D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST; break; }
		case (VertexTopology::PATCHLIST3): { m_topology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST; break; }
		case (VertexTopology::PATCHLIST4): { m_topology = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST; break; }
		default: { m_topology = D3D11_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_UNDEFINED; }
	}

	//Create input layout
	m_inputLayout = m_program->createInputLayout(info.vertexAttributeList, info.vertexAttributeCount);

}

void D3D11Pipeline::bind(ID3D11DeviceContext* context)
{
	//Bind shader
	m_program->bind(context);

	//Bind states
	context->RSSetState(m_rasterState.Get());
	const float blendfactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->OMSetBlendState(m_blendState.Get(), blendfactor, UINT_MAX);
	context->OMSetDepthStencilState(m_depthState.Get(), 0);
	
	//Bind samplers
	context->PSSetSamplers(0, m_samplers.size(), (ID3D11SamplerState**)m_samplers.data());

	//Bind vertex input description
	context->IASetInputLayout(m_inputLayout.Get());
	context->IASetPrimitiveTopology(m_topology);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

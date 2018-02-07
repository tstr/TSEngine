/*
	Render API

	D3D11Render draw command methods
*/

#include "Render.h"
#include "Helpers.h"
#include "HandleCommandDraw.h"
#include "HandleShader.h"
#include "HandleTexture.h"

#include <vector>

using namespace ts;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int createInputLayout(ID3D11Device* device, D3D11Shader* vertexShader, const SVertexAttribute* attrib, uint32 attribCount, ID3D11InputLayout** inputlayout);

//Gets a shader interface from a handle only if the handle is valid
template<typename ishader_t>
inline ishader_t* getID3DShader(HShader handle)
{
	if (auto s = D3D11Shader::upcast(handle))
	{
		return (ishader_t*)s->getShader();
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ERenderStatus D3D11Render::createDrawCommand(HDrawCmd& hDraw, const SDrawCommand& cmdDesc)
{
	D3D11DrawCommand* d3dCmd = new D3D11DrawCommand();
	
	/////////////////////////////////////////////////////////////////////////////////////////
	//Set all shaders

	d3dCmd->shaderVertex   = getID3DShader<ID3D11VertexShader>(cmdDesc.shaderVertex);
	d3dCmd->shaderPixel    = getID3DShader<ID3D11PixelShader>(cmdDesc.shaderPixel);
	d3dCmd->shaderGeometry = getID3DShader<ID3D11GeometryShader>(cmdDesc.shaderGeometry);
	d3dCmd->shaderHull     = getID3DShader<ID3D11HullShader>(cmdDesc.shaderHull);
	d3dCmd->shaderDomain   = getID3DShader<ID3D11DomainShader>(cmdDesc.shaderDomain);

	//Create the input layout if vertex attributes are present
	if (cmdDesc.vertexAttribCount)
	{
		tsassert(!createInputLayout(
			m_device.Get(),
			D3D11Shader::upcast(cmdDesc.shaderVertex),
			cmdDesc.vertexAttribs,
			cmdDesc.vertexAttribCount,
			d3dCmd->inputLayout.GetAddressOf()
		));
	}
	
	/////////////////////////////////////////////////////////////////////////////////////////
	//Render states
	
	ComPtr<ID3D11DepthStencilState> depthState;
	ComPtr<ID3D11BlendState> blendState;
	ComPtr<ID3D11RasterizerState> rasterState;

	m_stateManager.demandDepthState(cmdDesc.depthState, depthState.GetAddressOf());
	m_stateManager.demandBlendState(cmdDesc.blendState, blendState.GetAddressOf());
	m_stateManager.demandRasterizerState(cmdDesc.rasterState, rasterState.GetAddressOf());

	d3dCmd->depthState = depthState.Get();
	d3dCmd->blendState = blendState.Get();
	d3dCmd->rasterState = rasterState.Get();

	/////////////////////////////////////////////////////////////////////////////////////////
	//Set all buffers

	//Index buffer
	d3dCmd->indexBuffer = reinterpret_cast<ID3D11Buffer*>(cmdDesc.indexBuffer);

	//Constant buffers
	for (uint32 i = 0; i < cmdDesc.eMaxConstantBuffers; i++)
	{
		if (HBuffer h = cmdDesc.constantBuffers[i])
		{
			d3dCmd->constantBuffers[i] = reinterpret_cast<ID3D11Buffer*>(h);
		}
	}

	//Vertex buffers
	for (uint32 i = 0; i < cmdDesc.eMaxVertexBuffers; i++)
	{
		if (HBuffer h = cmdDesc.vertexBuffers[i])
		{
			d3dCmd->vertexBuffers[i] = reinterpret_cast<ID3D11Buffer*>(h);
			d3dCmd->vertexStrides[i] = cmdDesc.vertexStrides[i];
			d3dCmd->vertexOffsets[i] = cmdDesc.vertexOffsets[i];
		}
	}

	//Set primitive topology
	switch (cmdDesc.vertexTopology)
	{
		case (EVertexTopology::eTopologyTriangleList): { d3dCmd->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break; }
		case (EVertexTopology::eTopologyTriangleStrip): { d3dCmd->topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break; }
		case (EVertexTopology::eTopologyLineList): { d3dCmd->topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST; break; }
		case (EVertexTopology::eTopologyLineStrip): { d3dCmd->topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP; break; }
		case (EVertexTopology::eTopologyPointList): { d3dCmd->topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST; break; }
		case (EVertexTopology::eTopologyPatchList2): { d3dCmd->topology = D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST; break; }
		case (EVertexTopology::eTopologyPatchList3): { d3dCmd->topology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST; break; }
		case (EVertexTopology::eTopologyPatchList4): { d3dCmd->topology = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST; break; }
		default: { d3dCmd->topology = D3D11_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_UNDEFINED; }
	}
	
	/////////////////////////////////////////////////////////////////////////////////////////

	//Set an SRV for every active texture unit
	for (uint32 i = 0; i < cmdDesc.eMaxTextureSlots; i++)
	{
		const STextureUnit& unit = cmdDesc.textureUnits[i];

		if (D3D11Texture* tex = D3D11Texture::upcast(unit.texture))
		{
			d3dCmd->shaderResourceViews[i] = tex->getView(unit.arrayIndex, unit.arrayCount, unit.textureType).Get();
		}
	}

	//Set texture samplers
	for (uint32 i = 0; i < cmdDesc.eMaxTextureSamplerSlots; i++)
	{
		if (cmdDesc.textureSamplers[i].enabled)
		{
			ComPtr<ID3D11SamplerState> sampler;

			m_stateManager.demandSamplerState(
				cmdDesc.textureSamplers[i],
				sampler.GetAddressOf()
			);

			d3dCmd->shaderSamplerStates[i] = sampler.Get();
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////

	d3dCmd->indexStart = cmdDesc.indexStart;
	d3dCmd->indexCount = cmdDesc.indexCount;
	d3dCmd->vertexStart = cmdDesc.vertexStart;
	d3dCmd->vertexCount = cmdDesc.vertexCount;
	d3dCmd->vertexBase = cmdDesc.vertexBase;
	d3dCmd->instanceCount = cmdDesc.instanceCount;
	
	d3dCmd->mode = cmdDesc.mode;

	/////////////////////////////////////////////////////////////////////////////////////////
	
	hDraw = D3D11DrawCommand::downcast(d3dCmd);

	return eOk;
}

void D3D11Render::destroyDrawCommand(HDrawCmd hDraw)
{
	if (auto d = D3D11DrawCommand::upcast(hDraw))
	{
		delete d;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int createInputLayout(ID3D11Device* device, D3D11Shader* vertexShader, const SVertexAttribute* attrib, uint32 attribCount, ID3D11InputLayout** inputlayout)
{
	void* bytecode = nullptr;
	uint32 bytecodeSize = 0;
	vertexShader->getShaderBytecode(&bytecode, bytecodeSize);

	D3D11_INPUT_ELEMENT_DESC elements[SDrawCommand::eMaxVertexAttributes];

	for (uint32 i = 0; i < attribCount; i++)
	{
		elements[i].AlignedByteOffset = attrib[i].byteOffset;
		elements[i].InputSlot = attrib[i].bufferSlot;
		elements[i].SemanticName = attrib[i].semanticName.str();

		elements[i].SemanticIndex = 0;
		elements[i].InstanceDataStepRate = 0;

		if (attrib[i].channel == EVertexAttributeChannel::eChannelPerVertex)
		{
			elements[i].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		}
		else if (attrib[i].channel == EVertexAttributeChannel::eChannelPerInstance)
		{
			elements[i].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_INSTANCE_DATA;
			elements[i].InstanceDataStepRate = 1;
		}

		//todo: matrix
		switch (attrib[i].type)
		{
			case eAttribFloat: { elements[i].Format = DXGI_FORMAT_R32_FLOAT; break; }
			case eAttribFloat2: { elements[i].Format = DXGI_FORMAT_R32G32_FLOAT; break; }
			case eAttribFloat3: { elements[i].Format = DXGI_FORMAT_R32G32B32_FLOAT; break; }
			case eAttribFloat4: { elements[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break; }
			case eAttribInt32: { elements[i].Format = DXGI_FORMAT_R32_SINT; break; }
			case eAttribUint32: { elements[i].Format = DXGI_FORMAT_R32_UINT; break; }
			case eAttribRGB: {}
			case eAttribRGBA: { elements[i].Format = DXGI_FORMAT_R8G8B8A8_UNORM; break; }
			case eAttribMatrix:
			{
				for (int j = 0; j < 4; j++)
				{
					elements[j + i].AlignedByteOffset = attrib[i].byteOffset + (j * sizeof(Vector));
					elements[j + i].InputSlot = attrib[i].bufferSlot;
					elements[j + i].SemanticName = attrib[i].semanticName.str();

					elements[j + i].SemanticIndex = j;
					elements[j + i].InstanceDataStepRate = 0;

					if (attrib[i].channel == EVertexAttributeChannel::eChannelPerVertex)
					{
						elements[j + i].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
					}
					else if (attrib[i].channel == EVertexAttributeChannel::eChannelPerInstance)
					{
						elements[j + i].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_INSTANCE_DATA;
						elements[j + i].InstanceDataStepRate = 1;
					}

					elements[j + i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				}

				i += 3;
				attribCount += 3;

				break;
			}
			default: { elements[i].Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN; }
		}

	}

	return device->CreateInputLayout(&elements[0], attribCount, bytecode, bytecodeSize, inputlayout);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

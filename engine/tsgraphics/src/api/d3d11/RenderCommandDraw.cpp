/*
	Render API

	D3D11Render draw command methods
*/

#include "Render.h"
#include "Helpers.h"
#include "CommandDraw.h"
#include "Shader.h"

#include <vector>

using namespace ts;

//Creates a shader resource view from a texture unit description
int createTextureUnit(ID3D11Device* device, ID3D11ShaderResourceView** srv, const STextureUnit& unit);
int createInputLayout(ID3D11Device* device, D3D11Shader* vertexShader, const SVertexAttribute* attrib, uint32 attribCount, ID3D11InputLayout** inputlayout);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ERenderStatus D3D11Render::createDrawCommand(HDrawCmd& hDraw, const SDrawCommand& cmdDesc)
{
	D3D11DrawCommand* d3dCmd = new D3D11DrawCommand();

	/////////////////////////////////////////////////////////////////////////////////////////
	//Set all shaders

	d3dCmd->shaderVertex = (ID3D11VertexShader*)reinterpret_cast<D3D11Shader*>(cmdDesc.shaderVertex)->getShader();
	d3dCmd->shaderPixel = (ID3D11PixelShader*)reinterpret_cast<D3D11Shader*>(cmdDesc.shaderPixel)->getShader();
	d3dCmd->shaderGeometry = (ID3D11GeometryShader*)reinterpret_cast<D3D11Shader*>(cmdDesc.shaderGeometry)->getShader();
	d3dCmd->shaderHull = (ID3D11HullShader*)reinterpret_cast<D3D11Shader*>(cmdDesc.shaderHull)->getShader();
	d3dCmd->shaderDomain = (ID3D11DomainShader*)reinterpret_cast<D3D11Shader*>(cmdDesc.shaderDomain)->getShader();

	//Create the input layout if a vertex shader is present
	if (d3dCmd->shaderVertex)
	{
		tsassert(!createInputLayout(
			m_device.Get(),
			reinterpret_cast<D3D11Shader*>(cmdDesc.shaderVertex),
			cmdDesc.vertexAttribs,
			cmdDesc.vertexAttribCount,
			d3dCmd->inputLayout.GetAddressOf()
		));
	}

	/////////////////////////////////////////////////////////////////////////////////////////
	//Render states

	d3dCmd->depthState = this->getDepthStencilState();
	d3dCmd->rasterState = this->getRasterizerState();
	d3dCmd->blendState = this->getBlendState();
	
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
		default: { d3dCmd->topology = D3D11_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_UNDEFINED; }
	}

	/////////////////////////////////////////////////////////////////////////////////////////

	//Iterate over each texture unit description and create an SRV for each texture resource
	for (uint32 i = 0; i < cmdDesc.eMaxTextureSlots; i++)
	{
		const STextureUnit& unit = cmdDesc.textureUnits[i];

		if (unit.texture)
		{
			tsassert(!createTextureUnit(
				m_device.Get(),
				d3dCmd->shaderResourceViews[i].GetAddressOf(),
				unit
			));
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
	
	hDraw = (HDrawCmd)reinterpret_cast<HDrawCmd&>(d3dCmd);
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

int createTextureUnit(ID3D11Device* device, ID3D11ShaderResourceView** srv, const STextureUnit& unit)
{
	switch (unit.textureType)
	{
		case eTypeTexture1D:
		{
			D3D11_TEXTURE1D_DESC desc;
			if (auto tex = reinterpret_cast<ID3D11Texture1D*>(unit.texture))
			{
				tex->GetDesc(&desc);

				D3D11_SHADER_RESOURCE_VIEW_DESC viewdesc;
				ZeroMemory(&viewdesc, sizeof(viewdesc));
				viewdesc.Format = desc.Format;
				viewdesc.ViewDimension = (desc.ArraySize > 1) ? D3D11_SRV_DIMENSION_TEXTURE1DARRAY : D3D11_SRV_DIMENSION_TEXTURE1D;
				viewdesc.Texture1DArray.MipLevels = desc.MipLevels;
				viewdesc.Texture1DArray.ArraySize = unit.arrayCount;
				viewdesc.Texture1DArray.FirstArraySlice = unit.arrayIndex;
				viewdesc.Texture1DArray.MostDetailedMip = 0;

				return device->CreateShaderResourceView(tex, &viewdesc, srv);
			}

			break;
		}

		case eTypeTexture2D:
		{
			D3D11_TEXTURE2D_DESC desc;
			if (auto tex = reinterpret_cast<ID3D11Texture2D*>(unit.texture))
			{
				tex->GetDesc(&desc);

				D3D11_SHADER_RESOURCE_VIEW_DESC viewdesc;
				ZeroMemory(&viewdesc, sizeof(viewdesc));
				viewdesc.Format = desc.Format;
				viewdesc.ViewDimension = (desc.ArraySize > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DARRAY : D3D11_SRV_DIMENSION_TEXTURE2D;
				viewdesc.Texture2DArray.MipLevels = desc.MipLevels;
				viewdesc.Texture2DArray.ArraySize = unit.arrayCount;
				viewdesc.Texture2DArray.FirstArraySlice = unit.arrayIndex;
				viewdesc.Texture2DArray.MostDetailedMip = 0;

				return device->CreateShaderResourceView(tex, &viewdesc, srv);
			}

			break;
		}

		case eTypeTexture3D:
		{
			D3D11_TEXTURE3D_DESC desc;
			if (auto tex = reinterpret_cast<ID3D11Texture3D*>(unit.texture))
			{
				tex->GetDesc(&desc);

				D3D11_SHADER_RESOURCE_VIEW_DESC viewdesc;
				ZeroMemory(&viewdesc, sizeof(viewdesc));
				viewdesc.Format = desc.Format;
				viewdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
				viewdesc.Texture3D.MipLevels = desc.MipLevels;
				viewdesc.Texture3D.MostDetailedMip = 0;

				return device->CreateShaderResourceView(tex, &viewdesc, srv);
			}

			break;
		}

		case eTypeTextureCube:
		{
			D3D11_TEXTURE2D_DESC desc;
			if (auto tex = reinterpret_cast<ID3D11Texture2D*>(unit.texture))
			{
				tex->GetDesc(&desc);

				D3D11_SHADER_RESOURCE_VIEW_DESC viewdesc;
				ZeroMemory(&viewdesc, sizeof(viewdesc));
				viewdesc.Format = desc.Format;
				viewdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
				viewdesc.Texture2DArray.MipLevels = desc.MipLevels;
				viewdesc.Texture2DArray.ArraySize = unit.arrayCount;
				viewdesc.Texture2DArray.FirstArraySlice = unit.arrayIndex;
				viewdesc.Texture2DArray.MostDetailedMip = 0;

				return device->CreateShaderResourceView(tex, &viewdesc, srv);
			}

			break;
		}
	}

	return -1;
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
		elements[i].SemanticName = attrib[i].semanticName;

		elements[i].SemanticIndex = 0;
		elements[i].InstanceDataStepRate = 0;

		if (attrib[i].channel == EVertexAttributeChannel::eChannelPerVertex)
			elements[i].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA;
		else if (attrib[i].channel == EVertexAttributeChannel::eChannelPerInstance)
			elements[i].InputSlotClass = D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_INSTANCE_DATA;

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
			default: { elements[i].Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN; }
		}

	}

	return device->CreateInputLayout(&elements[0], attribCount, bytecode, bytecodeSize, inputlayout);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

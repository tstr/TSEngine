/*
	ui module drawing source
*/

#include "ui.h"
#include "imgui/imgui.h"

#include <tscore/debug/log.h>
#include <tscore/debug/assert.h>

using namespace ts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////

void UISystem::init()
{
	//Create the vertex shader
	static const char* shaderCode = R"(
		cbuffer uniforms : register(b0)
		{
			float4x4 projection;
		};

		Texture2D tex : register(t0);
		SamplerState texSampler : register(s0);

		struct vsInput
		{
			float2 pos : POSITION;
			float4 col : COLOR0;
			float2 uv  : TEXCOORD0;
		};

		struct psInput
		{
			float4 pos : SV_POSITION;
			float4 col : COLOR0;
			float2 uv  : TEXCOORD0;
		};

		psInput vs(vsInput input)
		{
			psInput output = (psInput)0;
			output.pos = mul(float4(input.pos.xy, 0.0f, 1.0f), projection);
			output.col = input.col;
			output.uv  = input.uv;
			return output;
		}

		float4 ps(psInput input) : SV_Target0
		{
			float4 out_col = input.col * tex.Sample(texSampler, input.uv);
			return out_col;
		}
	)";

	auto& shaderMng = m_rendermodule->getShaderManager();

	//Create shaders
	tsassert(shaderMng.createShaderFromString(m_vertexShader, shaderCode, "vs", eShaderStageVertex));
	tsassert(shaderMng.createShaderFromString(m_pixelShader, shaderCode, "ps", eShaderStagePixel));

	//Create shader input descriptor
	SShaderInputDescriptor inputdesc[3];
	inputdesc[0].bufferSlot = 0;
	inputdesc[0].byteOffset = (size_t)(&((ImDrawVert*)0)->pos);
	inputdesc[0].channel = EShaderInputChannel::eInputPerVertex;
	inputdesc[0].semanticName = "POSITION";
	inputdesc[0].type = eShaderInputFloat2;

	inputdesc[1].bufferSlot = 0;
	inputdesc[1].byteOffset = (size_t)(&((ImDrawVert*)0)->uv);
	inputdesc[1].channel = EShaderInputChannel::eInputPerVertex;
	inputdesc[1].semanticName = "TEXCOORD";
	inputdesc[1].type = eShaderInputFloat2;

	inputdesc[2].bufferSlot = 0;
	inputdesc[2].byteOffset = (size_t)(&((ImDrawVert*)0)->col);
	inputdesc[2].channel = EShaderInputChannel::eInputPerVertex;
	inputdesc[2].semanticName = "COLOR";
	inputdesc[2].type = eShaderInputRGBA;

	tsassert(!m_rendermodule->getApi()->createShaderInputDescriptor(m_vertexInput, shaderMng.getShaderProxy(m_vertexShader), inputdesc, 3));

	//Create buffers
	Matrix matrix;
	m_uniformBuffer = CUniformBuffer(m_rendermodule, matrix);

	m_vertexBufferSize = 10000;
	m_indexBufferSize = 5000;
	m_cpuIndexBuffer = new ImDrawIdx[m_indexBufferSize];
	m_cpuVertexBuffer = new ImDrawVert[m_vertexBufferSize];

	m_vertexBuffer = CVertexBuffer(m_rendermodule, m_cpuVertexBuffer, m_vertexBufferSize);
	m_indexBuffer = CIndexBuffer(m_rendermodule, m_cpuIndexBuffer, m_indexBufferSize);

	auto api = m_rendermodule->getApi();

	//Create texture atlas
	ImGuiIO& io = ImGui::GetIO();
	byte* pixels = nullptr;
	int width = 0;
	int height = 0;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	STextureResourceData atlasData;
	STextureResourceDescriptor atlasDesc;
	STextureViewDescriptor atlasViewDesc;

	atlasData.memory = pixels;
	atlasData.memoryByteWidth = 4 * width;
	atlasData.memoryByteDepth = 0;

	atlasDesc.arraySize = 1;
	atlasDesc.depth = 0;
	atlasDesc.height = height;
	atlasDesc.width = width;
	atlasDesc.multisampling.count = 1;
	atlasDesc.texformat = ETextureFormat::eTextureFormatColourRGBA;
	atlasDesc.texmask = ETextureResourceMask::eTextureMaskShaderResource;
	atlasDesc.textype = ETextureResourceType::eTypeTexture2D;
	atlasDesc.useMips = false;

	atlasViewDesc.arrayCount = 1;
	atlasViewDesc.arrayIndex = 0;

	ERenderStatus status = eOk;
	status = api->createResourceTexture(m_textureAtlasRsc, &atlasData, atlasDesc);
	tsassert(!status);
	status = api->createViewTexture2D(m_textureAtlasView, m_textureAtlasRsc, atlasViewDesc);
	tsassert(!status);

	//Store texture view as a pointer
	io.Fonts->TexID = (void*)m_textureAtlasView.get();

	//Create texture sampler
	STextureSamplerDescriptor samplerDesc;
	samplerDesc.addressU = eTextureAddressWrap;
	samplerDesc.addressV = eTextureAddressWrap;
	samplerDesc.addressW = eTextureAddressWrap;
	samplerDesc.filtering = eTextureFilterBilinear;
	api->createTextureSampler(m_textureAtlasSampler, samplerDesc);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UISystem::draw(IRenderContext* context, ResourceProxy rendertarget, Viewport viewport)
{
	ImDrawData* drawData = ImGui::GetDrawData();

	if ((uint32)drawData->TotalIdxCount > m_indexBufferSize)
	{
		m_indexBufferSize = drawData->TotalIdxCount;
		delete[] m_cpuIndexBuffer;
		m_cpuIndexBuffer = new ImDrawIdx[m_indexBufferSize];
		m_indexBuffer = CIndexBuffer(m_rendermodule, m_cpuIndexBuffer, m_indexBufferSize);
	}
	if ((uint32)drawData->TotalVtxCount > m_vertexBufferSize)
	{
		m_vertexBufferSize = drawData->TotalVtxCount;
		delete[] m_cpuVertexBuffer;
		m_cpuVertexBuffer = new ImDrawVert[m_vertexBufferSize];
		m_vertexBuffer = CVertexBuffer(m_rendermodule, m_cpuVertexBuffer, m_vertexBufferSize);
	}

	ImDrawVert* vbuf_ptr = m_cpuVertexBuffer;
	ImDrawIdx* ibuf_ptr = m_cpuIndexBuffer;
	for (int n = 0; n < drawData->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = drawData->CmdLists[n];
		memcpy(vbuf_ptr, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(ibuf_ptr, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		vbuf_ptr += cmd_list->VtxBuffer.Size;
		ibuf_ptr += cmd_list->IdxBuffer.Size;
	}
	//Update vertex/index buffers
	context->resourceBufferUpdate(m_vertexBuffer.getBuffer(), m_cpuVertexBuffer);
	context->resourceBufferUpdate(m_indexBuffer.getBuffer(), m_cpuIndexBuffer);

	//Update orthographics projection matrix
	float L = 0.0f;
	float R = ImGui::GetIO().DisplaySize.x;
	float B = ImGui::GetIO().DisplaySize.y;
	float T = 0.0f;
	float proj[4][4] =
	{
		{ 2.0f / (R - L),   0.0f,           0.0f,       0.0f },
		{ 0.0f,         2.0f / (T - B),     0.0f,       0.0f },
		{ 0.0f,         0.0f,           0.5f,       0.0f },
		{ (R + L) / (L - R),  (T + B) / (B - T),    0.5f,       1.0f },
	};
	Vector v0(proj[0]);
	Vector v1(proj[1]);
	Vector v2(proj[2]);
	Vector v3(proj[3]);

	Matrix ortho(v0, v1, v2, v3);
	Matrix::transpose(ortho);

	context->resourceBufferUpdate(m_uniformBuffer.getBuffer(), (const void*)&ortho);

	SRenderCommand command;

	command.renderTarget[0] = rendertarget;
	//command.depthTarget = depthtarget;
	command.viewport = viewport;

	auto& shaderMng = m_rendermodule->getShaderManager();

	command.vertexBuffer = m_vertexBuffer.getBuffer();
	command.indexBuffer = m_indexBuffer.getBuffer();
	command.shaders.stageVertex = shaderMng.getShaderProxy(m_vertexShader);
	command.shaders.stagePixel = shaderMng.getShaderProxy(m_pixelShader);

	command.vertexInputDescriptor = m_vertexInput;
	command.vertexTopology = EVertexTopology::eTopologyTriangleList;
	command.vertexStride = sizeof(ImDrawVert);

	command.textures[0] = m_textureAtlasView;
	command.textureSamplers[0] = m_textureAtlasSampler;
	command.uniformBuffers[0] = m_uniformBuffer.getBuffer();

	command.alphaBlending = true;

	// Render command lists
	int vtx_offset = 0;
	int idx_offset = 0;
	for (int n = 0; n < drawData->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = drawData->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				command.scissor.x = (uint32)pcmd->ClipRect.x;
				command.scissor.y = (uint32)pcmd->ClipRect.y;
				command.scissor.w = (uint32)pcmd->ClipRect.z;
				command.scissor.h = (uint32)pcmd->ClipRect.w;
				
				command.indexCount = pcmd->ElemCount;
				command.indexStart = idx_offset;
				command.vertexBase = vtx_offset;

				context->execute(command);
			}
			idx_offset += pcmd->ElemCount;
		}
		vtx_offset += cmd_list->VtxBuffer.Size;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void UISystem::destroy()
{
	m_textureAtlasRsc.reset(nullptr);
	m_textureAtlasView.reset(nullptr);
	m_textureAtlasSampler.reset(nullptr);
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_vertexInput.reset(nullptr);
	m_uniformBuffer = CUniformBuffer();
	m_vertexBuffer = CVertexBuffer();
	m_indexBuffer = CIndexBuffer();

	if (m_cpuVertexBuffer) delete[] m_cpuVertexBuffer;
	if (m_cpuIndexBuffer) delete[] m_cpuIndexBuffer;

	m_cpuVertexBuffer = nullptr;
	m_cpuIndexBuffer = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

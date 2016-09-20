/*
	Render API

	DirectX 11 implementation of rendering context
*/

#include "DX11render.h"
#include "DX11helpers.h"

using namespace std;
using namespace ts;
using namespace ts::dx11;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Constructor
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DX11RenderContext::DX11RenderContext(DX11RenderApi* api) : m_api(api)
{
	if (!m_api)
	{
		tserror("Unable to create DX11RenderContext, invalid parameter");
		return;
	}

	auto device = m_api->getDevice();

	if (FAILED(device->CreateDeferredContext(0, m_context.GetAddressOf())))
	{
		tserror("Unable to create ID3D11DeviceContext, invalid parameter");
	}
}

DX11RenderContext::~DX11RenderContext()
{

}

void DX11RenderContext::finish()
{
	m_contextCommandList.Reset();

	//Store current state in a command list
	m_context->FinishCommandList(false, m_contextCommandList.GetAddressOf());
}

void DX11RenderContext::reset()
{
	m_context->Flush();
	m_contextCommandList.Reset();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Rendering commands
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DX11RenderContext::execute(const SRenderCommand& command)
{
	//Bind render targets and depth stencil view
	auto rtarget = DX11View::upcast(command.renderTarget[0].get());
	auto dtarget = DX11View::upcast(command.depthTarget.get());
	auto rtv = (rtarget) ? (ID3D11RenderTargetView*)rtarget->get() : nullptr;
	auto dtv = (dtarget) ? (ID3D11DepthStencilView*)dtarget->get() : nullptr;
	m_context->OMSetRenderTargets(1, &rtv, dtv);

	//Bind viewport
	D3D11_VIEWPORT viewport;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	viewport.TopLeftX = (FLOAT)command.viewport.x;
	viewport.TopLeftY = (FLOAT)command.viewport.y;
	viewport.Width = (FLOAT)command.viewport.w;
	viewport.Height = (FLOAT)command.viewport.h;
	m_context->RSSetViewports(1, &viewport);

	if (command.scissor.x || command.scissor.y || command.scissor.w || command.scissor.h)
	{
		D3D11_RECT scissorrect;
		scissorrect.top = command.scissor.y;
		scissorrect.left = command.scissor.x;
		scissorrect.bottom = command.scissor.h;
		scissorrect.right = command.scissor.w;
		m_context->RSSetScissorRects(1, &scissorrect);
	}
	else
	{
		D3D11_RECT scissorrect;
		scissorrect.top = 0;
		scissorrect.left = 0;
		scissorrect.bottom = command.viewport.h;
		scissorrect.right = command.viewport.w;
		m_context->RSSetScissorRects(1, &scissorrect);
	}

	//Bind states
	if (command.alphaBlending)
	{
		const float blendfactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_context->OMSetBlendState(m_api->getBlendState().Get(), blendfactor, UINT_MAX);
	}
	else
	{
		const float blendfactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_context->OMSetBlendState(nullptr, nullptr, UINT_MAX);
	}
	
	//m_context->OMSetDepthStencilState(m_api->getDepthStencilState().Get(), 0);
	//m_context->RSSetState(m_api->getRasterizerState().Get());
	
	//Bind shaders
	auto vshader = DX11Shader::upcast(command.shaders.stageVertex.get());
	auto pshader = DX11Shader::upcast(command.shaders.stagePixel.get());
	auto gshader = DX11Shader::upcast(command.shaders.stageGeometry.get());
	auto dshader = DX11Shader::upcast(command.shaders.stageDomain.get());
	auto hshader = DX11Shader::upcast(command.shaders.stageHull.get());

	if (vshader) m_context->VSSetShader((ID3D11VertexShader*)vshader->getShader(), nullptr, 0);
	if (pshader) m_context->PSSetShader((ID3D11PixelShader*)pshader->getShader(), nullptr, 0);
	if (gshader) m_context->GSSetShader((ID3D11GeometryShader*)gshader->getShader(), nullptr, 0);
	if (dshader) m_context->DSSetShader((ID3D11DomainShader*)dshader->getShader(), nullptr, 0);
	if (hshader) m_context->HSSetShader((ID3D11HullShader*)hshader->getShader(), nullptr, 0);

	//Bind textures
	for (int i = 0; i < EResourceLimits::eMaxTextureSlots; i++)
	{
		if (auto tex = DX11Texture::upcast(command.textures[i].get()))
		{
			auto srv = (ID3D11ShaderResourceView*)tex->get();
			m_context->PSSetShaderResources(i, 1, &srv);
		}
	}

	//Bind texture samples
	for (int i = 0; i < EResourceLimits::eMaxTextureSamplerSlots; i++)
	{
		if (auto texsampler = DX11TextureSampler::upcast(command.textureSamplers[i].get()))
		{
			auto sampler = (ID3D11SamplerState*)texsampler->get();
			m_context->PSSetSamplers(i, 1, &sampler);
		}
	}

	//Bind vertex buffer
	if (auto vbuf = DX11Buffer::upcast(command.vertexBuffer.get()))
	{
		auto v = vbuf->get();
		uint32 offset = 0;
		m_context->IASetVertexBuffers(0, 1, &v, &command.vertexStride, &offset);
	}

	//Bind index buffer
	if (auto ibuf = DX11Buffer::upcast(command.indexBuffer.get()))
	{
		m_context->IASetIndexBuffer(ibuf->get(), DXGI_FORMAT_R32_UINT, 0);
	}

	//Bind constant buffer
	for (int i = 0; i < EResourceLimits::eMaxUniformBuffers; i++)
	{
		if (auto ubuf = DX11Buffer::upcast(command.uniformBuffers[i].get()))
		{
			auto u = ubuf->get();
			m_context->VSSetConstantBuffers(i, 1, &u);
			m_context->PSSetConstantBuffers(i, 1, &u);
			m_context->GSSetConstantBuffers(i, 1, &u);
			m_context->HSSetConstantBuffers(i, 1, &u);
			m_context->DSSetConstantBuffers(i, 1, &u);
		}
	}

	//Bind an input layout
	if (auto sid = DX11ShaderInputDescriptor::upcast(command.vertexInputDescriptor.get()))
	{
		m_context->IASetInputLayout(sid->getLayout());
	}

	//Set vertex primitive topology
	D3D11_PRIMITIVE_TOPOLOGY topology;

	switch (command.vertexTopology)
	{
	case (EVertexTopology::eTopologyTriangleList): { topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break; }
	case (EVertexTopology::eTopologyTriangleStrip): { topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break; }
	case (EVertexTopology::eTopologyLineList): { topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST; break; }
	case (EVertexTopology::eTopologyLineStrip): { topology = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP; break; }
	case (EVertexTopology::eTopologyPointList): { topology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST; break; }
	default: { topology = D3D11_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_UNDEFINED; }
	}

	m_context->IASetPrimitiveTopology(topology);

	//Call an indexed draw call
	if (command.indexCount > 0)
	{
		if (command.instanceCount > 1)
		{
			m_context->DrawIndexedInstanced(command.indexCount, command.instanceCount, command.indexStart, command.vertexBase, 0);
		}
		else
		{
			m_context->DrawIndexed(command.indexCount, command.indexStart, command.vertexBase);
		}
	}
	else
	{
		if (command.instanceCount > 1)
		{
			m_context->DrawInstanced(command.vertexCount, command.instanceCount, command.vertexStart, 0);
		}
		else
		{
			m_context->Draw(command.vertexCount, command.vertexStart);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Resource updating
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DX11RenderContext::clearRenderTarget(const ResourceProxy& viewptr, const Vector& vec)
{
	auto view = DX11View::upcast(viewptr.get());

	if (view && view->isRTV())
	{
		m_context->ClearRenderTargetView((ID3D11RenderTargetView*)view->get(), (const float*)&vec);
	}
}

void DX11RenderContext::clearDepthTarget(const ResourceProxy& viewptr, float depth)
{
	auto view = DX11View::upcast(viewptr.get());

	if (view && view->isDTV())
	{
		m_context->ClearDepthStencilView((ID3D11DepthStencilView*)view->get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DX11RenderContext::resourceBufferUpdate(const ResourceProxy& rsc, const void* memory)
{
	auto buffer = DX11Buffer::upcast(rsc.get());

	if (buffer)
	{
		m_context->UpdateSubresource(buffer->get(), 0, nullptr, memory, 0, 0);
	}
	else
	{
		tswarn("unable to update buffer");
	}
}

void DX11RenderContext::resourceBufferCopy(const ResourceProxy& src, ResourceProxy& dest)
{
	auto buffersrc = DX11Buffer::upcast(src.get());
	auto bufferdest = DX11Buffer::upcast(dest.get());

	if (buffersrc && bufferdest)
	{
		m_context->CopyResource(bufferdest->get(), buffersrc->get());
	}
	else
	{
		tswarn("unable to copy buffers");
	}
}

void DX11RenderContext::resourceTextureUpdate(uint32 index, ResourceProxy& rsc, const void* memory)
{
	auto texture = DX11Texture::upcast(rsc.get());

	if (texture)
	{
		m_context->UpdateSubresource(texture->get(), index, nullptr, memory, 0, 0);
	}
	else
	{
		tswarn("unable to update texture");
	}
}

void DX11RenderContext::resourceTextureCopy(const ResourceProxy& src, ResourceProxy& dest)
{
	auto texsrc = DX11Texture::upcast(src.get());
	auto texdest = DX11Texture::upcast(dest.get());

	if (texsrc && texdest)
	{
		m_context->CopyResource(texdest->get(), texsrc->get());
	}
	else
	{
		tswarn("unable to copy textures");
	}
}

void DX11RenderContext::resourceTextureResolve(const ResourceProxy& src, ResourceProxy& dest)
{
	auto texsrc = DX11Texture::upcast(src.get());
	auto texdest = DX11Texture::upcast(dest.get());

	if (texsrc && texdest)
	{
		m_context->ResolveSubresource(texdest->get(), 0, texsrc->get(), 0, DXGI_FORMAT_UNKNOWN);
	}
	else
	{
		tswarn("unable to resolve textures");
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
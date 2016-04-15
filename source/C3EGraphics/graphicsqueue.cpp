/*
	Graphics queue source

	- Handles queueing and rendering of render packets
*/

#include "pch.h"

#include <C3E\gfx\graphicsbase.h>
#include <C3E\gfx\abi\graphicsabi.h>

using namespace std;
using namespace C3E;

struct GraphicsQueue::Impl
{
	Graphics* m_renderer = nullptr;
	ABI::IRenderContext* m_rendercontext = nullptr;

	vector<GraphicsPacket> m_renderpacket_queue;
	vector<pair<GraphicsBuffer*, MemoryBuffer>> m_bufferUpdateQueue;
};

//state change optimization test
//#define TEST

/////////////////////////////////////////////////////////////////////////////////////////////////////////////

GraphicsQueue::GraphicsQueue(Graphics* renderer) :
	pImpl(new Impl)
{
	pImpl->m_renderer = renderer;

	auto api = (ABI::IRenderApi*)pImpl->m_renderer->api();
	pImpl->m_rendercontext = api->CreateContext();
}

GraphicsQueue::~GraphicsQueue()
{
	auto api = (ABI::IRenderApi*)pImpl->m_renderer->api();
	api->DestroyContext(pImpl->m_rendercontext);
}

void GraphicsQueue::Enqueue(GraphicsPacket& packet)
{
	if (!packet.block.m_renderTarget)
	{
		packet.block.m_renderTarget = pImpl->m_renderer->api()->GetDefaultRenderTarget();
	}
	if (!packet.block.m_depthBuffer)
	{
		packet.block.m_depthBuffer = pImpl->m_renderer->api()->GetDefaultDepthBuffer();
	}

	//todo: actual sorting of draw calls
	pImpl->m_renderpacket_queue.push_back(packet);
	packet.m_bufferUpdateCount = 0;
}

void GraphicsQueue::UpdateBuffer(GraphicsBuffer* buf, const void* mem, uint32 size)
{
	pImpl->m_bufferUpdateQueue.push_back(make_pair(buf, MemoryBuffer(mem, size)));
}

void GraphicsQueue::UpdateBuffer(GraphicsBuffer* buf, const MemoryBuffer& b)
{
	pImpl->m_bufferUpdateQueue.push_back(make_pair(buf, b));
}

void GraphicsQueue::ClearRenderTarget(const GraphicsRenderTarget* rt, const Colour& c)
{
	ABI::IRenderContext* rc = pImpl->m_rendercontext;

	rc->ClearRenderTarget(rt->hrt(), c);
}

void GraphicsQueue::Dispatch()
{
	//API backend interface
	auto api = (ABI::IRenderApi*)pImpl->m_renderer->api();
	//API command buffer
	ABI::IRenderContext* rc = pImpl->m_rendercontext;

	//Colour c = Colours::DarkBlue;
	//rc->ClearRenderTarget(api->GetDefaultRenderTarget(), c);

	if (!pImpl->m_renderpacket_queue.empty())
	{
		rc->ContextBegin();

		if (!pImpl->m_bufferUpdateQueue.empty())
		{
			for (auto& pair : pImpl->m_bufferUpdateQueue)
			{
				rc->ResourceSet(pair.first->handle(), pair.second.pointer(), (uint32)pair.second.size());
			}
		}

		//rc->SetRenderTarget(api->GetDefaultRenderTarget(), true);

		pImpl->m_bufferUpdateQueue.clear();

		GraphicsPacket::CommandBlock dummy_block;
		ZeroMemory(&dummy_block, sizeof(dummy_block));
		const GraphicsPacket::CommandBlock* block_cache = &dummy_block;
		
#ifdef TEST

		for (const DrawPacket& packet : pImpl->m_renderpacket_queue)
		{
			if (packet.m_bufferUpdateCount)
			{
				for (uint8 i = 0; i < packet.m_bufferUpdateCount; i++)
				{
					auto& p = packet.m_bufferUpdateQueue[i];
					rc->BufferSet(p.first->handle(), p.second.pointer(), (uint32)p.second.size());
				}
			}

			if (!packet.m_effect)
			{
				return;
				throw exception();
			}

			rc->SetShader(packet.m_effect->GetShader(ABI::ShaderType::SHADER_TYPE_VERTEX), ABI::ShaderType::SHADER_TYPE_VERTEX);
			rc->SetShader(packet.m_effect->GetShader(ABI::ShaderType::SHADER_TYPE_PIXEL), ABI::ShaderType::SHADER_TYPE_PIXEL);
			rc->SetShader(packet.m_effect->GetShader(ABI::ShaderType::SHADER_TYPE_GEOMETRY), ABI::ShaderType::SHADER_TYPE_GEOMETRY);
			rc->SetShaderInputDescriptor(packet.m_effect->GetShaderInputDescriptor());

			for (int i = 0; i < packet.m_effectresources.size(); i++)
			{
				EffectResource* tex = packet.m_effectresources[i];
				if (tex && tex->handle())
				{
					rc->SetShaderResource(i, tex->handle());
				}
			}

			for (int i = 0; i < packet.m_effectbuffers.size(); i++)
			{
				GraphicsBuffer* b = packet.m_effectbuffers[i];
				if (b)
				{
					rc->SetShaderBuffer(i, b->handle());
				}
			}

			for (int i = 0; i < packet.m_vertexbuffers.size(); i++)
			{
				GraphicsBuffer* b = packet.m_vertexbuffers[i];
				if (b)
				{
					rc->SetVertexBuffer(i, b->handle(), (uint32)sizeof(Vertex), 0);
				}
			}

			if (packet.m_indexbuffer)
				rc->SetIndexBuffer(packet.m_indexbuffer->handle(), 0);

			rc->SetVertexTopology(packet.m_topology);

			if (packet.m_indexed && packet.m_indexbuffer)
			{
				rc->DrawIndexed(packet.m_indexStart, packet.m_indexCount, 0);
			}
			else
			{
				rc->Draw(packet.m_vertexStart, packet.m_vertexCount);
			}
		}

#else
		//Iterate over sorted draw calls
		for (const GraphicsPacket& packet : pImpl->m_renderpacket_queue)
		{
			const GraphicsPacket::CommandBlock& block = packet.block;

			if (packet.m_bufferUpdateCount)
			{
				for (uint8 i = 0; i < packet.m_bufferUpdateCount; i++)
				{
					auto& p = packet.m_bufferUpdateQueue[i];
					rc->ResourceSet(p.first->handle(), p.second.pointer(), (uint32)p.second.size());
				}
			}

			if (block_cache->m_renderTarget != block.m_renderTarget ||
				block_cache->m_depthBuffer != block.m_depthBuffer ||
				block_cache->m_depthIndex != block.m_depthIndex ||
				block_cache->m_nodepth != block.m_nodepth
			)
			{
				//rc->SetRenderTarget(block.m_renderTarget, block.m_depthBuffer, block.m_depthIndex);
				//*
				rc->SetRenderTarget(
					block.m_renderTarget,
					//(block.m_norender) ? nullptr : block.m_renderTarget,
					(block.m_nodepth) ? nullptr : block.m_depthBuffer,
					block.m_depthIndex,
					block.m_renderIndex
				);
				//*/
			}

			rc->EnableAlphaBlending(block.m_blending);

			//Only set shaders if not bound in previous packet
			if (block_cache->m_vertexShader != block.m_vertexShader) rc->SetShader(block.m_vertexShader, ABI::ShaderType::SHADER_TYPE_VERTEX);
			if (block_cache->m_pixelShader != block.m_pixelShader) rc->SetShader(block.m_pixelShader, ABI::ShaderType::SHADER_TYPE_PIXEL);
			if (block_cache->m_geometryShader != block.m_geometryShader) rc->SetShader(block.m_geometryShader, ABI::ShaderType::SHADER_TYPE_GEOMETRY);

			//Only set shader input descriptor if not bound in previous packet
			if (block_cache->m_vertexShaderDescriptor != block.m_vertexShaderDescriptor)
				rc->SetShaderInputDescriptor(block.m_vertexShaderDescriptor);

			for (int i = 0; i < ABI::MAX_SRVS; i++)
			{
				//Only set shader resources if not bound in previous packet
				if (block_cache->m_srvs[i] == block.m_srvs[i])
					continue;

				rc->SetShaderResource(i, block.m_srvs[i]);
			}

			for (int i = 0; i < ABI::MAX_SHADER_BUFFERS; i++)
			{
				//Only set shader buffers if not bound in previous packet
				if (block_cache->m_shaderBuffers[i] == block.m_shaderBuffers[i])
					continue;

				rc->SetShaderBuffer(i, block.m_shaderBuffers[i]);
			}

			for (int i = 0; i < ABI::MAX_VERTEX_BUFFERS; i++)
			{
				//Only set vertex buffers if not bound in previous packet
				if (block_cache->m_vertexBuffers[i] == block.m_vertexBuffers[i])
					continue;

				rc->SetVertexBuffer(i, block.m_vertexBuffers[i], (uint32)sizeof(Vertex), 0);
			}

			//Only set primitive topology if not bound in previous packet
			if (block_cache->m_vertexTopology != block.m_vertexTopology)
				rc->SetVertexTopology(block.m_vertexTopology);

			if (block.m_indexed)
			{
				//Only set index buffer if not bound in previous packet
				if (block_cache->m_indexBuffer != block.m_indexBuffer)
					rc->SetIndexBuffer(block.m_indexBuffer, 0);

				rc->DrawIndexed(block.m_indexStart, block.m_indexCount, 0);
			}
			else
			{
				rc->Draw(block.m_vertexStart, block.m_vertexCount);
			}

			block_cache = &block;
		}
#endif

		rc->ContextEnd();

		api->DispatchContext(rc);

		pImpl->m_renderpacket_queue.clear();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
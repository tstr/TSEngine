/*
	Buffer pool
*/

#include <tscore/debug/assert.h>

#include <tsgraphics/BufferPool.h>
#include <tsgraphics/api/RenderApi.h>

using namespace ts;
using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Buffer allocation methods
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CBufferPool::Handle CBufferPool::createConstantBuffer(const void* data, size_t dataSize)
{
	tsassert(m_graphics);
	auto api = m_graphics->getApi();
	
	SBufferResourceData desc;
	desc.memory = data;
	desc.size = (uint32)dataSize;
	desc.usage = EBufferType::eBufferTypeConstant;

	HBuffer b;
	if (api->createResourceBuffer(b, desc))
	{
		//error handling
		return HBUFFER_NULL;
	}

	//Cache handle
	m_pool.push_back(b);

	return b;
}

CBufferPool::Handle CBufferPool::createVertexBuffer(const void* data, size_t dataSize)
{
	tsassert(m_graphics);
	auto api = m_graphics->getApi();

	SBufferResourceData desc;
	desc.memory = data;
	desc.size = (uint32)dataSize;
	desc.usage = EBufferType::eBufferTypeVertex;

	HBuffer b;
	if (api->createResourceBuffer(b, desc))
	{
		//error handling
		return HBUFFER_NULL;
	}

	//Cache handle
	m_pool.push_back(b);

	return b;
}

CBufferPool::Handle CBufferPool::createIndexBuffer(const void* data, size_t dataSize)
{
	tsassert(m_graphics);
	auto api = m_graphics->getApi();

	SBufferResourceData desc;
	desc.memory = data;
	desc.size = (uint32)dataSize;
	desc.usage = EBufferType::eBufferTypeIndex;

	HBuffer b;
	if (api->createResourceBuffer(b, desc))
	{
		//error handling
		return HBUFFER_NULL;
	}

	//Cache handle
	m_pool.push_back(b);

	return b;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Buffer destruction
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CBufferPool::destroy(Handle buffer)
{
	tsassert(m_graphics);

	auto it = find(m_pool.begin(), m_pool.end(), buffer);

	if (it != m_pool.end())
	{
		m_graphics->getApi()->destroyBuffer(*it);
		m_pool.erase(it);
	}
}

void CBufferPool::clear()
{
	tsassert(m_graphics);

	for (Handle h : m_pool)
	{
		m_graphics->getApi()->destroyBuffer(h);
	}

	m_pool.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

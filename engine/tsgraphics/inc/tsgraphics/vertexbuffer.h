/*
	Vertex buffer class
*/

#pragma once

#include "buffercommon.h"

namespace ts
{
	class CVertexBuffer : public CBuffer
	{
	private:
		
		uint32 m_vertexStride = 0;
		uint32 m_vertexCount = 0;
		
	public:
		
		CVertexBuffer() {}

		CVertexBuffer(GraphicsSystem* module) :
			CVertexBuffer::CBuffer(module)
		{}
		
		template<typename t>
		CVertexBuffer(GraphicsSystem* module, const t* vertices, uint32 vertexCount) :
			CVertexBuffer::CBuffer(module)
		{
			setVertexArray(vertices, vertexCount);
			m_vertexStride = (uint32)sizeof(t);
			m_vertexCount = vertexCount;
		}
		
		template<typename t>
		bool setVertexArray(const t* vertices, uint32 vertexCount)
		{
			return setBuffer(vertices, sizeof(t) * vertexCount, EBufferType::eBufferTypeVertex);
		}

		uint32 getVertexStride() const { return m_vertexStride; }
		uint32 getVertexCount() const { return m_vertexCount; }
	};
}

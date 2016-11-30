/*
	Index buffer class
*/

#pragma once

#include "buffercommon.h"

namespace ts
{
	typedef uint32 Index;

	class CIndexBuffer : public CBuffer
	{
	private:

		uint32 m_indexCount = 0;

	public:

		CIndexBuffer() {}

		CIndexBuffer(CRenderModule* module) :
			CIndexBuffer::CBuffer(module)
		{}

		CIndexBuffer(CRenderModule* module, const Index* indices, uint32 indexCount) :
			CIndexBuffer::CBuffer(module)
		{
			setIndexArray(indices, indexCount);
			m_indexCount = indexCount;
		}

		bool setIndexArray(const Index* indices, uint32 indexCount)
		{
			return setBuffer(indices, sizeof(Index) * indexCount, EBufferType::eBufferTypeIndex);
		}

		uint32 getIndexCount() const { return m_indexCount; }
	};
}

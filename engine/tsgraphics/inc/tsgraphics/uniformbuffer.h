/*
	Uniform buffer class
*/

#pragma once

#include "buffercommon.h"

namespace ts
{
	class CUniformBuffer : public CBuffer
	{
	public:

		CUniformBuffer() {}

		CUniformBuffer(GraphicsSystem* module) :
			CUniformBuffer::CBuffer(module)
		{}
		
		template<typename t>
		CUniformBuffer(GraphicsSystem* module, const t& data) :
			CUniformBuffer::CBuffer(module)
		{
			setUniformBufferRaw(data);
		}

		template<
			typename t,
			class = std::enable_if<!(sizeof(t) % 16)>::type
		>
		void setUniformBufferRaw(const t& data)
		{
			setBuffer(&data, sizeof(t), EBufferType::eBufferTypeUniform);
		}
	};
}

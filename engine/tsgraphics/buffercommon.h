/*
	Buffer base class
*/

#pragma once

#include "rendercommon.h"
#include "renderapi.h"

#include <tscore/system/memory.h>

namespace ts
{
	class CRenderModule;

	class CBuffer
	{
	private:
		
		CRenderModule* m_module = nullptr;
		
		ResourceProxy m_hardwareBuffer;
		MemoryBuffer m_buffer;
		
	protected:
		
		bool setBuffer(const void* memory, uint32 memorysize, EBufferType type);
		
	public:
		
		CRenderModule* getModule() const { return m_module; }
		ResourceProxy getBuffer() { return m_hardwareBuffer; }
		
		CBuffer() {}
		CBuffer(CRenderModule* module);
	};
}

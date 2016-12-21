/*
	Buffer base class
*/

#pragma once

#include <tsgraphics/abi.h>
#include <tsgraphics/api/renderapi.h>
#include <tsgraphics/api/rendercommon.h>

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
		
		bool TSGRAPHICS_API setBuffer(const void* memory, uint32 memorysize, EBufferType type);
		
	public:
		
		CRenderModule* getModule() const { return m_module; }
		ResourceProxy getBuffer() { return m_hardwareBuffer; }
		
		CBuffer() {}
		TSGRAPHICS_API CBuffer(CRenderModule* module);
	};
}

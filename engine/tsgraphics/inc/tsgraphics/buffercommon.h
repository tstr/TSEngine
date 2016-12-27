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
	class GraphicsSystem;

	class CBuffer
	{
	private:
		
		GraphicsSystem* m_system = nullptr;
		
		HBuffer m_hardwareBuffer;
		MemoryBuffer m_buffer;
		
	protected:
		
		bool TSGRAPHICS_API setBuffer(const void* memory, uint32 memorysize, EBufferType type);
		
	public:
		
		GraphicsSystem* getModule() const { return m_system; }
		HBuffer getBuffer() { return m_hardwareBuffer; }
		
		CBuffer() {}
		TSGRAPHICS_API CBuffer(GraphicsSystem* module);
	};
}

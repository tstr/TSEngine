/*
	Buffer source
*/

#include <tsgraphics/rendermodule.h>

#include <tsgraphics/vertexbuffer.h>
#include <tsgraphics/indexbuffer.h>
#include <tsgraphics/uniformbuffer.h>

#include <tscore/debug/assert.h>

using namespace ts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////
//Buffer base class
/////////////////////////////////////////////////////////////////////////////////////////////////

CBuffer::CBuffer(CRenderModule* module) :
	m_module(module)
{
	tsassert(m_module);
}

bool CBuffer::setBuffer(const void* memory, uint32 memorysize, EBufferType type)
{
	auto api = m_module->getApi();
	SBufferResourceData data;
	data.memory = memory;
	data.size = memorysize;
	data.usage = type;

	if (ERenderStatus status = api->createResourceBuffer(m_hardwareBuffer, data))
		return false;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

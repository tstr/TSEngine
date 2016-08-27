/*
	Buffer source
*/

#include "rendermodule.h"

#include "vertexbuffer.h"
#include "indexbuffer.h"
#include "uniformbuffer.h"

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

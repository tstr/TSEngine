/*
	Render Module Common header
	
	Defines common types shared between high level module and low level rendering implementation
*/

#pragma once

#include <tscore/types.h>
#include <tscore/maths.h>
#include <tscore/strings.h>

namespace ts
{
	enum ERenderApiFlags : uint8
	{
		eFlagNull		   = 0,
		eFlagDebug		   = 1,
		eFlagReportObjects = 2
	};

	struct SRenderAdapterDesc
	{
		StaticString<128> adapterName;
		uint64 gpuVideoMemory;		//GPU accessible video memory capacity
		uint64 gpuSystemMemory;		//GPU accessible system memory capacity
		uint64 sharedSystemMemory;	//GPU/CPU accessible system memory capacity
	};

	struct SMultisampling
	{
		uint32 count = 1;

		SMultisampling() {}
		SMultisampling(uint32 c) : count(c) {}
	};
	
	enum EVertexTopology
	{
		eTopologyUnknown,
		eTopologyPointList,
		eTopologyLineList,
		eTopologyLineStrip,
		eTopologyTriangleList,
		eTopologyTriangleStrip
	};
}
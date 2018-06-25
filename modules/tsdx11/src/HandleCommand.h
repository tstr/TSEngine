/*
	Render API
	
	Command object
*/

#pragma once

#include "Base.h"
#include "Handle.h"
#include "HandlePipeline.h"
#include "HandleResourceSet.h"
#include "HandleTarget.h"

namespace ts
{
	struct DxDrawCommand : public Handle<DxDrawCommand, CommandHandle>
	{
        DxPipeline* pipeline;
        DxResourceSet* inputs;
        DxTarget* outputs;

		uint32 start = 0;
		uint32 count = 0;
		int32 vertexBase = 0;
		uint32 instances = 1;

		DrawMode mode;
	};
}
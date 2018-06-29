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

		DrawCommandParams params;
	};
}
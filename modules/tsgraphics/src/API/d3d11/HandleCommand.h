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
	struct D3D11DrawCommand : public Handle<D3D11DrawCommand, CommandHandle>
	{
        D3D11Pipeline* pipeline;
        D3D11ResourceSet* inputs;
        D3D11Target* outputs;

		uint32 start = 0;
		uint32 count = 0;
		int32 vertexBase = 0;
		uint32 instances = 1;

		DrawMode mode;
	};
}
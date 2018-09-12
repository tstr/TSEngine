/*
	Renderable structure
*/

#pragma once

#include <tsgraphics/BindingSet.h>
#include <tsgraphics/Buffer.h>

namespace ts
{
	struct Renderable
	{
		Buffer materialBuffer;

		RPtr<PipelineHandle> pso;
		RPtr<ResourceSetHandle> inputs;

		RPtr<PipelineHandle> shadowPso;
		RPtr<ResourceSetHandle> shadowInputs;

		DrawParams params;
	};
}

/*
	Renderable structure
*/

#pragma once

#include <tsgraphics/BindingSet.h>
#include <tsgraphics/Buffer.h>

namespace ts
{
	struct MaterialInstance
	{
		BindingSet<ImageView> images;
		Buffer buffer;
	};

	struct Renderable
	{
		MaterialInstance mat;

		RPtr<PipelineHandle> pso;
		RPtr<ResourceSetHandle> inputs;

		RPtr<PipelineHandle> shadowPso;
		RPtr<ResourceSetHandle> shadowInputs;

		DrawParams params;
	};
}

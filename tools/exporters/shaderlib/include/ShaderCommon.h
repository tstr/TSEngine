/*
	Common type definitions
*/

#pragma once

#include <tscore/types.h>
#include <tscore/strings.h>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace tsr
{
	enum VariableType;
	enum ResourceType;
	
	class ShaderReflectionBuilder;
	class ShaderStageBuilder;
	class ShaderBuilder;
}

namespace ts
{
	enum EShaderStage
	{
		eShaderStageUnknown,
		eShaderStageVertex,
		eShaderStageTessCtrl, //hull
		eShaderStageTessEval, //domain
		eShaderStageGeometry,
		eShaderStagePixel,
		eShaderStageCompute
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////

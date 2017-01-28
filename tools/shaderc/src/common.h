/*
	Common type definitions
*/

#pragma once

#include <tscore/types.h>
#include <tscore/strings.h>

#include "preprocessor.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

struct SShaderStageInfo
{
	ts::Path sourceFile;
	std::string entryPoint;
	std::vector<ts::SPreprocessorMacro> macros;
};

struct SShaderInfo
{
	SShaderStageInfo vertexStage;
	SShaderStageInfo hullStage;
	SShaderStageInfo domainStage;
	SShaderStageInfo geometryStage;
	SShaderStageInfo pixelStage;
};

enum EShaderStage
{
	eShaderStageUnknown,
	eShaderStageVertex,
	eShaderStagePixel,
	eShaderStageGeometry,
	eShaderStageHull,
	eShaderStageDomain,
	eShaderStageCompute
};

/////////////////////////////////////////////////////////////////////////////////////////////////

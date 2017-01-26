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
	const char* entryPoint = nullptr;
	const ts::SPreprocessorMacro* macros = nullptr;
	size_t macroCount = 0;
};

struct SShaderInfo
{
	SShaderStageInfo vertexStage;
	SShaderStageInfo hullStage;
	SShaderStageInfo domainStage;
	SShaderStageInfo geometryStage;
	SShaderStageInfo pixelStage;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

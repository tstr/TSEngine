/*
	CTCore.lib - Precompiled header
*/

#pragma once

#ifdef CT_USE_DYNAMIC_LIB
#define CT_CORE_API __declspec(dllexport)
#endif

#include <Windows.h>
#include <ct\core\corecommon.h>

#include <ct\core\maths.h>
#include <ct\core\console.h>
#include <ct\core\threading.h>
#include <ct\core\time.h>
#include <ct\core\utility.h>

//#include "Core.h"
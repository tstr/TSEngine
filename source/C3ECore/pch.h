/*
	C3ECore.lib - Precompiled header
*/

#pragma once

#ifdef C3E_USE_DYNAMIC_LIB
#define C3E_CORE_API __declspec(dllexport)
#endif

#include <C3Ext\win32.h>
#include <C3E\core\corecommon.h>

#include <C3E\core\maths.h>
#include <C3E\core\console.h>
#include <C3E\core\threading.h>
#include <C3E\core\time.h>
#include <C3E\core\utility.h>

//#include "Core.h"
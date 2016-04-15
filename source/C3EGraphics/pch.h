/*
	C3EGraphics.lib - precompiled header
*/

#pragma once

#include <Windows.h>
#include <C3E\core\corecommon.h>

#ifdef C3E_USE_DYNAMIC_LIB
#define C3E_GFX_API EXPORT_ATTRIB
#endif
#include <C3E\gfx\graphics.h>

#include <C3E\core\maths.h>
#include <C3E\core\console.h>
#include <C3E\core\threading.h>
#include <C3E\core\time.h>
#include <C3E\core\utility.h>
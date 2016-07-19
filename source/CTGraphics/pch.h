/*
	CTGraphics.lib - precompiled header
*/

#pragma once

#include <Windows.h>
#include <ct\core\corecommon.h>

#ifdef CT_USE_DYNAMIC_LIB
#define CT_GFX_API EXPORT_ATTRIB
#endif
#include <ct\graphics\graphics.h>

#include <ct\core\maths.h>
#include <ct\core\console.h>
#include <ct\core\threading.h>
#include <ct\core\time.h>
#include <ct\core\utility.h>
/*
	Graphics module source
*/

#include "rendermodule.h"

#include <tscore/debug/assert.h>

#include <Windows.h>

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////s

CRenderModule::CRenderModule(const SRenderModuleConfiguration& cfg)
{
	HWND hwnd = reinterpret_cast<HWND>(cfg.targethandle);
	tsassert(IsWindow(hwnd));
}

CRenderModule::~CRenderModule()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////
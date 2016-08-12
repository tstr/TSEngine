/*
	Graphics module source
*/

#include <Windows.h>
#include "rendermodule.h"

//D3D11 interface
#include "API/DX11/DX11Render.h"

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////s

CRenderModule::CRenderModule(const SRenderModuleConfiguration& cfg)
{
	SRenderApiConfiguration apicfg;
	apicfg.adapterIndex = 0; //hard code the adapter for now
	apicfg.resolutionHeight = cfg.height;
	apicfg.resolutionWidth = cfg.width;
	apicfg.windowFullscreen = false;
	apicfg.windowHandle = cfg.windowHandle;

	m_api.reset(new dx11::DX11RenderApi(apicfg));
}

CRenderModule::~CRenderModule()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////

void CRenderModule::setScreenState(ERenderScreenState state)
{
	
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void CRenderModule::drawBegin(const Vector& vec)
{
	m_api->drawBegin(vec);
}

void CRenderModule::drawEnd()
{
	m_api->drawEnd();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
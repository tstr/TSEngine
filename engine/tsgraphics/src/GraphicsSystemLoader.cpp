/*
	Graphics module api loader
*/

#include <Windows.h>
#include <tsgraphics/rendermodule.h>

//D3D11 interface
#include "API/DX11/DX11Render.h"

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////s

int CRenderModule::loadApi(ERenderApiID id)
{
	if (id != eRenderApiD3D11)
	{
		tswarn("API's other than direct3d 11 are not supported at the moment.");
		return false;
	}

	dx11::DX11RenderAdapterFactory adapterfactory;

	for (uint32 i = 0; i < adapterfactory.getAdapterCount(); i++)
	{
		SRenderAdapterDesc desc;
		adapterfactory.enumAdapter(i, desc);

		tsinfo("Adapter(%): %", i, desc.adapterName.str());
	}

	SRenderApiConfiguration apicfg;
	apicfg.adapterIndex = 0; //hard code the adapter for now
	apicfg.resolutionHeight = m_config.height;
	apicfg.resolutionWidth = m_config.width;
	apicfg.fullscreen = (m_config.displaymode == EDisplayMode::eDisplayFullscreen);
	apicfg.windowHandle = m_config.windowHandle;
	apicfg.multisampleCount = m_config.multisampling.count;

#ifdef _DEBUG
	apicfg.flags |= ERenderApiFlags::eFlagDebug;
#endif

	m_api = new dx11::DX11RenderApi(apicfg);

	return 0;
}

int CRenderModule::unloadApi()
{
	if (auto api = static_cast<dx11::DX11RenderApi*>(m_api))
	{
		delete api;
	}
	else
	{
		return 1;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/*
	Graphics module api loader
*/

#include <Windows.h>
#include "rendermodule.h"

//D3D11 interface
#include "API/DX11/DX11Render.h"

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////s

bool CRenderModule::loadApi(ERenderApiID id)
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
	apicfg.windowMode = m_config.windowMode;
	apicfg.windowHandle = m_config.windowHandle;
	apicfg.multisampling = m_config.multisampling;

#ifdef _DEBUG
	apicfg.flags |= ERenderApiFlags::eFlagDebug;
#endif

	m_api.reset(new dx11::DX11RenderApi(apicfg));

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
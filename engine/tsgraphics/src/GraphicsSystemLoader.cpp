/*
	Graphics module api loader
*/

#include <Windows.h>
#include <tsgraphics/graphicssystem.h>
#include <tscore/debug/log.h>

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////s

int GraphicsSystem::loadApi(ERenderApiID id)
{
	if (id != eRenderApiD3D11)
	{
		tswarn("API's other than direct3d 11 are not supported at the moment.");
		return false;
	}

	IAdapterFactory* adapterfactory = nullptr;
	abi::createAdapterFactory(&adapterfactory);

	for (uint32 i = 0; i < adapterfactory->getAdapterCount(); i++)
	{
		SRenderAdapterDesc desc;
		adapterfactory->enumAdapter(i, desc);

		tsinfo("Adapter(%): %", i, desc.adapterName.str());
	}

	abi::destroyAdapterFactory(adapterfactory);
	
	SRenderApiConfig apicfg;
	apicfg.adapterIndex = 0; //hard code the adapter for now
	apicfg.display.resolutionH = m_config.height;
	apicfg.display.resolutionW = m_config.width;
	apicfg.display.fullscreen = (m_config.displaymode == EDisplayMode::eDisplayFullscreen);
	apicfg.display.multisampleCount = m_config.multisampling.count;
	apicfg.windowHandle = m_config.windowHandle;

#ifdef _DEBUG
	apicfg.flags |= ERenderApiFlags::eFlagDebug;
#endif

	return abi::createRenderApi(&m_api, apicfg);
}

int GraphicsSystem::unloadApi()
{
	abi::destroyRenderApi(m_api);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
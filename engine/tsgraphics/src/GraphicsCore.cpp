/*
	Graphics module api loader
*/

#include <Windows.h>
#include <tsgraphics/graphicssystem.h>
#include <tscore/debug/log.h>

#include <tsgraphics/api/RenderApi.h>

using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////

void GraphicsCore::init(EGraphicsAPIID id, const SRenderApiConfig& config)
{
	if (id != eGraphicsAPI_D3D11)
	{
		tswarn("API's other than direct3d 11 are not supported at the moment.");
		return;
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

	abi::createRenderApi(&m_api, config);
	m_apiid = id;
}

void GraphicsCore::deinit()
{
	if (m_api)
	{
		abi::destroyRenderApi(m_api);
		m_api = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Graphics module source
*/

#include "rendermodule.h"
#include <tscore/debug/log.h>

using namespace std;
using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////s

CRenderModule::CRenderModule(const SRenderModuleConfiguration& cfg) :
	m_config(cfg),
	m_textureManager(this),
	m_shaderManager(this)
{
	if (!loadApi(cfg.apiEnum))
		tserror("Unable to load graphics api (ERenderApiID::eRenderApiD3D11)");

	m_textureManager.setRootpath(m_config.rootpath);

	Path sourcepath(m_config.rootpath);
	sourcepath.addDirectories("shaders");
	m_shaderManager.setSourcepath(sourcepath);

	setWindowDimensions(cfg.width, cfg.height);
}

CRenderModule::~CRenderModule()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////
//todo: make these methods threadsafe

void CRenderModule::setWindowMode(EWindowMode mode)
{
	m_api->setWindowMode(mode);
	m_config.windowMode = mode;
}

void CRenderModule::setWindowDimensions(uint32 w, uint32 h)
{
	m_api->setWindowDimensions(w, h);
	m_config.width = w;
	m_config.height = h;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void CRenderModule::drawBegin(const Vector& colour)
{
	m_api->drawBegin(colour);
}

void CRenderModule::drawEnd()
{
	m_api->drawEnd();
}

/////////////////////////////////////////////////////////////////////////////////////////////////
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
	m_shaderManager(this, 0)
{
	if (!loadApi(cfg.apiEnum))
		tserror("Unable to load graphics api (ERenderApiID::eRenderApiD3D11)");

	m_textureManager.setRootpath(m_config.rootpath);

	Path sourcepath(m_config.rootpath);
	sourcepath.addDirectories("shaders");
	m_shaderManager.setSourcepath(sourcepath);

	m_shaderManager.setFlags(m_shaderManager.getFlags() | eShaderManagerFlag_Debug);

	setWindowSettings(cfg.windowMode, cfg.width, cfg.height, cfg.multisampling);
}

CRenderModule::~CRenderModule()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////

void CRenderModule::setWindowSettings(EWindowMode mode, uint32 w, uint32 h, SMultisampling sampling)
{
	m_api->setWindowSettings(mode, w, h, sampling);

	if (sampling.count) m_config.multisampling = sampling;
	if (mode) m_config.windowMode = mode;
	if (w) m_config.width = w;
	if (h) m_config.height = h;
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
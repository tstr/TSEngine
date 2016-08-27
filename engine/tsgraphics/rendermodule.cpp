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
	m_shaderManager.setRootpath(m_config.rootpath);
}

CRenderModule::~CRenderModule()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////

void CRenderModule::setWindowMode(EWindowMode mode)
{
	m_api->setWindowMode(mode);
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
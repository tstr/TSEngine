/*
	Graphics module source
*/

#include <tsgraphics/rendermodule.h>
#include <tscore/debug/log.h>

#include "platform/borderless.h"

using namespace std;
using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////s

CRenderModule::CRenderModule(const SRenderModuleConfiguration& cfg) :
	m_config(cfg),
	m_textureManager(this),
	m_shaderManager(this, 0)
{
	if (int err = loadApi(cfg.apiEnum))
		tserror("Unable to load graphics api (id:%)(error:%)", cfg.apiEnum, err);

	m_textureManager.setRootpath(m_config.rootpath);

	Path sourcepath(m_config.rootpath);
	sourcepath.addDirectories("shaders");
	m_shaderManager.setSourcepath(sourcepath);

	m_shaderManager.setFlags(m_shaderManager.getFlags() | eShaderManagerFlag_Debug);
}

CRenderModule::~CRenderModule()
{
	unloadApi();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void CRenderModule::setDisplayConfiguration(EDisplayMode displaymode, uint32 w, uint32 h, SMultisampling sampling)
{
	//Update multisample count
	if (sampling.count)
	{
		if (m_config.multisampling.count != sampling.count)
		{
			m_api->setDisplayMultisampleCount(sampling.count);

			m_config.multisampling = sampling;
		}
	}

	//Update fullscreen state
	if (displaymode)
	{
		if (m_config.displaymode != displaymode)
		{
			intptr winhandle = m_config.windowHandle;

			if (displaymode == eDisplayFullscreen)
			{
				//Exit borderless if previous mode was borderless
				if (m_config.displaymode == eDisplayBorderless)
					exitBorderless(winhandle);

				//Enter fullscreen
				m_api->setDisplayFullscreenState(true);
			}
			else if (displaymode == eDisplayBorderless)
			{
				//Exit fullscreen if the previous mode was fullscreen
				if (m_config.displaymode == eDisplayFullscreen)
					m_api->setDisplayFullscreenState(false);
				
				//Enter borderless fullscreen
				enterBorderless(winhandle);
			}
			else if (displaymode == eDisplayWindowed) //Windowed mode
			{
				//Exit fullscreen if the previous mode was fullscreen
				if (m_config.displaymode == eDisplayFullscreen)
					m_api->setDisplayFullscreenState(false);
				//Exit borderless if previous mode was borderless
				else if (m_config.displaymode == eDisplayBorderless)
					exitBorderless(winhandle);

				//Do nothing
			}

			m_config.displaymode = displaymode;
		}
	}

	//Update resolution
	if (w || h)
	{
		w = (w) ? w : m_config.width;
		h = (h) ? h : m_config.height;

		if (m_config.width != w || m_config.height != h)
		{
			m_api->setDisplayResolution(w, h);

			m_config.width = w;
			m_config.height = h;
		}
	}
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
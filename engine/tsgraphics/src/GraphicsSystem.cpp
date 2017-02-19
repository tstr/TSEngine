/*
	Graphics module source
*/

#include <tsgraphics/graphicssystem.h>
#include <tscore/debug/log.h>
#include <tsgraphics/api/RenderApi.h>

#include "platform/borderless.h"

using namespace std;
using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////s

GraphicsSystem::GraphicsSystem(const SGraphicsSystemConfig& cfg) :
	m_config(cfg),
	m_textureManager(this)
{
	if (int err = loadApi(cfg.apiid))
		tserror("Unable to load graphics api (id:%)(error:%)", cfg.apiid, err);

	m_api->createContext(&m_context);

	m_textureManager.setRootpath(m_config.rootpath);

	Path sourcepath(m_config.rootpath);
	sourcepath.addDirectories("shaders/bin");

	m_shaderManager = CShaderManager(this, sourcepath, eShaderManagerFlag_Debug);
	m_meshManager = CMeshManager(this);
}

GraphicsSystem::~GraphicsSystem()
{
	m_api->destroyContext(m_context);
	m_context = nullptr;

	//Destroy all cached shaders
	m_shaderManager.clear();
	m_meshManager.clear();

	unloadApi();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void GraphicsSystem::setDisplayConfiguration(EDisplayMode displaymode, uint32 w, uint32 h, SMultisampling sampling)
{
	SDisplayConfig display;
	m_api->getDisplayConfiguration(display);

	//Update multisample count
	if (sampling.count)
	{
		if (m_config.multisampling.count != sampling.count)
		{
			m_config.multisampling = sampling;
			display.multisampleCount = sampling.count;
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
				display.fullscreen = true;
			}
			else if (displaymode == eDisplayBorderless)
			{
				//Exit fullscreen if the previous mode was fullscreen
				if (m_config.displaymode == eDisplayFullscreen)
					display.fullscreen = false;
				
				//Enter borderless fullscreen
				enterBorderless(winhandle);
			}
			else if (displaymode == eDisplayWindowed) //Windowed mode
			{
				//Exit fullscreen if the previous mode was fullscreen
				if (m_config.displaymode == eDisplayFullscreen)
					display.fullscreen = false;
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
			display.resolutionH = h;
			display.resolutionW = w;

			m_config.width = w;
			m_config.height = h;
		}
	}

	m_api->setDisplayConfiguration(display);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void GraphicsSystem::drawBegin(const Vector& colour)
{
	m_api->drawBegin(colour);
}

void GraphicsSystem::drawEnd()
{
	m_api->drawEnd(&m_context, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
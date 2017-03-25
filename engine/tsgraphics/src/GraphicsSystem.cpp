/*
	Main Graphics Subsystem source
*/

#include <tscore/debug/log.h>

#include <tsgraphics/GraphicsSystem.h>
#include <tsgraphics/GraphicsContext.h>
#include <tsgraphics/api/RenderApi.h>

#include "platform/borderless.h"

using namespace std;
using namespace ts;

/////////////////////////////////////////////////////////////////////////////////////////////////
//	Internal implementation
/////////////////////////////////////////////////////////////////////////////////////////////////
struct GraphicsSystem::System
{
	GraphicsSystem* system;
	SGraphicsSystemInfo systemInfo;

	CTextureManager textureManager;
	CShaderManager shaderManager;

	IRenderContext* context;

	System(GraphicsSystem* system, const SGraphicsSystemInfo& cfg) :
		system(system),
		systemInfo(cfg),
		context(nullptr)
	{

	}

	~System()
	{

	}
};

/////////////////////////////////////////////////////////////////////////////////////////////////

GraphicsSystem::GraphicsSystem(const SGraphicsSystemInfo& cfg)
{
	//Configure low level render api
	SRenderApiConfig apicfg;
	apicfg.adapterIndex = 0; //hard code the adapter for now
	apicfg.display.resolutionH = cfg.display.height;
	apicfg.display.resolutionW = cfg.display.width;
	apicfg.display.fullscreen = (cfg.display.mode == EDisplayMode::eDisplayFullscreen);
	apicfg.display.multisampleCount = cfg.display.multisampling.count;
	apicfg.windowHandle = cfg.windowHandle;

#ifdef _DEBUG
	apicfg.flags |= ERenderApiFlags::eFlagDebug;
#endif

	//Initialize low level render API
	this->init(cfg.apiid, apicfg);

	//Allocate internal implementation
	pSystem.reset(new System(this, cfg));

	//Initialize texture/shader/mesh managers
	
	//Shader files are located in a folder called shaderbin in the root asset directory
	Path sourcepath(cfg.rootpath);
	sourcepath.addDirectories("shaderbin");

	pSystem->textureManager = CTextureManager(this, cfg.rootpath);
	pSystem->shaderManager = CShaderManager(this, sourcepath, eShaderManagerFlag_Debug);

	//Create main render context
	getApi()->createContext(&pSystem->context);
}

GraphicsSystem::~GraphicsSystem()
{
	if (pSystem)
	{
		getApi()->destroyContext(pSystem->context);
		pSystem->context = nullptr;

		//Destroy all cached shaders
		pSystem->shaderManager.clear();
		pSystem->textureManager.clear();

		//Release all resources before deinitialization (causes memory leak otherwise)
		pSystem.reset();

		//Deinitialize low level render api
		this->deinit();
	}
}

//Enumerate list of available render adapters on this machine
void GraphicsSystem::getAdapterList(std::vector<SRenderAdapterDesc>& adapters)
{
	IAdapterFactory* adapterfactory = nullptr;
	abi::createAdapterFactory(&adapterfactory);

	adapters.clear();

	for (uint32 i = 0; i < adapterfactory->getAdapterCount(); i++)
	{
		SRenderAdapterDesc desc;
		adapterfactory->enumAdapter(i, desc);
		adapters.push_back(desc);
	}

	abi::destroyAdapterFactory(adapterfactory);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//	Manage display settings
/////////////////////////////////////////////////////////////////////////////////////////////////

void GraphicsSystem::getDisplayInfo(SGraphicsDisplayInfo& info)
{
	tsassert(pSystem);

	info = pSystem->systemInfo.display;
}

void GraphicsSystem::setDisplayInfo(const SGraphicsDisplayInfo& newInfo)
{
	tsassert(pSystem);

	auto& info = pSystem->systemInfo.display;

	const EDisplayMode displaymode = newInfo.mode;
	uint32 w = newInfo.width;
	uint32 h = newInfo.height;
	const SMultisampling sampling = newInfo.multisampling;

	SDisplayConfig displaycfg;
	getApi()->getDisplayConfiguration(displaycfg);

	//Update multisample count
	if (sampling.count)
	{
		if (info.multisampling.count != sampling.count)
		{
			info.multisampling = sampling;
			displaycfg.multisampleCount = sampling.count;
		}
	}

	//Update fullscreen state
	if (displaymode)
	{
		if (info.mode != displaymode)
		{
			intptr winhandle = pSystem->systemInfo.windowHandle;

			if (displaymode == eDisplayFullscreen)
			{
				//Exit borderless if previous mode was borderless
				if (info.mode == eDisplayBorderless)
					exitBorderless(winhandle);

				//Enter fullscreen
				displaycfg.fullscreen = true;
			}
			else if (displaymode == eDisplayBorderless)
			{
				//Exit fullscreen if the previous mode was fullscreen
				if (info.mode == eDisplayFullscreen)
					displaycfg.fullscreen = false;
				
				//Enter borderless fullscreen
				enterBorderless(winhandle);
			}
			else if (displaymode == eDisplayWindowed) //Windowed mode
			{
				//Exit fullscreen if the previous mode was fullscreen
				if (info.mode == eDisplayFullscreen)
					displaycfg.fullscreen = false;
				//Exit borderless if previous mode was borderless
				else if (info.mode == eDisplayBorderless)
					exitBorderless(winhandle);

				//Do nothing
			}

			info.mode = displaymode;
		}
	}

	//Update resolution
	if (w || h)
	{
		w = (w) ? w : info.width;
		h = (h) ? h : info.height;

		if (info.width != w || info.height != h)
		{
			displaycfg.resolutionH = h;
			displaycfg.resolutionW = w;

			info.width = w;
			info.height = h;
		}
	}

	getApi()->setDisplayConfiguration(displaycfg);
}

intptr GraphicsSystem::getDisplayHandle() const
{
	return pSystem->systemInfo.windowHandle;
}

Path GraphicsSystem::getRootPath() const
{
	return pSystem->systemInfo.rootpath;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Get resource managers
/////////////////////////////////////////////////////////////////////////////////////////////////

CTextureManager* GraphicsSystem::getTextureManager()
{
	return &pSystem->textureManager;
}

CShaderManager* GraphicsSystem::getShaderManager()
{
	return &pSystem->shaderManager;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void GraphicsSystem::execute(GraphicsContext* context)
{
	tsassert(pSystem);

	IRenderContext* rc = pSystem->context;

	getApi()->drawBegin();

	HTarget display = HTARGET_NULL;
	getApi()->getDisplayTarget(display);

	rc->clearRenderTarget(display, (const Vector&)colours::Azure);
	rc->clearDepthTarget(display, 1.0f);

	if (CommandQueue* queue = context->renderFrame(display))
	{
		queue->flush(rc);
	}

	rc->finish();

	getApi()->drawEnd(&rc, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

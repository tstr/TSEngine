/*
	Main Graphics Subsystem source

	todo: fix resizing display in fullscreen mode
*/

#include <tscore/debug/log.h>

#include <mutex>
#include <atomic>

#include <tsgraphics/GraphicsSystem.h>
#include <tsgraphics/GraphicsContext.h>
#include <tsgraphics/api/RenderApi.h>

using namespace std;
using namespace ts;

typedef recursive_mutex Lock;
typedef lock_guard<Lock> Guard;

/////////////////////////////////////////////////////////////////////////////////////////////////
//	Internal implementation
/////////////////////////////////////////////////////////////////////////////////////////////////
struct GraphicsSystem::System : public GraphicsConfig
{
	GraphicsSystem* system;

	//Primary rendering context
	IRenderContext* context;

	Lock displayLock;
	atomic<bool> displayNeedRebuild;

	/*
		Construct system
	*/
	System(GraphicsSystem* system, const GraphicsConfig& cfg) :
		system(system),
		context(nullptr),
		GraphicsConfig(cfg)
	{}

	~System() {}


	void tryRebuildDisplay();
	void doRebuildDisplay();
};

/////////////////////////////////////////////////////////////////////////////////////////////////

GraphicsSystem::GraphicsSystem(const GraphicsConfig& cfg) :
	pSystem(new System(this, cfg))
{
	//Configure low level render api
	SRenderApiConfig apicfg;
	apicfg.adapterIndex = 0; //hard code the adapter for now
	apicfg.display.resolutionH = cfg.display.height;
	apicfg.display.resolutionW = cfg.display.width;
	apicfg.display.fullscreen = (cfg.display.mode == EDisplayMode::eDisplayFullscreen);
	apicfg.display.multisampleLevel = cfg.display.multisampleLevel;
	apicfg.windowHandle = pSystem->surface->getHandle();

#ifdef _DEBUG
	apicfg.flags |= ERenderApiFlags::eFlagDebug;
#endif

	//Initialize low level render API
	this->init(cfg.apiid, apicfg);

	//If desired display mode is borderless, ISurface::enableBorderless() must be called manually
	if (cfg.display.mode == eDisplayBorderless)
	{
		cfg.surface->enableBorderless(true);
		//Refreshing the display forces the display to resize
		this->refreshDisplay();
	}

	if (cfg.display.mode == eDisplayFullscreen)
	{
		this->refreshDisplay();
	}

	//Create main render context
	getApi()->createContext(&pSystem->context);
}

GraphicsSystem::~GraphicsSystem()
{
	if (pSystem)
	{
		getApi()->destroyContext(pSystem->context);
		pSystem->context = nullptr;

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
// Manage display settings
/////////////////////////////////////////////////////////////////////////////////////////////////

bool GraphicsSystem::setDisplayResolution(uint w, uint h)
{
	tsassert(pSystem);

	Guard g(pSystem->displayLock);

	pSystem->display.width = w;
	pSystem->display.height = h;

	pSystem->displayNeedRebuild.store(true);

	return true;
}

void GraphicsSystem::refreshDisplay()
{
	tsassert(pSystem);

	Guard g(pSystem->displayLock);

	pSystem->surface->getSize(
		pSystem->display.width,
		pSystem->display.height
	);

	pSystem->displayNeedRebuild.store(true);
}

bool GraphicsSystem::setDisplayMultisamplingLevel(uint level)
{
	tsassert(pSystem);

	Guard g(pSystem->displayLock);

	pSystem->display.multisampleLevel = level;

	pSystem->displayNeedRebuild.store(true);

	return true;
}

bool GraphicsSystem::setDisplayMode(EDisplayMode mode)
{
	tsassert(pSystem);

	Guard g(pSystem->displayLock);

	pSystem->display.mode = mode;

	pSystem->displayNeedRebuild.store(true);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void GraphicsSystem::System::tryRebuildDisplay()
{
	if (displayNeedRebuild.load())
	{
		Guard g(displayLock);

		doRebuildDisplay();

		displayNeedRebuild.store(false);
	}
}

void GraphicsSystem::System::doRebuildDisplay()
{
	SDisplayConfig config;
	system->getApi()->getDisplayConfiguration(config);

	/*
		Change Multisample level
	*/
	if (config.multisampleLevel != display.multisampleLevel)
	{
		config.multisampleLevel = display.multisampleLevel;
		system->getApi()->setDisplayConfiguration(config);
	}

	/*
		Change display mode
	*/
	{
		EDisplayMode curMode;

		//Get current display mode

		if (config.fullscreen)
			curMode = eDisplayFullscreen;
		else if (surface->isBorderless())
			curMode = eDisplayBorderless;
		else
			curMode = eDisplayWindowed;

		//Desired display mode
		EDisplayMode& mode = display.mode;

		//While current display mode is not the desired display mode
		while (curMode != mode)
		{
			switch (curMode)
			{
				case eDisplayWindowed:
				{
					if (mode == eDisplayBorderless)
					{
						//Enter borderless
						surface->enableBorderless(true);
					}
					else if (mode == eDisplayFullscreen)
					{
						//Enter fullscreen
						config.fullscreen = true;
						system->getApi()->setDisplayConfiguration(config);
					}

					curMode = mode;
					break;
				}

				case eDisplayBorderless:
				{
					curMode = eDisplayWindowed;

					//Exit borderless
					surface->enableBorderless(false);
					break;
				}

				case eDisplayFullscreen:
				{
					curMode = eDisplayWindowed;

					//Exit fullscreen
					config.fullscreen = false;
					system->getApi()->setDisplayConfiguration(config);
					break;
				}
			}
		}
	}

	/*
		Change resolution
	*/
	if ((config.resolutionH != display.height) || (config.resolutionW != display.width))
	{
		if (display.mode == eDisplayWindowed)
		{
			uint curW = 0;
			uint curH = 0;
			surface->getSize(curW, curH);
			
			if (curH != display.height || curW != display.width)
			{
				surface->resize(display.width, display.height);
			}
		}

		config.resolutionW = display.width;
		config.resolutionH = display.height;
		config.multisampleLevel = 0;
		config.fullscreen = display.mode == eDisplayFullscreen;

		system->getApi()->setDisplayConfiguration(config);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void GraphicsSystem::getDisplayOptions(GraphicsDisplayOptions& opt)
{
	tsassert(pSystem);

	opt = pSystem->display;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Get properties
/////////////////////////////////////////////////////////////////////////////////////////////////

Path GraphicsSystem::getRootPath() const
{
	return pSystem->rootpath;
}

HTarget GraphicsSystem::getDisplayTarget() const
{
	HTarget target;
	getApi()->getDisplayTarget(target);
	return target;
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void GraphicsSystem::begin()
{
	tsassert(pSystem);

	//Rebuild the display if changes have been made
	pSystem->tryRebuildDisplay();

	//Main context
	IRenderContext* rc = pSystem->context;

	getApi()->drawBegin();

	HTarget display = HTARGET_NULL;
	getApi()->getDisplayTarget(display);

	rc->clearRenderTarget(display, (const Vector&)colours::Azure);
	rc->clearDepthTarget(display, 1.0f);
}

void GraphicsSystem::end()
{
	tsassert(pSystem);

	//Main context
	IRenderContext* rc = pSystem->context;

	rc->finish();

	getApi()->drawEnd(&rc, 1);
}

void GraphicsSystem::execute(CommandQueue* queue)
{
	tsassert(pSystem);

	queue->flush(pSystem->context);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

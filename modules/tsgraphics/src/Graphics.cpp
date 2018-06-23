/*
	Main Graphics Subsystem source

	todo: fix resizing display in fullscreen mode
*/

#include <tscore/debug/log.h>
#include <tscore/debug/assert.h>

#include <mutex>
#include <atomic>

#include <tsgraphics/Graphics.h>
#include <tsgraphics/Driver.h>

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
	RenderContext* context;

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
	RenderDeviceConfig devcfg;
	devcfg.adapterIndex = 0; //hard code the adapter for now
	devcfg.display.resolutionH = cfg.display.height;
	devcfg.display.resolutionW = cfg.display.width;
	devcfg.display.fullscreen = (cfg.display.mode == EDisplayMode::eDisplayFullscreen);
	devcfg.display.multisampleLevel = cfg.display.multisampleLevel;
	devcfg.windowHandle = pSystem->surface->getHandle();

#ifdef _DEBUG
	apicfg.flags |= ERenderApiFlags::eFlagDebug;
#endif

	pDevice = RenderDevice::create(RenderDeviceID::D3D11, devcfg);

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
	pSystem->context = pDevice->context();
}

GraphicsSystem::~GraphicsSystem()
{
	//Release all resources before deinitialization (causes memory leak otherwise)
	pSystem.reset();
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
	DisplayConfig config;
	system->device()->getDisplayConfiguration(config);

	/*
		Change Multisample level
	*/
	if (config.multisampleLevel != display.multisampleLevel)
	{
		config.multisampleLevel = display.multisampleLevel;
		system->device()->setDisplayConfiguration(config);
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
						system->device()->setDisplayConfiguration(config);
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
					system->device()->setDisplayConfiguration(config);
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

		system->device()->setDisplayConfiguration(config);
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

/////////////////////////////////////////////////////////////////////////////////////////////////

void GraphicsSystem::begin()
{
	tsassert(pSystem);

	//Rebuild the display if changes have been made
	pSystem->tryRebuildDisplay();

	//Main context
	RenderContext* rc = pSystem->context;
}

void GraphicsSystem::end()
{
	tsassert(pSystem);

	//Main context
	RenderContext* rc = pSystem->context;

	rc->finish();
	device()->commit();
}

void GraphicsSystem::execute(CommandQueue* queue)
{
	tsassert(pSystem);

	queue->flush(pSystem->context);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

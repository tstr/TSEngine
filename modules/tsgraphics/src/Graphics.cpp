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

using namespace ts;

typedef std::recursive_mutex Lock;
typedef std::lock_guard<Lock> Guard;

/////////////////////////////////////////////////////////////////////////////////////////////////
//	Internal implementation
/////////////////////////////////////////////////////////////////////////////////////////////////
struct GraphicsSystem::System : public GraphicsConfig
{
	GraphicsSystem* system = nullptr;
	//Primary rendering context
	RenderContext* context = nullptr;
	
	//Render target pool
	ImageTargetPool displayTargets;

	Lock displayLock;
	std::atomic<bool> displayNeedRebuild;

	/*
		Construct system
	*/
	System(GraphicsSystem* system, const GraphicsConfig& cfg) :
		system(system),
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
	devcfg.display.fullscreen = (cfg.display.mode == DisplayMode::FULLSCREEN);
	devcfg.display.multisampleLevel = cfg.display.multisampleLevel;
	devcfg.windowHandle = pSystem->surface->getHandle();

#ifdef _DEBUG
	devcfg.flags |= RenderDeviceConfig::DEBUG;
#endif

	pDevice = RenderDevice::create(RenderDriverID::DX11, devcfg);

	//If desired display mode is borderless, ISurface::enableBorderless() must be called manually
	if (cfg.display.mode == DisplayMode::BORDERLESS)
	{
		cfg.surface->enableBorderless(true);
		//Refreshing the display forces the display to resize
		this->refreshDisplay();
	}

	if (cfg.display.mode == DisplayMode::FULLSCREEN)
	{
		this->refreshDisplay();
	}

	//Create main render context
	pSystem->context = pDevice->context();

	//Prepare image target pool
	pSystem->displayTargets = ImageTargetPool(device(), cfg.display.width, cfg.display.height, cfg.display.multisampleLevel);

	//Register display change signal handler
	onDisplayChange += DisplayEvent::CallbackType::fromMethod<ImageTargetPool, &ImageTargetPool::resize>(getDisplayTargetPool());
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

bool GraphicsSystem::setDisplayMode(DisplayMode mode)
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
		system->onDisplayChange(config);
	}

	/*
		Change display mode
	*/
	{
		DisplayMode curMode;

		//Get current display mode

		if (config.fullscreen)
			curMode = DisplayMode::FULLSCREEN;
		else if (surface->isBorderless())
			curMode = DisplayMode::BORDERLESS;
		else
			curMode = DisplayMode::WINDOWED;

		//Desired display mode
		DisplayMode& mode = display.mode;

		//While current display mode is not the desired display mode
		while (curMode != mode)
		{
			switch (curMode)
			{
				case DisplayMode::WINDOWED:
				{
					if (mode == DisplayMode::BORDERLESS)
					{
						//Enter borderless
						surface->enableBorderless(true);
					}
					else if (mode == DisplayMode::FULLSCREEN)
					{
						//Enter fullscreen
						config.fullscreen = true;
						system->device()->setDisplayConfiguration(config);
						system->onDisplayChange(config);
					}

					curMode = mode;
					break;
				}

				case DisplayMode::BORDERLESS:
				{
					curMode = DisplayMode::WINDOWED;

					//Exit borderless
					surface->enableBorderless(false);
					break;
				}

				case DisplayMode::FULLSCREEN:
				{
					curMode = DisplayMode::WINDOWED;

					//Exit fullscreen
					config.fullscreen = false;
					system->device()->setDisplayConfiguration(config);
					system->onDisplayChange(config);
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
		if (display.mode == DisplayMode::WINDOWED)
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
		config.fullscreen = display.mode == DisplayMode::FULLSCREEN;

		system->device()->setDisplayConfiguration(config);
		system->onDisplayChange(config);
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

ImageTargetPool* GraphicsSystem::getDisplayTargetPool() const
{
	return &pSystem->displayTargets;
}

ImageView GraphicsSystem::getDisplayView() const
{
	ImageView view;
	view.image = device()->getDisplayTarget();
	view.count = 1;
	view.index = 0;
	view.type = ImageType::_2D;

	return view;
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

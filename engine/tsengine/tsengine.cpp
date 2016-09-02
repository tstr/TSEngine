/*
	Engine definition
*/

#include <tsengine.h>
#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>
#include <tscore/system/info.h>
#include <tscore/system/thread.h>
#include <tsgraphics/colour.h>

//Modules
#include <tsgraphics/rendermodule.h>
#include <tsengine/input/inputmodule.h>

#include "platform/window.h"
#include "platform/console.h"

#include "event/messenger.h"
#include "cmdargs.h"
#include "configfile.h"

using namespace ts;
using namespace std;

/*
//todo: find a way to set program to exit correctly when console is closed
static void consoleClosingHandlerFunc()
{
	gSystem->deinit();
}
*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Application window class
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Window : public CWindow
{
private:

	CEngineSystem* m_pSystem = nullptr;

	//Window event listener
	struct CEventListener : public IEventListener
	{
		Window* m_wnd = nullptr;

		CEventListener(Window* wnd) :
			m_wnd(wnd)
		{}

		int onWindowEvent(const SWindowEventArgs& args) override
		{
			if (args.eventcode == EWindowEvent::eEventDestroy)
			{
				m_wnd->m_pSystem->shutdown();
				//return IEventListener::eHandled;
				return 0;
			}

			auto input = m_wnd->m_pSystem->getInputModule();
			if (input != nullptr)
			{
				input->onWindowInputEvent(args);
			}

			return 0;
		}
	} m_eventListener;

public:

	Window(CEngineSystem* sys, const SWindowDesc& desc) :
		m_pSystem(sys),
		m_eventListener(this),
		CWindow(desc)
	{
		this->addEventListener((IEventListener*)&m_eventListener);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Engine initialization
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CEngineSystem::CEngineSystem(const SEngineStartupParams& params)
{
	//Parse command line arguments
	CommandLineArgs args(params.commandArgs);

	//Console initialization
	if (!args.isArgumentTag("noconsole"))
	{
		consoleOpen();
		//setConsoleClosingHandler(consoleClosingHandlerFunc);
	}

	//Config loader
	Path cfgpath(params.appPath.getParent());
	cfgpath.addDirectories((args.isArgumentTag("config")) ? args.getArgumentValue("config") : "config.ini");
	ConfigFile config(cfgpath);

	//Set application instance
	m_app.reset(params.app);
	tsassert(m_app.get());

	//Set application window parameters
	SDisplayInfo dispinf;
	getPrimaryDisplayInformation(dispinf);
	uint32 width = 800;
	uint32 height = 600;
	config.getProperty("video.resolutionW", width);
	config.getProperty("video.resolutionH", height);

	SWindowDesc windesc;
	windesc.title = params.appPath.str();
	windesc.rect.x = (dispinf.width - width) / 2;
	windesc.rect.y = (dispinf.height - height) / 2;
	windesc.rect.w = width;
	windesc.rect.h = height;
	windesc.appInstance = params.appInstance;
	
	//Create application window object
	m_window.reset(new Window(this, windesc));

	//Runs window message loop on separate thread
	const int showcmd = params.showWindow;
	thread([this, &showcmd]() { this->m_window->open(showcmd); }).detach();
	//Delay this thread until the window has opened fully
	while (!m_window->isOpen())
	{
		this_thread::sleep_for(chrono::milliseconds(1));
		this_thread::yield();
	}

	uint32 fullscreenmode = 0;
	config.getProperty("video.fullscreen", fullscreenmode);
	if (fullscreenmode > 2)
	{
		tswarn("% is not a valid fullscreen mode", fullscreenmode);
		fullscreenmode = 0;
	}

	string assetpathbuf;
	config.getProperty("system.assetdir", assetpathbuf);
	Path assetpath = params.appPath.getParent();
	assetpath.addDirectories(assetpathbuf);

	uint32 samplecount = 1;
	config.getProperty("video.multisamplecount", samplecount);

	SRenderModuleConfiguration rendercfg;
	rendercfg.windowHandle = m_window->handle();
	rendercfg.width = width;
	rendercfg.height = height;
	rendercfg.apiEnum = ERenderApiID::eRenderApiD3D11;
	rendercfg.windowMode = (EWindowMode)fullscreenmode;
	rendercfg.rootpath = assetpath;
	rendercfg.multisampling.count = samplecount;
	m_renderModule.reset(new CRenderModule(rendercfg));

	m_inputModule.reset(new CInputModule(m_window.get()));

	//Close application from console
	thread([this] {
		while (true)
		{
			std::string str;
			std::cin >> str;
			if (compare_string_weak(str, "exit"))
				this->shutdown();
		}
	}).detach();

	onInit();

	//Run simulation loop
	run();

	onExit();
}

CEngineSystem::~CEngineSystem()
{
	shutdown();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CEngineSystem::run()
{
	lock_guard<mutex>lk(m_exitMutex);

	Timer timer;
	double pheta = 0.0f;
	Vector framecolour;
	framecolour.x() = 0.0f;
	framecolour.y() = 1.0f;

	ESystemMessage msg = ESystemMessage::eMessageNull;

	//Main engine loop
	while (msg != ESystemMessage::eMessageExit)
	{
		m_messageReciever.peek(msg);

		timer.tick();
		double dt = timer.deltaTime();
		pheta += (2 * Pi) * dt / 5;
		pheta = fmod(pheta, 2 * Pi);
		framecolour.x() = 0.5f + (float)sin(pheta);
		framecolour.y() = 0.5f - (float)sin(pheta);
		framecolour.z() = 0.5f + (float)cos(pheta);

		framecolour = colours::AntiqueWhite;

		//Clear the frame to a specific colour
		m_renderModule->drawBegin(framecolour);

		//rendering here
		m_app->onUpdate(dt);

		//Present the frame
		m_renderModule->drawEnd();
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CEngineSystem::shutdown()
{
	m_messageReciever.post(ESystemMessage::eMessageExit);
	lock_guard<mutex>lk(m_exitMutex);
}

//Event handlers
void CEngineSystem::onExit()
{
	m_app->onExit();

	m_window->close();

	m_app.reset();
	m_inputModule.reset();
	m_renderModule.reset();

	consoleClose();
}

void CEngineSystem::onInit()
{
	m_app->onInit(this);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
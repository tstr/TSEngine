/*
	Engine definition
*/

#include <tsengine.h>
#include <tscore/platform/window.h>
#include <tscore/platform/console.h>
#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>
#include <tscore/system/info.h>
#include <tscore/system/thread.h>
#include <tsgraphics/rendermodule.h>

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

		int onEvent(const SWindowEventArgs& args) override
		{
			switch (args.eventcode)
			{
			case (EWindowEvent::eEventDestroy) :
				m_wnd->m_pSystem->shutdown();
				break;
			case (EWindowEvent::eEventKeydown) :
				tsinfo("KeyDown Event (A:%) (B:%)", args.a, args.b);
				if (args.b == VK_ESCAPE)
					m_wnd->close();
				break;
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
		this->setEventListener((IEventListener*)&m_eventListener);
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
	cfgpath.addDirectories("config.ini");
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
	tsassert(fullscreenmode <= 2);

	SRenderModuleConfiguration rendercfg;
	rendercfg.windowHandle = m_window->handle();
	rendercfg.width = width;
	rendercfg.height = height;
	rendercfg.apiEnum = ERenderApiID::eRenderApiD3D11;
	rendercfg.windowMode = (EWindowMode)fullscreenmode;
	m_moduleRender.reset(new CRenderModule(rendercfg));

	//Close application from console
	thread([this] {
		while (true)
		{
			std::string str;
			std::cin >> str;
			if (str == "exit")
				this->shutdown();
		}
	}).detach();

	onInit();

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

		//Clear the frame to a specific colour
		m_moduleRender->drawBegin(framecolour);

		//rendering here

		//Present the frame
		m_moduleRender->drawEnd();
	}

	onExit();
}

CEngineSystem::~CEngineSystem()
{
	shutdown();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CEngineSystem::run()
{



	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CEngineSystem::shutdown()
{
	m_messageReciever.post(ESystemMessage::eMessageExit);
}

//Event handlers
void CEngineSystem::onExit()
{
	m_app->onExit();

	m_window->close();

	consoleClose();
}

void CEngineSystem::onInit()
{
	m_app->onInit();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
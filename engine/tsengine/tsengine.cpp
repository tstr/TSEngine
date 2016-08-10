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
#include <tsgraphics/rendermodule.h> //Graphics

#include "event/messenger.h"
#include "cmdargs.h"

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
				tsinfo("KeyDown Event (A:%) (B:%)", args.a, (char)args.b);
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

	//Set application instance
	m_app.reset(params.app);
	tsassert(m_app.get());

	//Set application window parameters
	SDisplayInfo dispinf;
	getPrimaryDisplayInformation(dispinf);
	uint32 width = 1280;
	uint32 height = 720;

	SWindowDesc windesc;
	windesc.title = params.appPath;
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
	while (!m_window->isOpen())
	{
		this_thread::sleep_for(chrono::milliseconds(2));
	}


	SRenderModuleConfiguration rendercfg;
	rendercfg.targethandle = m_window->handle();
	m_moduleRender.reset(new CRenderModule(rendercfg));

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

	//Main engine loop
	while (m_enabled)
	{
		this_thread::sleep_for(chrono::milliseconds(10));
		this_thread::yield();
	}

	onExit();
}

CEngineSystem::~CEngineSystem()
{
	shutdown();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CEngineSystem::shutdown()
{
	m_enabled = false;
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
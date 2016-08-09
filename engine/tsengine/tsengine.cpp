/*
	Engine definition
*/

#include <tsengine.h>
#include <tscore/platform/window.h>
#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>
#include <tscore/system/info.h>
#include <tscore/system/thread.h>
#include <tscore/platform/console.h>
#include "event/messenger.h"

#include "cmdargs.h"

#include <iostream>

using namespace ts;

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
			case (EWindowEvent::eEventClose) :
				m_wnd->m_pSystem->shutdown();
				break;
			}

			return 0;
		}
	};

	CEventListener m_eventListener;

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

CEngineSystem::CEngineSystem(const SEngineStartupParams& params)
{
	//Set window parameter struct
	SWindowDesc windesc;
	windesc.title = params.appPath;
	windesc.rect.x = 0;
	windesc.rect.y = 0;
	windesc.rect.w = 1280;
	windesc.rect.h = 720;
	windesc.appInstance = params.appInstance;

	//Set application instance and application window members
	m_pWindow.reset(new Window(this, windesc));
	m_pApp.reset(params.app);
	tsassert(m_pApp.get());

	CommandLineArgs args(params.commandArgs);

	//Console initialization
	if (!args.isArgumentTag("noconsole"))
	{
		consoleOpen();
		//setConsoleClosingHandler(consoleClosingHandlerFunc);
	}

	//Runs window message loop on separate thread
	thread([this, &params]() { this->m_pWindow->open(params.showWindow); }).detach();

	//Test
	thread([this]() {
		while (true)
		{
			std::string buf;
			std::cin >> buf;
			if (compare_string_weak(buf, "exit"))
			{
				this->shutdown();
				break;
			}
		}
	}).detach();


	onInit();

	while (true)
	{
		SEngineMessage msg;
		m_reciever.get(msg);

		if (msg.code == eEngineMessageShutdown)
			break;
	}
}

CEngineSystem::~CEngineSystem()
{
	shutdown();

	onDeinit();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CEngineSystem::shutdown()
{
	m_reciever.post(SEngineMessage(eEngineMessageShutdown));
}

//Event handlers
void CEngineSystem::onDeinit()
{
	m_pApp->onDeinit();

	if (m_pWindow->isOpen())
		m_pWindow->close();

	consoleClose();
}

void CEngineSystem::onInit()
{
	m_pApp->onInit();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
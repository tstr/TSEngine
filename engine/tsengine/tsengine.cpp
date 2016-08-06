/*
	Engine definition
*/

#include <tsengine.h>
#include <tscore/platform/Window.h>
#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>
#include <tscore/system/info.h>

#include <tscore/platform/console.h>
#include "event/messenger.h"

#include "cmdargs.h"

#include <iostream>

using namespace ts;

namespace ts
{
	static byte _systemblock[sizeof(CEngineSystem)];
	CEngineSystem* const gSystem = new(_systemblock) CEngineSystem;
}

/*
//todo: find a way to set program to exit correctly when console is closed
static void consoleClosingHandlerFunc()
{
	gSystem->deinit();
}
*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MainWindow : public Window
{
private:

public:

	MainWindow() : Window("application") {}

	void onCreate(WindowEventArgs args) override
	{
		SSystemInfo inf;
		getSystemInformation(inf);
		
		tsprint((std::string)"Hello " + inf.userName);

		Window::onCreate(args);
	}

	void onClose(WindowEventArgs args) override
	{
		gSystem->deinit();
	}

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CEngineSystem::init(const SEngineStartupParams& params)
{
	m_pWindow = new MainWindow();
	m_pApp = params.app;

	CommandLineArgs args(params.commandArgs);

	if (!args.isArgumentTag("noconsole"))
	{
		consoleOpen();
		//setConsoleClosingHandler(consoleClosingHandlerFunc);
	}

	tsassert(m_pApp);

	WindowRect rect;
	rect.w = 1280;
	rect.h = 720;

	onInit();

	m_pWindow->createAsync(rect);

	SEngineMessage msg;
	do
	{
		m_reciever.get(msg);
	}
	while (msg.code != EEngineMessageCode::eEngineMessageDeinit);

	onDeinit();
}

void CEngineSystem::deinit()
{
	m_reciever.post(SEngineMessage(eEngineMessageDeinit));
}

//Event handlers
void CEngineSystem::onDeinit()
{
	m_pApp->onDeinit();

	if (m_pWindow->isOpen())
		m_pWindow->close();

	consoleClose();

	delete m_pApp;
	delete m_pWindow;
}

void CEngineSystem::onInit()
{
	m_pApp->onInit();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CEngineSystem::CEngineSystem()
{

}

CEngineSystem::~CEngineSystem()
{
	//deinit();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
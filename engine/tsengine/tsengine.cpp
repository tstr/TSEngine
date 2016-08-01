/*
	Engine definition
*/

#include <tsengine.h>
#include <tscore/platform/Window.h>
#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>
#include <tscore/system/info.h>

#include <tscore/platform/console.h>

#include "cmdargs.h"

#include <iostream>

using namespace ts;

namespace ts
{
	static byte _systemblock[sizeof(CEngineSystem)];
	CEngineSystem* const gSystem = new(_systemblock) CEngineSystem;
}

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
		gSystem->shutdown();
	}

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CEngineSystem::init(const SEngineStartupParams& params)
{
	m_pWindow = new MainWindow();
	m_pApp = params.app;

	tsassert(m_pApp);

	WindowRect rect;
	rect.w = 1280;
	rect.h = 720;

	CommandLineArgs args(params.commandArgs);

	if (!args.isArgumentTag("noconsole"))
	{
		consoleOpen();
	}

	onInit();

	//run main loop
	m_pWindow->create(rect);

	onShutdown();
}

void CEngineSystem::shutdown()
{
	
}

//Event handlers
void CEngineSystem::onShutdown()
{
	m_pApp->onShutdown();

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
	shutdown();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
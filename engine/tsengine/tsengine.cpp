/*
	Engine definition
*/

#include <tsengine.h>
#include <tscore/platform/Window.h>
#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>
#include <tscore/system/info.h>
#include <tscore/cmdargs.h>

#include <iostream>

using namespace ts;

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
		
		tslog((std::string)"Hello " + inf.userName, eLevelDebug);

		Window::onCreate(args);
	}

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CApplicationCore::CApplicationCore(const char* cmdline)
{
	m_window = (Window*)new MainWindow;

	WindowRect rect;
	rect.w = 1280;
	rect.h = 720;

	CommandLineArgs args(cmdline);

	onInit();

	m_window->create(rect);
}

CApplicationCore::~CApplicationCore()
{
	onShutdown();

	if (m_window->isOpen())
		m_window->close();

	delete m_window;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
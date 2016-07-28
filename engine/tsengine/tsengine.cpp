/*
	Engine definition
*/

#include <tscore/platform/window.h>
#include <tsengine.h>

using namespace ts;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MainWindow : public Window
{
private:

public:

	MainWindow() :
		Window("application")
	{}

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ApplicationCore::ApplicationCore(const char* cmdline)
{
	m_window = (Window*)new MainWindow;

	WindowRect rect;
	rect.w = 1280;
	rect.h = 720;

	onInit();

	m_window->create(rect);
}

ApplicationCore::~ApplicationCore()
{
	onShutdown();

	if (m_window->isOpen())
		m_window->close();

	delete m_window;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
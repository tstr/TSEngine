/*
	Engine definition
*/

#include <tscore/platform/window.h>
#include <tsengine.h>

using namespace ts;
using namespace ts::core;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class MainWindow : public core::Window
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

	((core::Window*)m_window)->create(rect);
}

ApplicationCore::~ApplicationCore()
{
	onShutdown();

	if (((core::Window*)m_window)->isOpen())
		((core::Window*)m_window)->close();

	delete m_window;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
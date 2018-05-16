/*
	Engine environment source
*/

#include <tsengine/App.h>
#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>
#include <tscore/system/thread.h>
#include <tscore/path.h>
#include <tscore/pathutil.h>
#include <tsgraphics/colour.h>

//Subsystems
#include <tsgraphics/GraphicsSystem.h>
#include <tsengine/Input.h>

#include "platform/Window.h"
#include "platform/console.h"

#include "cmdargs.h"
#include "INIReader.h"

using namespace ts;
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Application window class
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class EngineWindow : public Window
{
private:

	Application& m_app;
	atomic<int> m_exitCode;

	/*
		Window event handlers
	*/

	void onResize() override
	{
		if (auto gfx = m_app.graphics())
			gfx->refreshDisplay();
	}

	void onActivate() override
	{
		if (auto gfx = m_app.graphics())
			gfx->refreshDisplay();
	}

	void onDestroy() override
	{

	}

	void onEvent(const PlatformEventArgs& arg) override
	{
		m_app.input()->onEvent(arg);
	}

public:

	EngineWindow(Application& app, const WindowInfo& desc) :
		m_app(app),
		m_exitCode(0),
		Window(desc)
	{
	}

	~EngineWindow()
	{
	}

	Application& getSystem() { return m_app; }

	int getExitCode() const
	{
		return m_exitCode.load();
	}

	void setExitCode(int code)
	{
		m_exitCode.store(code);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Engine initialization
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Application::Application(int argc, char** argv)
{
	/////////////////////////////////////////////////////////////////////////
	// Initialize error handlers
	
	initErrorHandler();
	
	/////////////////////////////////////////////////////////////////////////
	// Parse command line arguments and load config
	
	CommandLineArgs args(argv, argc);

	//Console initialization
	if (!args.isArgumentTag("noconsole"))
	{
		consoleOpen();
		//setConsoleClosingHandler(consoleClosingHandlerFunc);
	}
	
	//Determine path of config file
	Path cfgpath(getCurrentDir());
	
	if (args.isArgumentTag("config"))
	{
		cfgpath.addDirectories(args.getArgumentValue("config"));
	}
	else
	{
		cfgpath.addDirectories("config.ini");
	}

	initConfig(cfgpath);

	/////////////////////////////////////////////////////////////////////////
	
	//Set application window parameters
	SDisplayInfo dispinf;
	getSystemDisplayInfo(dispinf);
	uint32 width = 800;
	uint32 height = 600;
	m_vars->get("video.resolutionW", width);
	m_vars->get("video.resolutionH", height);

	WindowInfo winInf;
	winInf.title = format("%  ::::  %", cfgpath.str(), args.getArguments());
	winInf.rect.x = (dispinf.width - width) / 2;
	winInf.rect.y = (dispinf.height - height) / 2;
	winInf.rect.w = width;
	winInf.rect.h = height;
	
	//Create application window object
	m_window.reset(new EngineWindow(*this, winInf));

	//Runs window message loop on separate thread
	m_window->open();
	
	uint32 displaymode = 0;
	m_vars->get("video.displaymode", displaymode);
	if (displaymode > 2)
	{
		tswarn("% is not a valid fullscreen mode", displaymode);
		displaymode = 0;
	}

	//Resolve asset path
	Path assetpath = cfgpath.getParent();

	string assetpathbuf;
	m_vars->get("system.assetdir", assetpathbuf);

	if (isAbsolutePath(assetpathbuf))
	{
		assetpath = assetpathbuf;
	}
	else
	{
		assetpath.addDirectories(assetpathbuf);
	}

	uint32 samplecount = 1;
	m_vars->get("video.multisamplecount", samplecount);

	GraphicsConfig gcfg;
	gcfg.surface = static_cast<ISurface*>(m_window.get());
	gcfg.display.width = width;
	gcfg.display.height = height;
	gcfg.display.mode = (EDisplayMode)displaymode;
	gcfg.display.multisampleLevel = samplecount;
	gcfg.apiid = EGraphicsAPIID::eGraphicsAPI_D3D11;
	gcfg.rootpath = assetpath;

	m_graphicsSystem.reset(new GraphicsSystem(gcfg));
	m_inputSystem.reset(new InputSystem(m_window.get()));

	/////////////////////////////////////////////////////////////////////////
}

Application::~Application()
{
	//Shutdown
	m_inputSystem.reset();
	m_graphicsSystem.reset();
	m_window.reset();

	consoleClose();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Application::start()
{
	if (int err = this->onInit())
	{
		tserror("Failed to load application class (%)", err);

		//Force close window
		m_window->close();
		//Close events have to be handled manually
		while (m_window->poll()) {}

		return err;
	}

	{
		Stopwatch watch;
		double dt = 0.0;

		//Main engine loop
		while (m_window->poll())
		{
			watch.start();

			m_graphicsSystem->begin();
			
			//Update application
			this->onUpdate(dt);

			m_graphicsSystem->end();

			watch.stop();
			dt = max(0.0, watch.deltaTime()); //clamp delta time to positive value - just to be safe
		}
	}

	this->onExit();

	//Exit code is stored in the window
	return ((EngineWindow*)m_window.get())->getExitCode();
}

void Application::exit(int code)
{
	//Set environment exit code
	auto win = (EngineWindow*)m_window.get();
	win->setExitCode(code);
	//Destroy window
	win->close();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
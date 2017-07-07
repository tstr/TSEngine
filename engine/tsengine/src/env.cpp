/*
	Engine environment source
*/

#include <tsengine/env.h>
#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>
#include <tscore/system/thread.h>
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

	EngineEnv& m_env;
	int m_exitCode;

	/*
		Window event handlers
	*/

	void onResize() override
	{
		if (auto render = m_env.getGraphics())
		{
			uint w = 0;
			uint h = 0;
			Window::getSize(w, h);
			
			render->setDisplayInfo(eDisplayUnknown, w, h, SMultisampling(0));
		}
	}

	void onDestroy() override
	{
		m_env.getGraphics()->setDisplayInfo(eDisplayWindowed, 0, 0, 0);
	}

	void onEvent(const PlatformEventArgs& arg) override
	{
		m_env.getInput()->onEvent(arg);
	}

public:

	EngineWindow(EngineEnv& env, const WindowInfo& desc) :
		m_env(env),
		m_exitCode(0),
		Window(desc)
	{
	}

	~EngineWindow()
	{
	}

	EngineEnv& getSystem() { return m_env; }

	int getExitCode() const
	{
		return m_exitCode;
	}

	void setExitCode(int code)
	{
		m_exitCode = code;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Engine initialization
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EngineEnv::EngineEnv(int argc, char** argv)
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

	string assetpathbuf;
	m_vars->get("system.assetdir", assetpathbuf);
	Path assetpath = cfgpath.getParent();
	assetpath.addDirectories(assetpathbuf);

	uint32 samplecount = 1;
	m_vars->get("video.multisamplecount", samplecount);

	SGraphicsSystemInfo graphicscfg;
	graphicscfg.windowHandle = m_window->getHandle();
	graphicscfg.display.width = width;
	graphicscfg.display.height = height;
	graphicscfg.display.mode = (EDisplayMode)(displaymode + 1);
	graphicscfg.display.multisampling.count = samplecount;
	graphicscfg.apiid = EGraphicsAPIID::eGraphicsAPI_D3D11;
	graphicscfg.rootpath = assetpath;

	m_graphicsSystem.reset(new GraphicsSystem(graphicscfg));
	m_inputSystem.reset(new InputSystem(m_window.get()));

	/////////////////////////////////////////////////////////////////////////
}

EngineEnv::~EngineEnv()
{
	//Shutdown
	m_inputSystem.reset();
	m_graphicsSystem.reset();
	m_window.reset();

	consoleClose();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int EngineEnv::start(Application& app)
{
	if (int err = app.onInit())
	{
		tserror("Failed to load application class (%)", err);

		//Force close window
		m_window->close();
		//Close events have to be handled manually
		while (m_window->poll()) {}
		
		return err;
	}

	{
		Timer timer;
		Stopwatch watch;
		double dt = 0.0;

		//Main engine loop
		while (m_window->poll())
		{
			watch.start();

			m_graphicsSystem->begin();

			//Update application
			app.onUpdate(dt);

			m_graphicsSystem->end();

			watch.stop();
			dt = max(0.0, watch.deltaTime()); //clamp delta time to positive value - just to be safe
		}
	}

	app.onExit();

	//Exit code is stored in the window - workaround
	auto win = (EngineWindow*)m_window.get();
	return win->getExitCode();
}

void EngineEnv::exit(int code)
{
	//Set environment exit code
	auto win = (EngineWindow*)m_window.get();
	win->invoke([=] { win->setExitCode(code); });
	//Destroy window
	win->close();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
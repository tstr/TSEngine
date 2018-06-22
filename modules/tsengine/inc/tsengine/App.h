/*
	Engine Environment
*/

#pragma once

#include <tsengine/abi.h>

#include <tscore/path.h>
#include <tscore/system/memory.h>

#include <tsgraphics/Graphics.h>

#include "Input.h"
#include "VarTable.h"
#include "EnvInfo.h"

namespace ts
{
	class Window;

	/*
		Application class:

		- Represents the application environment.
		- Abstracts away platform functionality.
		- Handles application events.
		- Manages initialization/access of subsystems ie. Input/Graphics etc.
	*/
	class Application
	{
	private:

		// Platform window
		UPtr<Window> m_window;

		/*
			SubSystems
		*/

		// Graphical Subsystem
		UPtr<GraphicsSystem> m_graphicsSystem;
		
		// Input Subsystem
		UPtr<InputSystem> m_inputSystem;

		// Vars
		UPtr<VarTable> m_vars;

		//Internal methods
		void initErrorHandler();
		void initConfig(const Path& filepath);

	public:
		
		/*
			Construct an environment from given command line arguments.

			argc - number of command line arguments.
			argv - array of command line arguments, where the first argument is the path to the executing binary.
		*/
		TSENGINE_API Application(int argc, char** argv);
		TSENGINE_API ~Application();

		//Non-copyable/moveable
		Application(const Application& sys) = delete;
		Application(Application&& sys) = delete;

		//SubSystem methods
		Window* const window() const { return m_window.get(); }
		GraphicsSystem* const graphics() const { return m_graphicsSystem.get(); }
		InputSystem* const input() const { return m_inputSystem.get(); }

		/*
			Application events
		*/
		virtual int onInit() = 0;
		virtual void onExit() = 0;
		virtual void onUpdate(double deltatime) = 0;
		
		//System properties
		static TSENGINE_API void getSystemInfo(SSystemInfo& i);
		static TSENGINE_API void getSystemDisplayInfo(SDisplayInfo& i);
		static TSENGINE_API void getSystemMemoryInfo(SSystemMemoryInfo& i);

		TSENGINE_API Path getCurrentDir() const;
		TSENGINE_API Path getBinaryDir() const;

		//Start and run an application
		TSENGINE_API int start();

		//Exit current application
		TSENGINE_API void exit(int code);
	};
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////
};
/*
	Engine Environment
*/

#pragma once

#include <tsengine/abi.h>
#include <tsengine/VarTable.h>
#include <tsengine/EnvInfo.h>
#include <tsengine/Surface.h>

#include <tscore/path.h>
#include <tscore/system/memory.h>

#include <tsgraphics/GraphicsSystem.h>
#include <tsengine/Input.h>

namespace ts
{
	class Window;
	class InputSystem;
	class GraphicsSystem;
	class EngineEnv;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
		Application class:

		- Represents the application
		- Handles application events
		- Access environment resources (through EngineEnv)
	*/
	class Application
	{
	private:
		
		EngineEnv& m_env;
		
	protected:
		
		EngineEnv& getEnv() { return m_env; }
		const EngineEnv& getEnv() const { return m_env; }
		
	public:
		
		/*
			Construct an application with a given environment
		*/
		Application(EngineEnv& env) : m_env(env) {}
		Application(const Application&) = delete; 
		
		/*
			Application events
		*/
		virtual int onInit() = 0;
		virtual void onExit() = 0;
		virtual void onUpdate(double deltatime) = 0;
	};
	
	/*
		Environment class:

		- Represents the application environment.
		- Abstracts away platform functionality.
		- Manages initialization/communication/operation of subsystems ie. Input/Graphics etc.
	*/
	class EngineEnv
	{
	private:

		// Platform window
		UniquePtr<Window> m_window;

		/*
			SubSystems
		*/

		// Graphical Subsystem
		UniquePtr<GraphicsSystem> m_graphicsSystem;
		
		// Input Subsystem
		UniquePtr<InputSystem> m_inputSystem;

		// Environment Variable Table
		UniquePtr<VarTable> m_vars;


		//Internal methods
		void initErrorHandler();
		void initConfig(const Path& filepath);

	public:
		
		/*
			Construct an environment from given command line arguments.

			argc - number of command line arguments.
			argv - array of command line arguments, where the first argument is the path to the executing binary.
		*/
		TSENGINE_API EngineEnv(int argc, char** argv);
		TSENGINE_API ~EngineEnv();

		//Non-copyable/moveable
		EngineEnv(const EngineEnv& sys) = delete;
		EngineEnv(EngineEnv&& sys) = delete;

		//SubSystem methods
		GraphicsSystem* const getGraphics() const { return m_graphicsSystem.get(); }
		InputSystem* const getInput() const { return m_inputSystem.get(); }
		VarTable* const getVars() const { return m_vars.get(); }
		
		//System properties
		static TSENGINE_API void getSystemInfo(SSystemInfo& i);
		static TSENGINE_API void getSystemDisplayInfo(SDisplayInfo& i);
		static TSENGINE_API void getSystemMemoryInfo(SSystemMemoryInfo& i);

		TSENGINE_API Path getCurrentDir() const;
		TSENGINE_API Path getBinaryDir() const;

		//Start and run an application
		TSENGINE_API int start(Application& app);

		//Exit current application
		TSENGINE_API void exit(int code);
	};
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////
};
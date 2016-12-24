/*
	Base engine api
*/

#pragma once

#include <tsconfig.h>
#include <tsengine/abi.h>
#include <tscore/strings.h>
#include <tscore/system/memory.h>
#include <tscore/system/thread.h>
#include <tscore/filesystem/path.h>
#include <tsengine/cvar.h>

namespace ts
{
	class CWindow;
	class CInputModule;
	class GraphicsSystem;
	class CEngineEnv;

	struct IApplication
	{
		virtual int onInit() = 0;
		virtual void onExit() = 0;
		virtual void onUpdate(double deltatime) = 0;

		virtual ~IApplication() {}
	};

	struct SEngineStartupParams
	{
		char** argv = nullptr;
		int argc = 0;
		void* appInstance = nullptr;
		Path appPath;
		int showWindow = 0;
	};

	//Engine Environment class - root of application
	class CEngineEnv
	{
	private:

		UniquePtr<CWindow> m_window;
		UniquePtr<GraphicsSystem> m_graphics;
		UniquePtr<CInputModule> m_inputModule;
		UniquePtr<CVarTable> m_cvarTable;
		
	public:
		
		//constructor/destructor
		TSENGINE_API CEngineEnv(const SEngineStartupParams& params);
		TSENGINE_API ~CEngineEnv();

		CEngineEnv(const CEngineEnv& sys) = delete;
		CEngineEnv(CEngineEnv&& sys) = delete;

		//system methods
		CWindow* const getWindow() const { return m_window.get(); }
		GraphicsSystem* const getGraphics() const { return m_graphics.get(); }
		CInputModule* const getInput() const { return m_inputModule.get(); }
		CVarTable* const getCVarTable() const { return m_cvarTable.get(); }

		//Start and run an application
		TSENGINE_API int start(IApplication& app);
		//Notifies the engine to shut down
		TSENGINE_API void shutdown();
	};
};
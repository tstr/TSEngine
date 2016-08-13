/*
	Base engine api
*/

#pragma once

#include <tsconfig.h>
#include <tscore/strings.h>
#include <tscore/system/memory.h>
#include <tscore/system/thread.h>
#include <tscore/filesystem/path.h>
#include <tsengine/event/messenger.h>

namespace ts
{
	class CWindow;
	class CRenderModule;
	class CInputModule;

	struct IApplication
	{
		virtual void onInit() = 0;
		virtual void onExit() = 0;
		virtual void onUpdate() = 0;
		virtual void onRender() = 0;

		virtual ~IApplication() {}
	};

	struct SEngineStartupParams
	{
		IApplication* app = nullptr;
		void* appInstance = nullptr;
		std::string commandArgs;
		Path appPath;
		int showWindow = 0;
	};

	//Base engine class - this class is the center of the application and is responsible for initializing and connecting all engine modules together
	class CEngineSystem
	{
	private:

		std::atomic<bool> m_enabled;

		UniquePtr<IApplication> m_app;
		UniquePtr<CWindow> m_window;
		UniquePtr<CRenderModule> m_moduleRender;
		//UniquePtr<CInputModule> m_moduleInput;		//todo: implement

		void onExit();
		void onInit();

	public:

		//constructor/destructor
		CEngineSystem(const SEngineStartupParams& params);
		~CEngineSystem();

		CEngineSystem(const CEngineSystem& sys) = delete;
		CEngineSystem(CEngineSystem&& sys) = delete;

		//methods
		IApplication* const getApp() const { return m_app.get(); }
		CWindow* const getWindow() const { return m_window.get(); }

		//Close the application and shutdown engine
		void shutdown();
	};
};
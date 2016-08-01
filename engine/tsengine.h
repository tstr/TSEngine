/*
	Base engine api
*/

#pragma once

#include <tsconfig.h>
#include <tscore/strings.h>
#include <tscore/system/memory.h>

namespace ts
{
	class Window;
	
	struct IApplication
	{
		virtual void onInit() {}
		virtual void onShutdown() {}
		virtual void onUpdate() {}
		virtual void onRender() {}
	};

	struct SEngineStartupParams
	{
		std::string commandArgs;
		IApplication* app = nullptr;
		void* appInstance = nullptr;
		int showWindow = 0;
	};

	//Global engine class
	class CEngineSystem
	{
	private:

		Window* m_pWindow = nullptr;
		IApplication* m_pApp = nullptr;

		void onShutdown();
		void onInit();

	public:

		IApplication* getApp() const { return m_pApp; }
		Window* getWindow() const { return m_pWindow; }

		void init(const SEngineStartupParams& params);
		void shutdown();

		CEngineSystem();
		~CEngineSystem();

		CEngineSystem(const CEngineSystem& sys) = delete;
		CEngineSystem(CEngineSystem&& sys) = delete;
	};

	extern CEngineSystem* const gSystem;
};
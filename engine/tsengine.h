/*
	Base engine api
*/

#pragma once

#include <tsconfig.h>
#include <tscore/strings.h>
#include <tscore/system/memory.h>
#include <tsengine/event/messenger.h>

namespace ts
{
	class CWindow;

	struct IApplication
	{
		virtual void onInit() {}
		virtual void onDeinit() {}
		virtual void onUpdate() {}
		virtual void onRender() {}

		virtual ~IApplication() {}
	};

	struct SEngineStartupParams
	{
		IApplication* app = nullptr;
		void* appInstance = nullptr;
		std::string commandArgs;
		std::string appPath;
		int showWindow = 0;
	};

	enum EEngineMessageCode
	{
		eEngineMessageNull = 0,
		eEngineMessageShutdown = 2
	};

	struct SEngineMessage
	{
		EEngineMessageCode code = eEngineMessageNull;
		uint64 a = 0;
		uint64 b = 0;

		SEngineMessage() {}
		SEngineMessage(EEngineMessageCode c) : code(c) {}
	};

	//Base engine class - this class is the center of the application and is responsible for initializing and connecting all engine modules together
	class CEngineSystem
	{
	private:

		UniquePtr<CWindow> m_pWindow;
		UniquePtr<IApplication> m_pApp;

		CMessageReciever<SEngineMessage> m_reciever;

		void onDeinit();
		void onInit();

	public:

		//constructor/destructor
		CEngineSystem(const SEngineStartupParams& params);
		~CEngineSystem();

		CEngineSystem(const CEngineSystem& sys) = delete;
		CEngineSystem(CEngineSystem&& sys) = delete;

		//methods
		IApplication* const getApp() const { return m_pApp.get(); }
		CWindow* const getWindow() const { return m_pWindow.get(); }

		//Close the application and shutdown engine
		void shutdown();
	};
};
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
#include <tsengine/cvar.h>

namespace ts
{
	class CWindow;
	class CRenderModule;
	class CInputModule;
	class CEngineSystem;

	struct IApplication
	{
		virtual void onInit(CEngineSystem* system) = 0;
		virtual void onExit() = 0;
		virtual void onUpdate(double deltatime) = 0;

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

	enum class ESystemMessage
	{
		eMessageNull   = 0,
		eMessageExit   = 1,
	};
	
	struct SSystemMessage
	{
		ESystemMessage eventcode = ESystemMessage::eMessageNull;
		uint64 param = 0;

		SSystemMessage() {}
		SSystemMessage(ESystemMessage code) : eventcode(code) {}
	};
	
	//Base engine class - this class is the center of the application and is responsible for initializing and handling communications between engine modules
	class CEngineSystem
	{
	private:

		UniquePtr<IApplication> m_app;
		UniquePtr<CWindow> m_window;
		UniquePtr<CRenderModule> m_renderModule;
		UniquePtr<CInputModule> m_inputModule;
		UniquePtr<CVarTable> m_cvarTable;

		CMessageReciever<SSystemMessage> m_messageReciever;

		void onExit();
		void onInit();

		int run();
		void consoleCommands();

		mutex m_exitMutex;

	public:
	
		//constructor/destructor
		CEngineSystem(const SEngineStartupParams& params);
		~CEngineSystem();

		CEngineSystem(const CEngineSystem& sys) = delete;
		CEngineSystem(CEngineSystem&& sys) = delete;

		//methods
		IApplication* const getApp() const { return m_app.get(); }
		CWindow* const getWindow() const { return m_window.get(); }
		CRenderModule* const getRenderModule() const { return m_renderModule.get(); }
		CInputModule* const getInputModule() const { return m_inputModule.get(); }
		CVarTable* const getCVarTable() const { return m_cvarTable.get(); }

		//Close the application and shutdown engine
		void shutdown();
	};
};
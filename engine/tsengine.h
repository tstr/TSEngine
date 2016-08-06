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
	class Window;

	struct IApplication
	{
		virtual void onInit() {}
		virtual void onDeinit() {}
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

		enum EEngineMessageCode
		{
			eEngineMessageNull = 0,
			eEngineMessageInit = 1,
			eEngineMessageDeinit = 2
		};

		struct SEngineMessage
		{
			EEngineMessageCode code = eEngineMessageNull;
			uint64 a = 0;
			uint64 b = 0;

			SEngineMessage() {}
			SEngineMessage(EEngineMessageCode c) : code(c) {}
		};

		Window* m_pWindow = nullptr;
		IApplication* m_pApp = nullptr;

		CMessageReciever<SEngineMessage> m_reciever;

		void onDeinit();
		void onInit();

	public:

		IApplication* getApp() const { return m_pApp; }
		Window* getWindow() const { return m_pWindow; }

		void init(const SEngineStartupParams& params);
		void deinit();

		CEngineSystem();
		~CEngineSystem();

		CEngineSystem(const CEngineSystem& sys) = delete;
		CEngineSystem(CEngineSystem&& sys) = delete;
	};

	extern CEngineSystem* const gSystem;
};
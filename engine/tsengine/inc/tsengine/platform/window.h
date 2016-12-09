/*
	Window API - abstraction layer for platform specific windowing system
*/

#pragma once

#include <tsengine/abi.h>
#include <tscore/types.h>
#include <tscore/strings.h>

namespace ts
{
	class CWindow;

	enum EWindowEvent : uint32
	{
		eEventNull,
		eEventCreate,
		eEventDestroy,
		eEventClose,
		eEventResize,
		eEventInput,
		eEventActivate,
		eEventDraw,
		eEventSetfocus,
		eEventKillfocus,
		eEventChar,
		eEventKeydown,
		eEventKeyup,
		eEventScroll,
		eEventMouseDown,
		eEventMouseUp,
		eEventMouseMove,
		EnumMax
	};

	struct SWindowEventArgs
	{
		CWindow* pWindow = nullptr;
		EWindowEvent eventcode = EWindowEvent::eEventNull;
		uint64 a = 0; //lparam (Win32)
		uint64 b = 0; //wparam (Win32)
	};

	struct SWindowRect
	{
		uint32 x = 0;
		uint32 y = 0;
		uint32 w = 0;
		uint32 h = 0;
	};

	struct SWindowDesc
	{
		std::string title;
		SWindowRect rect;
		void* appInstance = nullptr;
	};

	//Window class - represents an application window
	class TSENGINE_API CWindow
	{
	private:

		struct Impl;
		Impl* pImpl = nullptr;

		struct IInvoker
		{
			virtual void execute() = 0;
		};

		void invoke_internal(IInvoker* i);

	public:

		CWindow(const SWindowDesc& desc);
		virtual ~CWindow();
		
		CWindow(const CWindow&) = delete;
		CWindow(CWindow&&) = delete;

		struct IEventListener
		{
			enum EReturnValue
			{
				eDefault = 0,
				eHandled = 1	//Return this if IEventListener::onEvent handles the event
			};

			virtual int onWindowEvent(const SWindowEventArgs& args) = 0;
		};

		void addEventListener(IEventListener* listener);
		void removeEventListener(IEventListener* listener);

		void open(int show = 0);
		void close();

		enum EEventQueueRet
		{
			eQueueExit			 = 0,
			eQueueMessageEmpty   = 1,
			eQueueMessagePresent = 2
		};

		bool isOpen() const;
		int handleEvents();	//Handle window messages (non blocking) - Returns true if a messages are present in queue
		intptr nativeHandle() const;	//Returns a handle to the window - depends on platform

		template<typename t>
		void invoke(t _f)
		{
			using namespace std;

			struct CInvoker : public IInvoker
			{
				reference_wrapper<t> m_f;

				CInvoker(reference_wrapper<t> _f) : m_f(_f) {}

				void execute() override
				{
					m_f();
				}
				
			} i(ref(_f));

			invoke_internal(&i);
		}
	};
	
	//Helper functions
	void getWindowResizeEventArgs(const SWindowEventArgs& args, uint32& w, uint32& h);
}
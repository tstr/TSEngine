/*
	Window API - abstraction layer for platform specific windowing system
*/

#pragma once

#include <tscore/types.h>

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
	class CWindow
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
		~CWindow();
			
		CWindow(const CWindow&) = delete;
		CWindow(CWindow&& mov) { pImpl = mov.pImpl; mov.pImpl = nullptr; }

		struct IEventListener
		{
			enum EReturnValue
			{
				eDefault = 0,
				eHandled = 1	//Return this if IEventListener::onEvent handles the event
			};

			virtual int onEvent(const SWindowEventArgs& args) = 0;
		};

		void setEventListener(IEventListener* listener);

		void open(int show = 0);
		void close();

		bool isOpen() const;

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

		//todo: move these methods to CRenderModule when it is ready
		bool isFullscreen() const;
		void setFullscreen(bool on);

		intptr handle() const;
		void raiseEvent(EWindowEvent e, uint64 a, uint64 b);
		void messageBox(const char* text, const char* caption = "");
	};
}
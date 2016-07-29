/*
	Window API - abstraction layer for platform specific windowing system
*/

#pragma once

#include <tscore/types.h>

namespace ts
{
	class Window;

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

	struct WindowEventArgs
	{
		Window* window = nullptr;
		EWindowEvent eventcode = EWindowEvent::eEventNull;
		uint64 a = 0; //lparam
		uint64 b = 0; //wparam
	};

	struct WindowRect
	{
		uint32 x = 0;
		uint32 y = 0;
		uint32 w = 0;
		uint32 h = 0;
	};

	struct IWindowEventhandler
	{
		EWindowEvent eventcode = EWindowEvent::eEventNull;
		bool handled = true;

		virtual void execute(WindowEventArgs args) = 0;
	};

	class Window
	{
	private:

		struct Impl;
		Impl* pImpl = nullptr;

		int defaultEventhandler(uint32 msg, uint64 a, uint64 b);

		struct IInvoker
		{
			virtual void execute() = 0;
		};

		void invoke_internal(IInvoker* i);

	public:

		Window(const char* title);
		~Window();
			
		Window(const Window&) = delete;
		Window(Window&& mov) { pImpl = mov.pImpl; mov.pImpl = nullptr; }

		bool setEventHandler(EWindowEvent e, IWindowEventhandler* handler);

		void create(WindowRect);
		void createAsync(WindowRect);
		void close();

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

		//void Invoke(const std::function<void()>& f);
		bool isOpen() const;
		bool isFullscreen() const;
		void setFullscreen(bool on);

		uint64 id() const;
		void raiseEvent(EWindowEvent e, uint64 a, uint64 b);
		void msgBox(const char* text, const char* caption = "");

		void setTitle(const char* title);

		//Default event handlers
		virtual void onCreate(WindowEventArgs e);
		virtual void onDestroy(WindowEventArgs e);
		virtual void onClose(WindowEventArgs e);
		virtual void onResize(WindowEventArgs e);
		virtual void onInput(WindowEventArgs e);
		virtual void onActivate(WindowEventArgs e);
		virtual void onDraw(WindowEventArgs e);
		virtual void onSetfocus(WindowEventArgs e);
		virtual void onKillfocus(WindowEventArgs e);
		virtual void onKeydown(WindowEventArgs e);
		virtual void onKeyup(WindowEventArgs e);
		virtual void onScroll(WindowEventArgs e);
		virtual void onMouseDown(WindowEventArgs e);
		virtual void onMouseUp(WindowEventArgs e);
		virtual void onMouseMove(WindowEventArgs e);
	};
}
/*
	Window API - abstraction layer for platform specific windowing system
*/

#pragma once

#include <C3E\core\corecommon.h>
#include <functional>

namespace C3E
{
	class Window;

	enum class WindowEvent : uint32
	{
		EventNull,
		EventCreate,
		EventDestroy,
		EventClose,
		EventResize,
		EventInput,
		EventActivate,
		EventDraw,
		EventSetfocus,
		EventKillfocus,
		EventKeydown,
		EventKeyup,
		EventScroll,
		EventMouseDown,
		EventMouseUp,
		EventMouseMove,
	};

	struct WindowEventArgs
	{
		Window* window = nullptr;
		WindowEvent eventcode = WindowEvent(0);
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
		WindowEvent eventcode = WindowEvent(0);
		bool handled = true;

		virtual void execute(WindowEventArgs args) = 0;
	};

	class IWindow
	{
	public:

		virtual uint64 id() const = 0;
	};

	class Window : public IWindow
	{
	private:

		struct Impl;
		Impl* pImpl = nullptr;

		int DefaultEventhandler(uint32 msg, uint64 a, uint64 b);

		struct IInvoker
		{
			virtual void execute() = 0;
		};

		void Invoke_internal(IInvoker* i);

	public:

		Window(const char* title);
		~Window();
			
		Window(const Window&) = delete;
		Window(Window&& mov) { pImpl = mov.pImpl; mov.pImpl = nullptr; }

		bool SetEventHandler(WindowEvent e, IWindowEventhandler* handler);

		void Create(WindowRect);
		void AsyncCreate(WindowRect);
		void Close();

		template<typename t>
		void Invoke(t _f)
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

			Invoke_internal(&i);
		}

		//void Invoke(const std::function<void()>& f);
		bool IsOpen() const;
		bool IsFullscreen() const;
		void SetFullscreen(bool on);

		uint64 id() const override;
		void RaiseEvent(WindowEvent e, uint64 a, uint64 b);
		void MsgBox(const char* text, const char* caption = "");

		void SetTitle(const char* title);

		//Default event handlers
		virtual void OnCreate(WindowEventArgs e);
		virtual void OnDestroy(WindowEventArgs e);
		virtual void OnClose(WindowEventArgs e);
		virtual void OnResize(WindowEventArgs e);
		virtual void OnInput(WindowEventArgs e);
		virtual void OnActivate(WindowEventArgs e);
		virtual void OnDraw(WindowEventArgs e);
		virtual void OnSetfocus(WindowEventArgs e);
		virtual void OnKillfocus(WindowEventArgs e);
		virtual void OnKeydown(WindowEventArgs e);
		virtual void OnKeyup(WindowEventArgs e);
		virtual void OnScroll(WindowEventArgs e);
		virtual void OnMouseDown(WindowEventArgs e);
		virtual void OnMouseUp(WindowEventArgs e);
		virtual void OnMouseMove(WindowEventArgs e);
	};
}
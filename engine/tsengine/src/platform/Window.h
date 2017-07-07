/*
	Abstract Window Class
*/

#pragma once

#include <tscore/types.h>
#include <tscore/strings.h>

#include <tsengine/Surface.h>

namespace ts
{
	struct PlatformEventArgs
	{
		uint32 code; //msg
		uint64 a;    //wparam
		uint64 b;    //lparam
	};

	struct WindowRect
	{
		uint32 x;
		uint32 y;

		uint32 w;
		uint32 h;
	};

	struct WindowInfo
	{
		String title;
		WindowRect rect;
	};

	/*
		Window class

		Encapsulates basic window functionality: basic events, resizing, drawing etc.
		
		Implements the ISurface interface.
	*/
	class Window : public ISurface
	{
	private:
		
		struct Impl;
		Impl* pImpl = nullptr;
		
		struct IInvoker
		{
			virtual void execute() = 0;
		};
		
		void invokeImpl(IInvoker* i);
		
	public:
		
		/*
			Construct a window with given parameters
		*/
		Window(const WindowInfo& info);
		
		virtual ~Window();
		
		Window(const Window&) = delete;
		Window(Window&&) = delete;
		
		////////////////////////////////////////////////////////////////////////
		
		/*
			Opens the window
		*/
		void open();
		
		/*
			Closes the window
		*/
		void close();
		
		/*
			Return true if window is in open state
		*/
		bool isOpen() const;
		
		/*
			Polls for messages in the message queue
		*/
		int poll();
		
		////////////////////////////////////////////////////////////////////////
		// Window event handlers
		////////////////////////////////////////////////////////////////////////
		
		virtual void onCreate();
		virtual void onSuspend();
		
		virtual void onClose();
		virtual void onDestroy();
		
		virtual void onActivate();
		virtual void onDeactivate();
		
		virtual void onResize();
		virtual void onRedraw();

		virtual void onEvent(const PlatformEventArgs& arg);
		
		////////////////////////////////////////////////////////////////////////
		// ISurface overrides
		////////////////////////////////////////////////////////////////////////
		
		void enableBorderless(bool enable) override;
		bool isBorderless() const  override;
		bool supportsBorderless() const override;

		void getSize(uint& width, uint& height) const override;
		
		void resize(uint width, uint height) override;
		
		void redraw() override;
		
		intptr getHandle() const override;
		
		////////////////////////////////////////////////////////////////////////
		
		/*
			Call a given functor on the window message queue
		*/
		template<typename F>
		void invoke(F func)
		{
			using namespace std;

			struct CInvoker : public IInvoker
			{
				reference_wrapper<F> m_f;

				CInvoker(reference_wrapper<F> func) : m_f(func) {}

				void execute() override
				{
					m_f();
				}
				
			} i(ref(func));

			invokeImpl(&i);
		}
	};
}
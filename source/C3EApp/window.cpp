/*
	Windowing system
*/

#include <array>
#include <atomic>
#include <map>

#include <C3Ext\win32.h>
#include <Windows.h>
#include <windowsx.h> //todo: use the macros

#include "window.h"
#include <C3E\core\corecommon.h>
#include <C3E\core\threading.h>


#ifdef _WIN32

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#else

#error this platform does not support visual styles

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <CommCtrl.h>

static bool EnableVisualStyles()
{
	if (HMODULE h = LoadLibraryA("Comctl32.dll"))
	{
		typedef decltype(InitCommonControlsEx)* fnc_t;
		fnc_t f = (fnc_t)GetProcAddress(h, "InitCommonControlsEx");

		if (f)
		{
			INITCOMMONCONTROLSEX icc;
			icc.dwICC = ICC_STANDARD_CLASSES;
			icc.dwSize = sizeof(INITCOMMONCONTROLSEX);

			return (f(&icc) != 0);
		}

		FreeLibrary(h);
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define WMO_CREATE (WM_USER + 0x0001)
#define WMO_INVOKE (WM_USER + 0x0002)
#define WMO_DESTROY WM_DESTROY

#define WINDOW_MAX_EVENTHANDLERS 32

//Declare default event handler
#define DECLARE_EVENTHANDLER(index, eventcode, handler_func)\
	windowDefaultEventhandlers[index].fptr = handler_func ;\
	windowEventhandlerTable[EventCodes.GetWin32MessageEnum(eventcode)].eventhandler = &windowDefaultEventhandlers[index]

enum Win32flags
{
	win_registered = 1,
	win_open = 2,
	win_borderless = 4
};

struct Win32Rect
{
	UINT x;
	UINT y;
	UINT w;
	UINT h;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace C3E;

static const class Win32EventEnums
{
private:

	map<WindowEvent, uint32> m_windowEvents;

public:

	Win32EventEnums()
	{
		//Set up event table for matching enums
		m_windowEvents[WindowEvent::EventInput] = WM_INPUT;
		m_windowEvents[WindowEvent::EventActivate] = WM_ACTIVATE;
		m_windowEvents[WindowEvent::EventResize] = WM_SIZE;
		m_windowEvents[WindowEvent::EventCreate] = WMO_CREATE;
		m_windowEvents[WindowEvent::EventDestroy] = WMO_DESTROY;
		m_windowEvents[WindowEvent::EventClose] = WM_CLOSE;
		m_windowEvents[WindowEvent::EventDraw] = WM_PAINT;
		m_windowEvents[WindowEvent::EventSetfocus] = WM_SETFOCUS;
		m_windowEvents[WindowEvent::EventKillfocus] = WM_KILLFOCUS;
		m_windowEvents[WindowEvent::EventKeydown] = WM_KEYDOWN;
		m_windowEvents[WindowEvent::EventKeyup] = WM_KEYUP;
		m_windowEvents[WindowEvent::EventScroll] = WM_MOUSEWHEEL;
		m_windowEvents[WindowEvent::EventMouseDown] = WM_LBUTTONDOWN;
		m_windowEvents[WindowEvent::EventMouseUp] = WM_LBUTTONUP;
		m_windowEvents[WindowEvent::EventMouseMove] = WM_MOUSEMOVE;
	}

	WindowEvent GetWindowEventEnum(uint32 eventcode) const
	{
		//return m_windowEvents.at(eventcodes);
		return WindowEvent::EventNull;
	}

	uint32 GetWin32MessageEnum(WindowEvent eventcode) const
	{
		return m_windowEvents.at(eventcode);
	}

} EventCodes;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Window::Impl
{
	Window* window = nullptr;
	HWND windowHandle;
	WNDCLASSEX windowClass;
	HMODULE windowModule;

	string windowClassname;
	string windowTitle;

	thread windowThread;

	Win32Rect size;
	Win32Rect sizeCache;

	atomic<int> flags = 0;

	struct Event
	{
		WindowEvent eventcode = WindowEvent(0);
		IWindowEventhandler* eventhandler = nullptr;
	};

	typedef void(Window::*WindowDefaultEventhandlerPtr)(WindowEventArgs);

	struct DefaultEventhandler : public IWindowEventhandler
	{
		WindowDefaultEventhandlerPtr fptr = nullptr;

		void execute(WindowEventArgs e) override
		{
			(e.window->*fptr)(e);
		}
	};

	DefaultEventhandler windowDefaultEventhandlers[WINDOW_MAX_EVENTHANDLERS];
	map<uint32, Event> windowEventhandlerTable;

	Impl(Window* window, const char* t) :
		window(window),
		windowClassname("C3EAppWindow"),
		windowTitle(t)
	{
		DECLARE_EVENTHANDLER(0, WindowEvent::EventCreate, &Window::OnCreate);
		DECLARE_EVENTHANDLER(1, WindowEvent::EventActivate, &Window::OnActivate);
		DECLARE_EVENTHANDLER(2, WindowEvent::EventClose, &Window::OnClose);
		DECLARE_EVENTHANDLER(3, WindowEvent::EventDestroy, &Window::OnDestroy);
		DECLARE_EVENTHANDLER(4, WindowEvent::EventInput, &Window::OnInput);
		DECLARE_EVENTHANDLER(5, WindowEvent::EventMouseMove, &Window::OnMouseMove);
		DECLARE_EVENTHANDLER(6, WindowEvent::EventResize, &Window::OnResize);
		DECLARE_EVENTHANDLER(7, WindowEvent::EventDraw, &Window::OnDraw);
		DECLARE_EVENTHANDLER(8, WindowEvent::EventSetfocus, &Window::OnSetfocus);
		DECLARE_EVENTHANDLER(9, WindowEvent::EventKillfocus, &Window::OnKillfocus);
		DECLARE_EVENTHANDLER(10, WindowEvent::EventKeydown, &Window::OnKeydown);
		DECLARE_EVENTHANDLER(11, WindowEvent::EventKeyup, &Window::OnKeyup);
		DECLARE_EVENTHANDLER(12, WindowEvent::EventScroll, &Window::OnScroll);
		DECLARE_EVENTHANDLER(13, WindowEvent::EventMouseDown, &Window::OnMouseDown);
		DECLARE_EVENTHANDLER(14, WindowEvent::EventMouseUp, &Window::OnMouseUp);

		//Win32 window class

		windowModule = GetModuleHandle(0);

		ZeroMemory(&windowClass, sizeof(WNDCLASSEX));

		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;

		windowClass.hInstance = windowModule;
		windowClass.lpfnWndProc = (WNDPROC)&Window::Impl::WndProc;
		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

		windowClass.lpszClassName = windowClassname.c_str();
		windowClass.lpszMenuName = 0;

		//windowClass.hbrBackground = HBRUSH(GetStockObject(BLACK_BRUSH));
		windowClass.hbrBackground = 0;

		windowClass.hCursor = LoadCursor(0, IDC_ARROW);
		windowClass.hIcon = LoadIcon(0, IDI_APPLICATION);
		windowClass.hIconSm = 0;

		if (!RegisterClassEx(&windowClass))
			throw exception("Unabled to register Win32 window");

		flags |= win_registered;
	}

	~Impl()
	{
		if (!(flags & win_registered)) return;

		Destroy();

		C3E_ASSERT(UnregisterClass(windowClassname.c_str(), windowModule));
		flags &= ~win_registered;
	}

	static LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		auto wnd = reinterpret_cast<Window::Impl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

		if (wnd)
		{
			if (msg == WMO_INVOKE)
			{
				auto f = (Window::IInvoker*)wparam;
				if (f) f->execute();
				return 0;
			}

			const map<uint32, Window::Impl::Event>::iterator& it = wnd->windowEventhandlerTable.find(msg);

			if (it != wnd->windowEventhandlerTable.end())
			{
				const Window::Impl::Event& e = it->second;

				if (e.eventhandler)
				{
					WindowEventArgs args;
					args.window = wnd->window;
					args.eventcode = e.eventcode;
					args.a = lparam;
					args.b = wparam;
					e.eventhandler->execute(args);

					return 0;
				}
			}
		}
		
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	void Create(const WindowRect& rect)
	{
		size.x = rect.x;
		size.y = rect.y;
		size.h = rect.h;
		size.w = rect.w;

		C3E_ASSERT(EnableVisualStyles());

		windowHandle = CreateWindowEx(
			WS_EX_ACCEPTFILES,
			windowClassname.c_str(),
			windowTitle.c_str(),
			WS_VISIBLE | WS_OVERLAPPEDWINDOW,
			size.x,
			size.y,
			size.w,
			size.h,
			0,
			0,
			windowModule,
			0
		);

		if (!IsWindow(windowHandle))
		{
			throw exception("Failed to create win32 window");
		}

		flags |= win_open;

		SetWindowLongPtr(windowHandle, GWLP_USERDATA, LONG_PTR(this));
		SendMessage(windowHandle, WMO_CREATE, 0, 0);

		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));

		UpdateWindow(windowHandle);

		while (BOOL ret = GetMessage(&msg, NULL, 0, 0))
		{
			C3E_ASSERT(ret >= 0);

			if (ret)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				return;
			}
		}

	}

	void AsyncCreate(const WindowRect& rect)
	{
		windowThread = thread(&Window::Impl::Create, this, rect);

		while (!IsWindow(windowHandle));
	}

	void Destroy()
	{
		if (!(flags & win_open)) return;

		SendMessageA(windowHandle, WM_DESTROY, 0, 0);

		if (windowThread.joinable())
			windowThread.join();

		flags &= ~win_open;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Window::Window(const char* t) :
	pImpl(new Impl(this, t))
{

}

Window::~Window()
{
	if (pImpl)
	{
		delete pImpl;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Window::SetEventHandler(WindowEvent ecode, IWindowEventhandler* handler)
{	
	if (uint32 code = EventCodes.GetWin32MessageEnum(ecode))
	{
		Impl::Event& e = pImpl->windowEventhandlerTable[code];
		e.eventhandler = handler;
		e.eventcode = ecode;

		return true;
	}

	return false;
}

int Window::DefaultEventhandler(uint32 msg, uint64 a, uint64 b)
{
	return (int)DefWindowProc(pImpl->windowHandle, msg, a, b);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::SetFullscreen(bool on)
{
	if (on)
	{
		DEVMODE dev;
		ZeroMemory(&dev, sizeof(DEVMODE));

		pImpl->sizeCache = pImpl->size;

		UINT width = GetSystemMetrics(SM_CXSCREEN), height = GetSystemMetrics(SM_CYSCREEN);

		pImpl->size.h = height;
		pImpl->size.w = width;

		EnumDisplaySettings(NULL, 0, &dev);

		HDC context = GetWindowDC(pImpl->windowHandle);
		int colourBits = GetDeviceCaps(context, BITSPIXEL);
		int refreshRate = GetDeviceCaps(context, VREFRESH);

		dev.dmPelsWidth = width;
		dev.dmPelsHeight = height;
		dev.dmBitsPerPel = colourBits;
		dev.dmDisplayFrequency = refreshRate;
		dev.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

		//if (ChangeDisplaySettingsA(&dev, CDS_FULLSCREEN) == DISP_CHANGE_SUCCESSFUL) success = true;

		SetWindowLongPtr(pImpl->windowHandle, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		SetWindowPos(pImpl->windowHandle, HWND_TOP, 0, 0, width, height, 0);
		BringWindowToTop(pImpl->windowHandle);

		pImpl->flags |= win_borderless;
	}
	else
	{
		SetWindowLongPtr(pImpl->windowHandle, GWL_EXSTYLE, WS_EX_LEFT);
		SetWindowLongPtr(pImpl->windowHandle, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);

		SetWindowPos(pImpl->windowHandle, HWND_NOTOPMOST, pImpl->sizeCache.x, pImpl->sizeCache.y, pImpl->sizeCache.w, pImpl->sizeCache.h, SWP_SHOWWINDOW);
		ShowWindow(pImpl->windowHandle, SW_RESTORE);

		pImpl->size = pImpl->sizeCache;

		//flagFullscreen = false;
		pImpl->flags &= ~win_borderless;
	}
}

bool Window::IsFullscreen() const
{
	return ((pImpl->flags & win_borderless) != 0);
}

bool Window::IsOpen() const
{
	return ((pImpl->flags & win_open) != 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void Window::Create(WindowRect r)
{
	pImpl->Create(r);
}

void Window::AsyncCreate(WindowRect r)
{
	pImpl->AsyncCreate(r);
}

void Window::Close()
{
	RaiseEvent(WindowEvent::EventDestroy, 0, 0);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint64 Window::id() const
{
	return (uint64)pImpl->windowHandle;
}

void Window::MsgBox(const char* text, const char* caption)
{
	MessageBoxA(pImpl->windowHandle, text, caption, 0);
}

void Window::RaiseEvent(WindowEvent e, uint64 a, uint64 b)
{
	SendMessage(pImpl->windowHandle, EventCodes.GetWin32MessageEnum(e), a, b);
}

void Window::SetTitle(const char* title)
{
	SetWindowTextA(pImpl->windowHandle, title);
}

void Window::Invoke_internal(Window::IInvoker* i)
{
	SendMessageA(pImpl->windowHandle, WMO_INVOKE, (WPARAM)i, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Default event handlers
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::OnCreate(WindowEventArgs e)
{
	DefaultEventhandler(WM_CREATE, e.a, e.b);
}

void Window::OnDestroy(WindowEventArgs e)
{
	PostQuitMessage(0);
}

void Window::OnClose(WindowEventArgs e)
{
	this->RaiseEvent(WindowEvent::EventDestroy, 0, 0);
}

void Window::OnResize(WindowEventArgs e)
{

}

void Window::OnMouseMove(WindowEventArgs e)
{

}

void Window::OnInput(WindowEventArgs e)
{

}

void Window::OnActivate(WindowEventArgs e)
{

}

void Window::OnDraw(WindowEventArgs e)
{
	DefaultEventhandler(WM_PAINT, e.a, e.b);
}

void Window::OnSetfocus(WindowEventArgs e)
{
	DefaultEventhandler(WM_SETFOCUS, e.a, e.b);
}

void Window::OnKillfocus(WindowEventArgs e)
{
	DefaultEventhandler(WM_KILLFOCUS, e.a, e.b);
}

void Window::OnKeydown(WindowEventArgs e)
{

}

void Window::OnKeyup(WindowEventArgs e)
{

}

void Window::OnScroll(WindowEventArgs e)
{

}

void Window::OnMouseDown(WindowEventArgs e)
{

}

void Window::OnMouseUp(WindowEventArgs e)
{

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
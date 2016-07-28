/*
	Windowing system
*/

#include <array>
#include <atomic>
#include <map>
#include <thread>

#include <Windows.h>
#include <windowsx.h> //todo: use the macros

#include "window.h"
#include <tscore/debug/assert.h>

//#define USE_VISUAL_STYLES

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef USE_VISUAL_STYLES

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

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

#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace ts;

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
using namespace ts;

static const class Win32EventEnums
{
private:

	array<uint32, WindowEvent::EnumMax> m_windowEvents;

public:

	Win32EventEnums()
	{
		//Set up event table for matching enums
		m_windowEvents[WindowEvent::eEventNull] = WM_NULL;
		m_windowEvents[WindowEvent::eEventInput] = WM_INPUT;
		m_windowEvents[WindowEvent::eEventActivate] = WM_ACTIVATE;
		m_windowEvents[WindowEvent::eEventResize] = WM_SIZE;
		m_windowEvents[WindowEvent::eEventCreate] = WMO_CREATE;
		m_windowEvents[WindowEvent::eEventDestroy] = WMO_DESTROY;
		m_windowEvents[WindowEvent::eEventClose] = WM_CLOSE;
		m_windowEvents[WindowEvent::eEventDraw] = WM_PAINT;
		m_windowEvents[WindowEvent::eEventSetfocus] = WM_SETFOCUS;
		m_windowEvents[WindowEvent::eEventKillfocus] = WM_KILLFOCUS;
		m_windowEvents[WindowEvent::eEventKeydown] = WM_KEYDOWN;
		m_windowEvents[WindowEvent::eEventKeyup] = WM_KEYUP;
		m_windowEvents[WindowEvent::eEventScroll] = WM_MOUSEWHEEL;
		m_windowEvents[WindowEvent::eEventMouseDown] = WM_LBUTTONDOWN;
		m_windowEvents[WindowEvent::eEventMouseUp] = WM_LBUTTONUP;
		m_windowEvents[WindowEvent::eEventMouseMove] = WM_MOUSEMOVE;
	}

	WindowEvent GetWindowEventEnum(uint32 eventcode) const
	{
		//return m_windowEvents.at(eventcodes);
		return WindowEvent::eEventNull;
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
		windowClassname("tsAppWindow"),
		windowTitle(t)
	{
		DECLARE_EVENTHANDLER(0, WindowEvent::eEventCreate, &Window::onCreate);
		DECLARE_EVENTHANDLER(1, WindowEvent::eEventActivate, &Window::onActivate);
		DECLARE_EVENTHANDLER(2, WindowEvent::eEventClose, &Window::onClose);
		DECLARE_EVENTHANDLER(3, WindowEvent::eEventDestroy, &Window::onDestroy);
		DECLARE_EVENTHANDLER(4, WindowEvent::eEventInput, &Window::onInput);
		DECLARE_EVENTHANDLER(5, WindowEvent::eEventMouseMove, &Window::onMouseMove);
		DECLARE_EVENTHANDLER(6, WindowEvent::eEventResize, &Window::onResize);
		DECLARE_EVENTHANDLER(7, WindowEvent::eEventDraw, &Window::onDraw);
		DECLARE_EVENTHANDLER(8, WindowEvent::eEventSetfocus, &Window::onSetfocus);
		DECLARE_EVENTHANDLER(9, WindowEvent::eEventKillfocus, &Window::onKillfocus);
		DECLARE_EVENTHANDLER(10, WindowEvent::eEventKeydown, &Window::onKeydown);
		DECLARE_EVENTHANDLER(11, WindowEvent::eEventKeyup, &Window::onKeyup);
		DECLARE_EVENTHANDLER(12, WindowEvent::eEventScroll, &Window::onScroll);
		DECLARE_EVENTHANDLER(13, WindowEvent::eEventMouseDown, &Window::onMouseDown);
		DECLARE_EVENTHANDLER(14, WindowEvent::eEventMouseUp, &Window::onMouseUp);

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

		tsassert(UnregisterClass(windowClassname.c_str(), windowModule));
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

#ifdef USE_VISUAL_STYLES
		tsassert(EnableVisualStyles());
#endif

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
			tsassert(ret >= 0);
			
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

bool Window::setEventHandler(WindowEvent ecode, IWindowEventhandler* handler)
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

int Window::defaultEventhandler(uint32 msg, uint64 a, uint64 b)
{
	return (int)DefWindowProc(pImpl->windowHandle, msg, a, b);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::setFullscreen(bool on)
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

bool Window::isFullscreen() const
{
	return ((pImpl->flags & win_borderless) != 0);
}

bool Window::isOpen() const
{
	return ((pImpl->flags & win_open) != 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void Window::create(WindowRect r)
{
	pImpl->Create(r);
}

void Window::asyncCreate(WindowRect r)
{
	pImpl->AsyncCreate(r);
}

void Window::close()
{
	raiseEvent(WindowEvent::eEventDestroy, 0, 0);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint64 Window::id() const
{
	return (uint64)pImpl->windowHandle;
}

void Window::msgBox(const char* text, const char* caption)
{
	MessageBoxA(pImpl->windowHandle, text, caption, 0);
}

void Window::raiseEvent(WindowEvent e, uint64 a, uint64 b)
{
	SendMessage(pImpl->windowHandle, EventCodes.GetWin32MessageEnum(e), a, b);
}

void Window::setTitle(const char* title)
{
	SetWindowTextA(pImpl->windowHandle, title);
}

void Window::invoke_internal(Window::IInvoker* i)
{
	SendMessageA(pImpl->windowHandle, WMO_INVOKE, (WPARAM)i, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Default event handlers
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::onCreate(WindowEventArgs e)
{
	defaultEventhandler(WM_CREATE, e.a, e.b);
}

void Window::onDestroy(WindowEventArgs e)
{
	PostQuitMessage(0);
}

void Window::onClose(WindowEventArgs e)
{
	this->raiseEvent(WindowEvent::eEventDestroy, 0, 0);
}

void Window::onResize(WindowEventArgs e)
{

}

void Window::onMouseMove(WindowEventArgs e)
{

}

void Window::onInput(WindowEventArgs e)
{

}

void Window::onActivate(WindowEventArgs e)
{

}

void Window::onDraw(WindowEventArgs e)
{
	defaultEventhandler(WM_PAINT, e.a, e.b);
}

void Window::onSetfocus(WindowEventArgs e)
{
	defaultEventhandler(WM_SETFOCUS, e.a, e.b);
}

void Window::onKillfocus(WindowEventArgs e)
{
	defaultEventhandler(WM_KILLFOCUS, e.a, e.b);
}

void Window::onKeydown(WindowEventArgs e)
{

}

void Window::onKeyup(WindowEventArgs e)
{

}

void Window::onScroll(WindowEventArgs e)
{

}

void Window::onMouseDown(WindowEventArgs e)
{

}

void Window::onMouseUp(WindowEventArgs e)
{

}

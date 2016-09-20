/*
	Window system
*/

#include <tsconfig.h>

#include <array>
#include <atomic>
#include <map>

#include <Windows.h>
#include <Windowsx.h> //todo: use the macros

#include "Window.h"
#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>
#include <tscore/system/thread.h>

#define USE_VISUAL_STYLES

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined USE_VISUAL_STYLES && defined TS_PLATFORM_WIN32

#pragma comment(linker,"/manifestdependency:\"type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

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

//Custom window event codes
#define TSM_CREATE (WM_USER + 0x0001)
#define TSM_INVOKE (WM_USER + 0x0002)
#define TSM_DESTROY WM_DESTROY


enum Win32flags
{
	win_registered = 1,
	win_open	   = 2,
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace ts;

//Maps a EWindowEvent enum to it's corresponding windows specific WM_* event code
static const class Win32EventEnums
{
private:

	array<uint32, EWindowEvent::EnumMax> m_windowEvents;

public:

	Win32EventEnums()
	{
		//Set up event table for matching enums
		m_windowEvents[EWindowEvent::eEventNull] = WM_NULL;
		m_windowEvents[EWindowEvent::eEventInput] = WM_INPUT;
		m_windowEvents[EWindowEvent::eEventActivate] = WM_ACTIVATE;
		m_windowEvents[EWindowEvent::eEventResize] = WM_SIZE;
		m_windowEvents[EWindowEvent::eEventCreate] = TSM_CREATE;
		m_windowEvents[EWindowEvent::eEventDestroy] = TSM_DESTROY;
		m_windowEvents[EWindowEvent::eEventClose] = WM_CLOSE;
		m_windowEvents[EWindowEvent::eEventDraw] = WM_PAINT;
		m_windowEvents[EWindowEvent::eEventSetfocus] = WM_SETFOCUS;
		m_windowEvents[EWindowEvent::eEventKillfocus] = WM_KILLFOCUS;
		m_windowEvents[EWindowEvent::eEventChar] = WM_CHAR;
		m_windowEvents[EWindowEvent::eEventKeydown] = WM_KEYDOWN;
		m_windowEvents[EWindowEvent::eEventKeyup] = WM_KEYUP;
		m_windowEvents[EWindowEvent::eEventScroll] = WM_MOUSEWHEEL;
		m_windowEvents[EWindowEvent::eEventMouseDown] = WM_LBUTTONDOWN;
		m_windowEvents[EWindowEvent::eEventMouseUp] = WM_LBUTTONUP;
		m_windowEvents[EWindowEvent::eEventMouseMove] = WM_MOUSEMOVE;
	}

	EWindowEvent GetWindowEventEnum(uint32 w32code) const
	{
		//return m_CWindowEvents.at(eventcodes);
		EWindowEvent eventcode = eEventNull;
		
		for (uint32 i = 0; i < m_windowEvents.size(); i++)
		{
			if (m_windowEvents[i] == w32code)
			{
				eventcode = (EWindowEvent)i;
				break;
			}
		}

		return eventcode;
	}

	uint32 GetWin32MessageEnum(EWindowEvent eventcode) const
	{
		return m_windowEvents.at(eventcode);
	}

} EventCodes;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct CWindow::Impl
{
	CWindow* window = nullptr;
	HWND windowHandle;
	WNDCLASSEX windowClass;
	HMODULE windowModule;

	string windowClassname;
	string windowTitle;

	SWindowRect size;

	mutex m_eventMutex;

	vector<CWindow::IEventListener*> windowEventListeners;

	atomic<int> flags = 0;


	Impl(CWindow* window, const SWindowDesc& desc) :
		window(window),
		windowClassname("tsAppWindow"),
		windowTitle(desc.title),
		size(desc.rect)
	{
		windowModule = (HMODULE)desc.appInstance;

		//Set win32 window class values
		ZeroMemory(&windowClass, sizeof(WNDCLASSEX));

		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;

		windowClass.hInstance = windowModule;
		windowClass.lpfnWndProc = (WNDPROC)&CWindow::Impl::WndProc;
		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

		windowClass.lpszClassName = windowClassname.c_str();
		windowClass.lpszMenuName = 0;

		windowClass.hbrBackground = HBRUSH(GetStockObject(WHITE_BRUSH));
		//windowClass.hbrBackground = 0;

		windowClass.hCursor = LoadCursor(0, IDC_ARROW);
		windowClass.hIcon = LoadIcon(0, IDI_APPLICATION);
		windowClass.hIconSm = 0;

		tsassert(RegisterClassEx(&windowClass));

		flags |= win_registered;
	}

	~Impl()
	{
		if (~flags & win_registered) return;

		//Destroy window if it is still open
		if (flags & win_open)
		{
			window->close();
		}

		tsassert(UnregisterClass(windowClassname.c_str(), windowModule));

		flags &= ~win_registered;
	}

	static LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		auto wnd = reinterpret_cast<CWindow::Impl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

		if (wnd)
		{
			if (msg == TSM_INVOKE)
			{
				auto f = (CWindow::IInvoker*)wparam;
				if (f) f->execute();
				return 0;
			}

			EWindowEvent code = EventCodes.GetWindowEventEnum(msg);

			//Only call the event listener for WM_* messages which have a corresponding EWindowEvent enum
			if (code != EWindowEvent::eEventNull)
			{
				SWindowEventArgs args;
				args.pWindow = wnd->window;
				args.eventcode = EventCodes.GetWindowEventEnum(msg);
				args.a = lparam;
				args.b = wparam;

				unique_lock<mutex> lk(wnd->m_eventMutex);
				auto ls = wnd->windowEventListeners;
				lk.unlock();

				for (CWindow::IEventListener* listener : ls)
				{
					if (listener != nullptr)
					{
						//If the return value is not equal to zero the event is marked as handled by the listener
						if (listener->onWindowEvent(args))
						{
							return 0;
						}
					}
				}

			}
		}
		
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	void open(int showCmd)
	{

#ifdef USE_VISUAL_STYLES
		tsassert(EnableVisualStyles());
#endif

		UINT styles = WS_VISIBLE | WS_OVERLAPPEDWINDOW;
		UINT exStyles = WS_EX_APPWINDOW;

		RECT r = { size.x, size.y, size.w, size.h };
		AdjustWindowRectEx(&r,
			styles,
			false,
			exStyles  
		);

		windowHandle = CreateWindowEx(
			exStyles,
			windowClassname.c_str(),
			windowTitle.c_str(),
			styles,
			//size.x,
			//size.y,
			//size.w,
			//size.h,
			r.left,
			r.top,
			r.right,
			r.bottom,
			0,
			0,
			windowModule,
			0
		);

		tsassert(IsWindow(windowHandle));

		ShowWindow(windowHandle, showCmd);

		flags |= win_open;

		SetWindowLongPtr(windowHandle, GWLP_USERDATA, LONG_PTR(this));
		SendMessage(windowHandle, TSM_CREATE, 0, 0);

		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));

		UpdateWindow(windowHandle);

		while (BOOL ret = GetMessage(&msg, NULL, 0, 0))
		{			
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		flags &= ~win_open;
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CWindow::CWindow(const SWindowDesc& desc) :
	pImpl(new Impl(this, desc))
{

}

CWindow::~CWindow()
{
	if (pImpl)
	{
		delete pImpl;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CWindow::addEventListener(IEventListener* listener)
{
	lock_guard<mutex>lk(pImpl->m_eventMutex);

	pImpl->windowEventListeners.push_back(listener);
}

void CWindow::removeEventListener(IEventListener* listener)
{
	lock_guard<mutex>lk(pImpl->m_eventMutex);

	auto& listeners = pImpl->windowEventListeners;
	auto it = find(listeners.begin(), listeners.end(), listener);

	if (it != listeners.end())
		pImpl->windowEventListeners.erase(it);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWindow::isOpen() const
{
	return ((pImpl->flags & win_open) != 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void CWindow::open(int showCmd)
{
	pImpl->open(showCmd);
}

void CWindow::close()
{
	raiseEvent(EWindowEvent::eEventClose, 0, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

intptr CWindow::handle() const
{
	return (uint64)pImpl->windowHandle;
}

void CWindow::messageBox(const char* text, const char* caption)
{
	MessageBoxA(pImpl->windowHandle, text, caption, 0);
}

void CWindow::raiseEvent(EWindowEvent e, uint64 a, uint64 b)
{
	SendMessage(pImpl->windowHandle, EventCodes.GetWin32MessageEnum(e), a, b);
}

void CWindow::invoke_internal(CWindow::IInvoker* i)
{
	SendMessage(pImpl->windowHandle, TSM_INVOKE, (WPARAM)i, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Window event helper functions
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	void getWindowResizeEventArgs(const SWindowEventArgs& args, uint32& w, uint32& h)
	{
		w = LOWORD(args.a);
		h = HIWORD(args.a);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

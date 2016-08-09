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

using namespace ts;

//Custom window event codes
#define TSM_CREATE (WM_USER + 0x0001)
#define TSM_INVOKE (WM_USER + 0x0002)
#define TSM_DESTROY WM_DESTROY


enum Win32flags
{
	win_registered = 1,
	win_open	   = 2,
	win_borderless = 4
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
	SWindowRect sizeCache;

	CWindow::IEventListener* windowEventListener = nullptr;

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
			
			if (code != EWindowEvent::eEventNull)
			{
				if (wnd->windowEventListener != nullptr)
				{
					SWindowEventArgs args;
					args.pWindow = wnd->window;
					args.eventcode = EventCodes.GetWindowEventEnum(msg);
					args.a = lparam;
					args.b = wparam;

					if (wnd->windowEventListener->onEvent(args) == 0)
					{
						return DefWindowProc(hwnd, msg, wparam, lparam);
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

		windowHandle = CreateWindowEx(
			WS_EX_APPWINDOW,
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

void CWindow::setEventListener(IEventListener* listener)
{
	//todo: actually make thread safe
	pImpl->windowEventListener = listener;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CWindow::setFullscreen(bool on)
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

bool CWindow::isFullscreen() const
{
	return ((pImpl->flags & win_borderless) != 0);
}

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

intptr CWindow::id() const
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
	SendMessageA(pImpl->windowHandle, TSM_INVOKE, (WPARAM)i, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
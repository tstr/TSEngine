/*
	Window system
*/

#include <tsconfig.h>

#include <array>
#include <map>
#include <list>

#include <Windows.h>
#include <Windowsx.h> //todo: use the macros

#include <tsengine/platform/window.h>
#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>

//#define USE_VISUAL_STYLES

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
#define TSM_INVOKE (WM_USER + 0x0001)

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
		m_windowEvents[EWindowEvent::eEventCreate] = WM_CREATE;
		m_windowEvents[EWindowEvent::eEventDestroy] = WM_DESTROY;
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

	list<CWindow::IEventListener*> windowEventListeners;
	
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

		//Register the window class
		tsassert(RegisterClassExA(&windowClass));
	}

	~Impl()
	{
		tsassert(UnregisterClassA(windowClassname.c_str(), windowModule));
	}

	static LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		//If window was just created, set user data to value in create parameters
		if (msg == WM_CREATE)
		{
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((LPCREATESTRUCT)lparam)->lpCreateParams);
		}
		//Manually ensure message pump exits
		else if (msg == WM_DESTROY)
		{
			PostQuitMessage(0);
		}

		//Get window ptr from user data
		if (auto wnd = reinterpret_cast<CWindow::Impl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)))
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

				//Copy event listeners
				auto ls = wnd->windowEventListeners;

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

		RECT r = {
			(LONG)size.x,
			(LONG)size.y,
			//Convert width/height into coordinates of bottom right corner
			(LONG)size.x + (LONG)size.w,
			(LONG)size.y + (LONG)size.h
		};

		//Set size of the client area of the window - prevents visual artifacting
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
			r.left,
			r.top,
			//Convert coordinates of bottom right corner back into width and height of window
			r.right - r.left,
			r.bottom - r.top,
			0,
			0,
			windowModule,
			this
		);

		tsassert(IsWindow(windowHandle));

		ShowWindow(windowHandle, showCmd);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CWindow::CWindow(const SWindowDesc& desc) :
	pImpl(new Impl(this, desc))
{

}

CWindow::~CWindow()
{
	delete pImpl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CWindow::addEventListener(IEventListener* listener)
{
	auto f = [this, &listener]() {
			pImpl->windowEventListeners.push_back(listener);
	};

	if (isOpen())
	{
		invoke(f);
	}
	else
	{
		f();
	}
}

void CWindow::removeEventListener(IEventListener* listener)
{
	auto f = [this, &listener]() {
		auto& listeners = pImpl->windowEventListeners;
		auto it = find(listeners.begin(), listeners.end(), listener);

		if (it != listeners.end())
			pImpl->windowEventListeners.erase(it);
	};

	if (isOpen())
	{
		invoke(f);
	}
	else
	{
		f();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CWindow::open(int showCmd)
{
	tsassert(pImpl);
	pImpl->open(showCmd);
}

void CWindow::close()
{
	tsassert(pImpl);
	DestroyWindow(pImpl->windowHandle);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CWindow::isOpen() const
{
	tsassert(pImpl);
	return IsWindow(pImpl->windowHandle) != 0;
}

int CWindow::handleEvents()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	BOOL ret = 0;

	while (ret = PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			return eQueueExit;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (ret != 0) ? eQueueMessagePresent : eQueueMessageEmpty;
}

intptr CWindow::nativeHandle() const
{
	return (intptr)pImpl->windowHandle;
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

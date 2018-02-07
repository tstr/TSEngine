/*
	Window system
*/

#include <tsconfig.h>

#include <array>
#include <map>
#include <list>

#include <Windows.h>
#include <Windowsx.h> //todo: use the macros

#include "Window.h"

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct Window::Impl
{
	Window* window = nullptr;
	HWND windowHandle;
	WNDCLASSEX windowClass;
	HMODULE windowModule;

	string windowClassname;
	string windowTitle;

	WindowRect size;
	
	Impl(Window* window, const WindowInfo& info) :
		window(window),
		windowClassname("tsAppWindow"),
		windowTitle(info.title),
		size(info.rect)
	{
		windowModule = (HMODULE)GetModuleHandle(0);

		//Set win32 window class values
		ZeroMemory(&windowClass, sizeof(WNDCLASSEX));

		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;

		windowClass.hInstance = windowModule;
		windowClass.lpfnWndProc = (WNDPROC)&Window::Impl::WndProc;
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
		if (auto wnd = reinterpret_cast<Window::Impl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA)))
		{
			if (msg == TSM_INVOKE)
			{
				auto f = (Window::IInvoker*)wparam;
				if (f) f->execute();
				return 0;
			}

			switch (msg)
			{
			case WM_SIZE:
				//wnd->window->onResize(LOWORD(lparam), HIWORD(lparam));
				wnd->window->onResize();
				break;

			case WM_PAINT:
				wnd->window->onRedraw();
				break;

			case WM_SETFOCUS:
				wnd->window->onActivate();
				break;

			case WM_KILLFOCUS:
				wnd->window->onDeactivate();
				break;

			case WM_CREATE:
				wnd->window->onCreate();
				break;

			case WM_CLOSE:
				wnd->window->onClose();
				break;

			case WM_DESTROY:
				wnd->window->onDestroy();
				break;

			case TSM_INVOKE:
				if (auto f = (Window::IInvoker*)wparam)
					f->execute();
				return 0;

			default:

				PlatformEventArgs arg;
				arg.code = msg;
				arg.a = lparam;
				arg.b = wparam;

				wnd->window->onEvent(arg);
			}
		}

		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	void open()
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

		//ShowWindow(windowHandle, SW_SHOWDEFAULT);
	}
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Window::Window(const WindowInfo& info) :
	pImpl(new Impl(this, info))
{

}

Window::~Window()
{
	delete pImpl;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::open()
{
	tsassert(pImpl);
	pImpl->open();
}

void Window::close()
{
	tsassert(pImpl);
	DestroyWindow(pImpl->windowHandle);
}

bool Window::isOpen() const
{
	tsassert(pImpl);
	return IsWindow(pImpl->windowHandle) != 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Window::poll()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	BOOL ret = 0;

	while (ret = PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			return 0;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (ret != 0) ? 2 : 1;
}

void Window::invokeImpl(Window::IInvoker* i)
{
	tsassert(pImpl);
	SendMessage(pImpl->windowHandle, TSM_INVOKE, (WPARAM)i, 0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ISurface overrides
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

intptr Window::getHandle() const
{
	tsassert(pImpl);
	return (intptr)pImpl->windowHandle;
}

void Window::enableBorderless(bool enable)
{
	tsassert(pImpl);
	const HWND hwnd = pImpl->windowHandle;

	if (enable && !isBorderless())
	{
		Window::invoke([=]() {

			//Set borderless mode
			DEVMODE dev;
			ZeroMemory(&dev, sizeof(DEVMODE));

			int width = GetSystemMetrics(SM_CXSCREEN);
			int	height = GetSystemMetrics(SM_CYSCREEN);

			EnumDisplaySettings(NULL, 0, &dev);

			HDC context = GetWindowDC(hwnd);
			int colourBits = GetDeviceCaps(context, BITSPIXEL);
			int refreshRate = GetDeviceCaps(context, VREFRESH);

			dev.dmPelsWidth = width;
			dev.dmPelsHeight = height;
			dev.dmBitsPerPel = colourBits;
			dev.dmDisplayFrequency = refreshRate;
			dev.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;

			//todo: fix error with changing settings
			//LONG result = ChangeDisplaySettingsA(&dev, CDS_FULLSCREEN);// == DISP_CHANGE_SUCCESSFUL);
			//tserror("ChangeDisplaySettings returned %", result);

			SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
			SetWindowPos(hwnd, HWND_TOP, 0, 0, width, height, SWP_SHOWWINDOW);
			BringWindowToTop(hwnd);
		});
	}
	else if (!enable && isBorderless())
	{
		Window::invoke([=]() {

			//Exit borderless mode
			SetWindowLongPtr(hwnd, GWL_EXSTYLE, WS_EX_LEFT);
			SetWindowLongPtr(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);

			ShowWindow(hwnd, SW_MAXIMIZE);			
		});
	}
}

bool Window::isBorderless() const
{
	tsassert(pImpl);

	LONG_PTR style = GetWindowLongPtr(pImpl->windowHandle, GWL_STYLE);

	return (style & (WS_POPUP | WS_VISIBLE)) == (WS_POPUP | WS_VISIBLE);
}

bool Window::supportsBorderless() const
{
	//Window class does support this feature
	return true;
}

void Window::resize(uint width, uint height)
{
	tsassert(pImpl);

	if (!isBorderless())
	{
		Window::invoke([=]() {
			
			RECT r;

			const HWND hwnd = pImpl->windowHandle;

			//Get current size of client area
			//GetWindowRect(hwnd, &r);

			r.left = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
			r.top = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;
			r.right = width;
			r.bottom = height;

			LONG_PTR styles = GetWindowLongPtr(hwnd, GWL_STYLE);
			LONG_PTR exStyles = GetWindowLongPtr(hwnd, GWL_EXSTYLE);

			//Convert client area to window rect
			AdjustWindowRectEx(
				&r,
				styles,
				false,
				exStyles
			);

			//Set new window rect
			SetWindowPos(
				hwnd,
				HWND_TOP,
				r.left,
				r.top,
				r.right,
				r.bottom,
				SWP_SHOWWINDOW
			);
		});
	}
}

void Window::getSize(uint& w, uint& h) const
{
	tsassert(pImpl);
	
	RECT r;
	GetClientRect(pImpl->windowHandle, &r);

	w = r.right - r.left;
	h = r.bottom - r.top;
}

void Window::redraw()
{
	tsassert(pImpl);
	
	RedrawWindow(pImpl->windowHandle, NULL, NULL, RDW_UPDATENOW);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Default event handlers
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Window::onCreate() {}
void Window::onSuspend() {}

void Window::onClose() {}
void Window::onDestroy() {}

void Window::onActivate() {}
void Window::onDeactivate() {}

void Window::onResize() {}
void Window::onRedraw() {}

void Window::onEvent(const PlatformEventArgs& arg) {}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

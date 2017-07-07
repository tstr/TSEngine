/*
	Input System source
*/

#include <tsengine/Input.h>
#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>
#include "platform/Window.h"

#include <Windows.h>

using namespace ts;
using namespace std;

#define RAW_INPUT_DEVICE_ID_MOUSE	 0x2
#define RAW_INPUT_DEVICE_ID_KEYBOARD 0x6

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//Constructor
/////////////////////////////////////////////////////////////////////////////////////////////////////////

InputSystem::InputSystem(Window* window, uint flags) :
	m_window(window),
	m_flags(flags),
	m_mouseX(0),
	m_mouseY(0)
{
	//Verify window is valid
	tsassert(m_window != nullptr);

	HWND hwnd = (HWND)m_window->getHandle();

	tsassert(IsWindow(hwnd));

	RAWINPUTDEVICE devices[2];

	//Raw input mouse device
	devices[0].usUsagePage = 0x01;
	devices[0].usUsage = RAW_INPUT_DEVICE_ID_MOUSE;
	devices[0].dwFlags = RIDEV_DEVNOTIFY;
	devices[0].hwndTarget = hwnd;

	//Raw input keyboard device
	devices[1].usUsagePage = 0x01;
	devices[1].usUsage = RAW_INPUT_DEVICE_ID_KEYBOARD;
	devices[1].dwFlags = RIDEV_DEVNOTIFY;// | RIDEV_NOLEGACY; //Ignores WM_KEYDOWN/WM_KEYUP messages
	devices[1].hwndTarget = hwnd;

	//Register devices so that our window can recieve WM_INPUT messages
	if (!RegisterRawInputDevices(devices, ARRAYSIZE(devices), sizeof(RAWINPUTDEVICE)))
	{
		tswarn("A raw input device could not be registered.");
		return;
	}
}

InputSystem::~InputSystem()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Event Listeners
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void InputSystem::addListener(IListener* listener)
{
	if (listener != nullptr)
	{
		m_eventListeners.push_back(listener);
	}
}

void InputSystem::removeListener(IListener* listener)
{
	using namespace std;

	auto it = find(m_eventListeners.begin(), m_eventListeners.end(), listener);

	if (it != m_eventListeners.end())
	{
		m_eventListeners.erase(it);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Mouse cursor methods
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void InputSystem::showCursor(bool show)
{
	if (show != m_cursorShown)
	{
		m_window->invoke([=]()
		{
			::ShowCursor(show);

			if (show)
			{
				::ClipCursor(nullptr);
			}
			else
			{
				auto hwnd = (HWND)m_window->getHandle();

				RECT rect;
				::GetWindowRect(hwnd, &rect);
				::ClipCursor(&rect);

				POINT p;
				GetCursorPos(&p);

				m_mouseY = p.y;
				m_mouseX = p.x;
			}
		});

		m_cursorShown = show;
	}
}

bool InputSystem::cursorShown() const
{
	return m_cursorShown;
}

void InputSystem::getCursorPos(uint& x, uint& y) const
{
	POINT p;
	::GetCursorPos(&p);
	::ScreenToClient((HWND)m_window->getHandle(), &p);
	x = (uint)p.x;
	y = (uint)p.y;
}

void InputSystem::setCursorPos(uint x, uint y)
{
	POINT p;
	p.x = x;
	p.y = y;
	::ClientToScreen((HWND)m_window->getHandle(), &p);
	::SetCursorPos(p.x, p.y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
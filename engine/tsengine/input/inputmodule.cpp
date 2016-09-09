/*
	Input module source
*/

#include <tsconfig.h>
#include "inputmodule.h"
#include "inputdevice.h"

#include <Windows.h>

using namespace ts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//Constructor
/////////////////////////////////////////////////////////////////////////////////////////////////////////

CInputModule::CInputModule(CWindow* window) :
	m_window(window),
	m_device(window)
{
	m_device.setInputCallback(CInputDevice::Callback::fromMethod<CInputModule, &CInputModule::inputLayerCallback>(this));
}

CInputModule::~CInputModule()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//Event
/////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CInputModule::addEventListener(IInputEventListener* listener)
{
	if (listener == nullptr)
		return false;

	m_eventListeners.push_back(listener);
	return true;
}

bool CInputModule::removeEventListener(IInputEventListener* listener)
{
	using namespace std;

	auto it = find(m_eventListeners.begin(), m_eventListeners.end(), listener);
	if (it == m_eventListeners.end())
		return false;

	m_eventListeners.erase(it);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void CInputModule::inputLayerCallback(const SInputEvent& event)
{
	if (event.type == EInputEventType::eInputEventMouse)
	{
		for (auto& l : m_eventListeners)
			l->onMouse(event.mouse.deltaX, event.mouse.deltaY);
	}
	else if (event.type == EInputEventType::eInputEventKeyboard)
	{
		if(event.key.isKeyUp)
		{
			for (auto& l : m_eventListeners)
				l->onKeyUp(event.key.keycode);
		}
		else
		{
			for (auto& l : m_eventListeners)
				l->onKeyDown(event.key.keycode);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

void CInputModule::showCursor(bool show)
{
	if (show != m_cursorShown)
	{
		m_window->invoke([=]() {
			::ShowCursor(show);

			if (show)
			{
				ClipCursor(nullptr);
			}
			else
			{
				auto hwnd = (HWND)m_window->handle();
				//POINT p;
				//ClientToScreen(hwnd, &p);
				RECT rect;
				GetWindowRect(hwnd, &rect);
				ClipCursor(&rect);

				::ClipCursor(&rect);
			}
		});
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
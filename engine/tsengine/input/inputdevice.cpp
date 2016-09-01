/*
	Low level input device source
*/

#include <tsconfig.h>
#include "inputdevice.h"

#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>
#include <tscore/system/thread.h>

#ifndef TS_PLATFORM_WIN32
#error This source file can only be compiled on windows
#endif

#include <Windows.h>

using namespace std;
using namespace ts;

#define RAW_INPUT_DEVICE_ID_MOUSE	 0x2
#define RAW_INPUT_DEVICE_ID_KEYBOARD 0x6

//#define DEBUG_PRINT_KEYS

///////////////////////////////////////////////////////////////////////////////////////////
//Constructors
///////////////////////////////////////////////////////////////////////////////////////////
CInputDevice::CInputDevice(CWindow* window) :
	m_pWindow(window)
{
	auto hwnd = (HWND)m_pWindow->handle();
	
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
	devices[1].dwFlags = RIDEV_DEVNOTIFY | RIDEV_NOLEGACY; //Ignores WM_KEYDOWN/WM_KEYUP messages
	devices[1].hwndTarget = hwnd;

	if (!RegisterRawInputDevices(devices, 2, sizeof(RAWINPUTDEVICE)))
	{
		tswarn("A raw input device could not be registered.");
		return;
	}

	thread(&CInputDevice::messageProcedure, this).detach();
}

CInputDevice::~CInputDevice()
{
	m_messageReciever.post(SInputMessage(EInputMessage::eMessageExit));
	lock_guard<mutex>lk(m_exitMutex);
}

///////////////////////////////////////////////////////////////////////////////////////////
//Internal message processor - all input messages are handled on a separate thread to the
// win32 window message thread.
///////////////////////////////////////////////////////////////////////////////////////////
void CInputDevice::messageProcedure()
{
	lock_guard<mutex>lk(m_exitMutex);

	SInputMessage msg;

	while(true)
	{
		m_messageReciever.get(msg);

		if (msg.code == EInputMessage::eMessageExit)
			return;

		if (msg.code == EInputMessage::eMessageInput)
		{
			if (m_inputCallback != nullptr)
				m_inputCallback(msg.event);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//Win32 window messages are processed by this method and dispatched to a separate message
// queue owned by the input module
///////////////////////////////////////////////////////////////////////////////////////////
bool CInputDevice::onWindowInputEvent(const SWindowEventArgs& args)
{
	if (args.eventcode == EWindowEvent::eEventInput)
	{
		UINT size = 128;
		char rawinputBuffer[128];

		//Get the raw input data
		GetRawInputData((HRAWINPUT)args.a, RID_INPUT, (void*)rawinputBuffer, &size, sizeof(RAWINPUTHEADER));

		auto Raw = reinterpret_cast<RAWINPUT*>(rawinputBuffer);

		SInputMessage msg;
		msg.code = EInputMessage::eMessageInput;

		//Check if input device is mouse
		if (Raw->header.dwType == RIM_TYPEMOUSE)
		{
			auto mdata = Raw->data.mouse;

			msg.event.mouse.deltaX = (int16)mdata.lLastX;
			msg.event.mouse.deltaY = (int16)mdata.lLastY;
			msg.event.type = EInputEventType::eInputEventMouse;

			//tswarn("(x:%) (y:%) %", mdata.lLastX, mdata.lLastY, mdata.ulButtons);

		}
		else if (Raw->header.dwType == RIM_TYPEKEYBOARD)
		{
			auto kdata = Raw->data.keyboard;

			EKeyCode enumKey = eKeyUnknown;

			USHORT virtualKey = kdata.VKey;
			USHORT scanCode = kdata.MakeCode;
			USHORT flags = kdata.Flags;

			/////////////////////////////////////////////////////////////////////////////////////////////////////
			//Ignore this part it is a work around for windows silliness
			/////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma region hacks

			if (virtualKey == 255)
			{
				return false;
			}
			else if (virtualKey == VK_SHIFT)
			{
				virtualKey = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);
			}
			else if (virtualKey == VK_NUMLOCK)
			{
				// correct PAUSE/BREAK and NUM LOCK silliness, and set the extended bit
				scanCode = (MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC) | 0x100);
			}

			//Check if e0 or e1 bits are present
			const bool isE0 = ((flags & RI_KEY_E0) != 0);
			const bool isE1 = ((flags & RI_KEY_E1) != 0);

			if (isE1)
			{
				// for escaped sequences, turn the virtual key into the correct scan code using MapVirtualKey.
				// however, MapVirtualKey is unable to map VK_PAUSE (this is a known bug), hence we map that by hand.
				if (virtualKey == VK_PAUSE)
					scanCode = 0x45;
				else
					scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
			}

			switch (virtualKey)
			{
				// right-hand CONTROL and ALT have their e0 bit set
			case VK_CONTROL:
				if (isE0)
					virtualKey = VK_RCONTROL;
				else
					virtualKey = VK_LCONTROL;
				break;

			case VK_MENU:
				if (isE0)
					virtualKey = VK_RMENU;
				else
					virtualKey = VK_LMENU;
				break;

				/*
				// NUMPAD ENTER has its e0 bit set
			case VK_RETURN:
				if (isE0)
					virtualKey = 0x0;
				break;
				/*/
				
			// the standard INSERT, DELETE, HOME, END, PRIOR and NEXT keys will always have their e0 bit set, but the
			// corresponding keys on the NUMPAD will not.
			case VK_INSERT:
				if (!isE0)
					virtualKey = VK_NUMPAD0;
				break;

			case VK_DELETE:
				if (!isE0)
					virtualKey = VK_DECIMAL;
				break;

			case VK_HOME:
				if (!isE0)
					virtualKey = VK_NUMPAD7;
				break;

			case VK_END:
				if (!isE0)
					virtualKey = VK_NUMPAD1;
				break;

			case VK_PRIOR:
				if (!isE0)
					virtualKey = VK_NUMPAD9;
				break;

			case VK_NEXT:
				if (!isE0)
					virtualKey = VK_NUMPAD3;
				break;

				// the standard arrow keys will always have their e0 bit set, but the
				// corresponding keys on the NUMPAD will not.
			case VK_LEFT:
				if (!isE0)
					virtualKey = VK_NUMPAD4;
				break;

			case VK_RIGHT:
				if (!isE0)
					virtualKey = VK_NUMPAD6;
				break;

			case VK_UP:
				if (!isE0)
					virtualKey = VK_NUMPAD8;
				break;

			case VK_DOWN:
				if (!isE0)
					virtualKey = VK_NUMPAD2;
				break;

				// NUMPAD 5 doesn't have its e0 bit set
			case VK_CLEAR:
				if (!isE0)
					virtualKey = VK_NUMPAD5;
				break;
			}

#pragma endregion

			/////////////////////////////////////////////////////////////////////////////////////////////////////

			const bool keyUp = ((flags & RI_KEY_BREAK) != 0);

			msg.event.key.keycode = m_keyTable.mapFromVirtualKey(virtualKey);
			msg.event.key.isKeyUp = keyUp;
			msg.event.type = EInputEventType::eInputEventKeyboard;

#ifdef DEBUG_PRINT_KEYS
			UINT key = (scanCode << 16) | (isE0 << 24);
			char buffer[512] = {};
			GetKeyNameText((LONG)key, buffer, 512);
			tswarn("% %", keyUp, buffer);

			CKeyTable::KeyName name;
			m_keyTable.getKeyName(msg.event.key.keycode, name);
			tswarn(name.str());
#endif

			/////////////////////////////////////////////////////////////////////////////////////////////////////
		}
		else if (Raw->header.dwType == RIM_TYPEHID)
		{
			//do nothing
			return false;
		}

		m_messageReciever.post(msg);
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////
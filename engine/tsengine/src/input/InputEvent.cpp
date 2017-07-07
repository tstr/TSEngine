/*
	Input System - Event
	
	Translating Window Event information to usable Input Event information
*/

#include <tsengine/Input.h>
#include <tscore/debug/assert.h>
#include <tscore/debug/log.h>
#include "platform/Window.h"

#include <Windows.h>

using namespace ts;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Window event handler
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void InputSystem::onEvent(const PlatformEventArgs& args)
{
	if (args.code == WM_CHAR)
	{
		if (args.b > 0 && args.b < 0x10000)
		{
			dispatch(&IListener::onChar, (wchar)args.b);
		}
	}
	else if (args.code == WM_INPUT)
	{
		//Translate event
		InputEvent event = translate(args);

		//Log input
		const bool doLog = m_flags & eFlagEnableLogging;

		//If event is a mouse event
		if (event.device == eInputDeviceMouse)
		{
			//If cursor is disabled then fix it in place
			if (!m_cursorShown)
			{
				SetCursorPos(m_mouseX, m_mouseY);
			}

			//Mouse scroll
			if (event.deltaScroll != 0.0f)
			{
				dispatch(&IListener::onMouseScroll, (int)event.deltaScroll);

				if (doLog)
					tsprofile("[Scroll] %", event.deltaScroll);
			}

			//Mouse move
			if (event.deltaX || event.deltaY)
			{
				dispatch(&IListener::onMouseMove, (int)event.deltaX, (int)event.deltaY);

				if (doLog)
					tsprofile("[Move] %  %", event.deltaX, event.deltaY);
			}
		}

		//On keyboard press or mouse click
		if (event.keyCode != EKeyCode::eKeyUnknown)
		{
			if (event.isUp)
			{
				dispatch(&IListener::onKeyUp, event.keyCode);
			}
			else
			{
				dispatch(&IListener::onKeyDown, event.keyCode);
			}

			if (doLog)
				tsprofile("% \"%\"", (event.isUp) ? "[KeyUp]  " : "[KeyDown]", keys::getKeyName(event.keyCode));
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
	Device Event translators -

	Takes a given RAWINPUT structure and fills out a given InputEvent structure
*/
typedef bool(*InputDeviceEventTranslator)(RAWINPUT*, InputEvent&);

static bool translateMouseEvent(RAWINPUT* input, InputEvent& event);
static bool translateKeyboardEvent(RAWINPUT* input, InputEvent& event);
static bool translateHIDEvent(RAWINPUT* input, InputEvent& event);

InputEvent InputSystem::translate(const PlatformEventArgs& args)
{
	UINT size = 128;
	char rawinputBuffer[128];

	InputEvent event;

	event.device = eInputDeviceOther;
	event.keyCode = eKeyUnknown;

	//Get the raw input data
	if (GetRawInputData((HRAWINPUT)args.a, RID_INPUT, (void*)rawinputBuffer, &size, sizeof(RAWINPUTHEADER)) > 0)
	{
		auto raw = reinterpret_cast<RAWINPUT*>(rawinputBuffer);

		static const InputDeviceEventTranslator functionTable[] =
		{
			translateMouseEvent,	//RIM_TYPEMOUSE
			translateKeyboardEvent,	//RIM_TYPEKEYBOARD
			translateHIDEvent		//RIM_TYPEHID
		};

		//Lookup and call correct device translator in table
		if (!functionTable[raw->header.dwType](raw, event))
		{
			//If there was a problem with the translation, reset event state to default values
			event.device = eInputDeviceOther;
			event.keyCode = eKeyUnknown;
		}
	}

	return event;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Translate Mouse Raw Input
/////////////////////////////////////////////////////////////////////////////////////////////////////////

bool translateMouseEvent(RAWINPUT* raw, InputEvent& event)
{
	event.device = EInputDevice::eInputDeviceMouse;

	auto mdata = raw->data.mouse;

	EKeyCode& button = event.keyCode;
	bool& up = event.isUp;

	switch (mdata.usButtonFlags)
	{
	case (RI_MOUSE_LEFT_BUTTON_DOWN): { button = eMouseButtonLeft; up = false; break; }
	case (RI_MOUSE_LEFT_BUTTON_UP): { button = eMouseButtonLeft; up = true; break; }

	case (RI_MOUSE_RIGHT_BUTTON_DOWN): { button = eMouseButtonRight; up = false; break; }
	case (RI_MOUSE_RIGHT_BUTTON_UP): { button = eMouseButtonRight; up = true; break; }

	case (RI_MOUSE_MIDDLE_BUTTON_DOWN): { button = eMouseButtonMiddle; up = false; break; }
	case (RI_MOUSE_MIDDLE_BUTTON_UP): { button = eMouseButtonMiddle; up = true; break; }

	case (RI_MOUSE_BUTTON_4_DOWN): { button = eMouseXbutton1; up = false; break; }
	case (RI_MOUSE_BUTTON_4_UP): { button = eMouseXbutton1; up = true; break; }

	case (RI_MOUSE_BUTTON_5_DOWN): { button = eMouseXbutton2; up = false; break; }
	case (RI_MOUSE_BUTTON_5_UP): { button = eMouseXbutton2; up = true; break; }

	default: { button = eKeyUnknown; up = false; }
	}

	event.deltaScroll = 0;

	if (mdata.usButtonFlags == RI_MOUSE_WHEEL)
	{
		event.deltaScroll = (short)mdata.usButtonData / WHEEL_DELTA;
	}

	event.deltaX = (int16)mdata.lLastX;
	event.deltaY = (int16)mdata.lLastY;

	//tswarn("(x:%) (y:%) %", mdata.lLastX, mdata.lLastY, mdata.ulButtons);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Translate Keyboard Raw Input
/////////////////////////////////////////////////////////////////////////////////////////////////////////

bool translateKeyboardEvent(RAWINPUT* raw, InputEvent& event)
{
	event.device = EInputDevice::eInputDeviceKeyboard;

	auto kdata = raw->data.keyboard;

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

	event.keyCode = keys::mapFromVirtualKey(virtualKey);
	event.isUp = keyUp;
	

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

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Translate HID raw input - currently unused
/////////////////////////////////////////////////////////////////////////////////////////////////////////

bool translateHIDEvent(RAWINPUT* raw, InputEvent& event)
{
	event.device = EInputDevice::eInputDeviceOther;
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

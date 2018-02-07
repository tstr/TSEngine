/*
	Keycodes

	todo: fix NUMPAD minus and NUMPAD enter keys
*/

#include <tscore/strings.h>
#include <tsengine/KeyCodes.h>
#include <array>

#include <Windows.h>

using namespace std;
using namespace ts;

enum VirtualKeyCodes
{
	vk_BackSpace = 8,
	vk_Tab = 9,
	vk_Return = 13,
	vk_Command = 15,
	vk_Shift = 16,
	vk_Control = 17,
	vk_Alt = 18,
	vk_Pause = 19,
	vk_CapsLock = 20,
	vk_Escape = 27,
	vk_Space = 32,
	vk_PageUp = 33,
	vk_PageDown = 34,
	vk_End = 35,
	vk_Home = 36,
	vk_Left = 37,
	vk_Up = 38,
	vk_Right = 39,
	vk_Down = 40,
	vk_PrintScreen = 44,
	vk_Insert = 45,
	vk_Delete = 46,

	vk_0 = 48,
	vk_1 = 49,
	vk_2 = 50,
	vk_3 = 51,
	vk_4 = 52,
	vk_5 = 53,
	vk_6 = 54,
	vk_7 = 55,
	vk_8 = 56,
	vk_9 = 57,

	vk_A = 65,
	vk_B = 66,
	vk_C = 67,
	vk_D = 68,
	vk_E = 69,
	vk_F = 70,
	vk_G = 71,
	vk_H = 72,
	vk_I = 73,
	vk_J = 74,
	vk_K = 75,
	vk_L = 76,
	vk_M = 77,
	vk_N = 78,
	vk_O = 79,
	vk_P = 80,
	vk_Q = 81,
	vk_R = 82,
	vk_S = 83,
	vk_T = 84,
	vk_U = 85,
	vk_V = 86,
	vk_W = 87,
	vk_X = 88,
	vk_Y = 89,
	vk_Z = 90,

	vk_LWin = 91,
	vk_RWin = 92,
	vk_Apps = 93,

	vk_NumPad0 = 96,
	vk_NumPad1 = 97,
	vk_NumPad2 = 98,
	vk_NumPad3 = 99,
	vk_NumPad4 = 100,
	vk_NumPad5 = 101,
	vk_NumPad6 = 102,
	vk_NumPad7 = 103,
	vk_NumPad8 = 104,
	vk_NumPad9 = 105,

	vk_Multiply = 106,
	vk_Add = 107,
	vk_Subtract = 109,
	vk_Decimal = 110,
	vk_Divide = 111,

	vk_F1 = 112,
	vk_F2 = 113,
	vk_F3 = 114,
	vk_F4 = 115,
	vk_F5 = 116,
	vk_F6 = 117,
	vk_F7 = 118,
	vk_F8 = 119,
	vk_F9 = 120,
	vk_F10 = 121,
	vk_F11 = 122,
	vk_F12 = 123,
	vk_F13 = 124,
	vk_F14 = 125,
	vk_F15 = 126,
	vk_F16 = 127,

	vk_NumLock = 144,
	vk_ScrollLock = 145,
	vk_LShift = 160,
	vk_RShift = 161,
	vk_LControl = 162,
	vk_RControl = 163,
	vk_LAlt = 164,
	vk_RAlt = 165,
	vk_SemiColon = 186,
	vk_Equals = 187,
	vk_Comma = 188,
	vk_UnderScore = 189,
	vk_Period = 190,
	vk_Slash = 191,
	vk_BackSlash = 220,
	vk_RightBrace = 221,
	vk_LeftBrace = 219,

	//These might be the wrong way around
	vk_Apostrophe = 192,
	vk_Pound = 222,

	vk_Grave = 223
};


///////////////////////////////////////////////////////////////////////////////////////////
//Win32 key code mappings
///////////////////////////////////////////////////////////////////////////////////////////
static const struct KeyTable
{
	KeyTable()
	{
		/*
			Key codes
		*/

		const uint32 unused = 0xffffffff;

		keys[EKeyCode::eKeyEsc] = vk_Escape;

		keys[EKeyCode::eKeyA] = vk_A;
		keys[EKeyCode::eKeyB] = vk_B;
		keys[EKeyCode::eKeyC] = vk_C;
		keys[EKeyCode::eKeyD] = vk_D;
		keys[EKeyCode::eKeyE] = vk_E;
		keys[EKeyCode::eKeyF] = vk_F;
		keys[EKeyCode::eKeyG] = vk_G;
		keys[EKeyCode::eKeyH] = vk_H;
		keys[EKeyCode::eKeyI] = vk_I;
		keys[EKeyCode::eKeyJ] = vk_J;
		keys[EKeyCode::eKeyK] = vk_K;
		keys[EKeyCode::eKeyL] = vk_L;
		keys[EKeyCode::eKeyM] = vk_M;
		keys[EKeyCode::eKeyN] = vk_N;
		keys[EKeyCode::eKeyO] = vk_O;
		keys[EKeyCode::eKeyP] = vk_P;
		keys[EKeyCode::eKeyQ] = vk_Q;
		keys[EKeyCode::eKeyR] = vk_R;
		keys[EKeyCode::eKeyS] = vk_S;
		keys[EKeyCode::eKeyT] = vk_T;
		keys[EKeyCode::eKeyU] = vk_U;
		keys[EKeyCode::eKeyV] = vk_V;
		keys[EKeyCode::eKeyW] = vk_W;
		keys[EKeyCode::eKeyZ] = vk_Z;
		keys[EKeyCode::eKeyY] = vk_Y;
		keys[EKeyCode::eKeyX] = vk_X;

		keys[EKeyCode::eKeyF1] = vk_F1;
		keys[EKeyCode::eKeyF2] = vk_F2;
		keys[EKeyCode::eKeyF3] = vk_F3;
		keys[EKeyCode::eKeyF4] = vk_F4;
		keys[EKeyCode::eKeyF5] = vk_F5;
		keys[EKeyCode::eKeyF6] = vk_F6;
		keys[EKeyCode::eKeyF7] = vk_F7;
		keys[EKeyCode::eKeyF8] = vk_F8;
		keys[EKeyCode::eKeyF9] = vk_F9;
		keys[EKeyCode::eKeyF10] = vk_F10;
		keys[EKeyCode::eKeyF11] = vk_F11;
		keys[EKeyCode::eKeyF12] = vk_F12;
		keys[EKeyCode::eKeyF13] = vk_F13;
		keys[EKeyCode::eKeyF14] = vk_F14;
		keys[EKeyCode::eKeyF15] = vk_F15;
		keys[EKeyCode::eKeyF16] = vk_F16;

		keys[EKeyCode::eKey0] = vk_0;
		keys[EKeyCode::eKey1] = vk_1;
		keys[EKeyCode::eKey2] = vk_2;
		keys[EKeyCode::eKey3] = vk_3;
		keys[EKeyCode::eKey4] = vk_4;
		keys[EKeyCode::eKey5] = vk_5;
		keys[EKeyCode::eKey6] = vk_6;
		keys[EKeyCode::eKey7] = vk_7;
		keys[EKeyCode::eKey8] = vk_8;
		keys[EKeyCode::eKey9] = vk_9;

		keys[EKeyCode::eKeyMinus] = vk_UnderScore;
		keys[EKeyCode::eKeyEquals] = vk_Equals;

		keys[EKeyCode::eKeyGrave] = vk_Grave;
		keys[EKeyCode::eKeyCapsLock] = vk_CapsLock;
		keys[EKeyCode::eKeyTab] = vk_Tab;
		keys[EKeyCode::eKeyLWindows] = vk_LWin;
		keys[EKeyCode::eKeyRWindows] = vk_RWin;
		keys[EKeyCode::eKeyAltL] = vk_LAlt;
		keys[EKeyCode::eKeyAltR] = vk_RAlt;

		keys[EKeyCode::eKeySpace] = vk_Space;
		keys[EKeyCode::eKeyBackSlash] = vk_BackSlash;
		keys[EKeyCode::eKeyForwardSlash] = vk_Slash;
		keys[EKeyCode::eKeyComma] = vk_Comma;
		keys[EKeyCode::eKeyFullStop] = vk_Period;
		keys[EKeyCode::eKeyBracketL] = vk_LeftBrace;
		keys[EKeyCode::eKeyBracketR] = vk_RightBrace;
		keys[EKeyCode::eKeySemiColon] = vk_SemiColon;
		keys[EKeyCode::eKeyApostrophe] = vk_Apostrophe;
		keys[EKeyCode::eKeyHash] = vk_Pound;
		keys[EKeyCode::eKeyApps] = vk_Apps;

		keys[EKeyCode::eKeyBackspace] = vk_BackSpace;
		keys[EKeyCode::eKeyEnter] = vk_Return;
		keys[EKeyCode::eKeyCtrlL] = vk_LControl;
		keys[EKeyCode::eKeyCtrlR] = vk_RControl;
		keys[EKeyCode::eKeyShiftL] = vk_LShift;
		keys[EKeyCode::eKeyShiftR] = vk_RShift;

		keys[EKeyCode::eKeyPrintScreen] = vk_PrintScreen;
		keys[EKeyCode::eKeyScrollLock] = vk_ScrollLock;
		keys[EKeyCode::eKeyPause] = vk_Pause;

		keys[EKeyCode::eKeyInsert] = vk_Insert;
		keys[EKeyCode::eKeyHome] = vk_Home;
		keys[EKeyCode::eKeyPageUp] = VK_PRIOR;
		keys[EKeyCode::eKeyDelete] = vk_Delete;
		keys[EKeyCode::eKeyEnd] = vk_End;
		keys[EKeyCode::eKeyPageDown] = VK_NEXT;

		//Arrow keys
		keys[EKeyCode::eKeyArrowUp] = vk_Up;
		keys[EKeyCode::eKeyArrowDown] = vk_Down;
		keys[EKeyCode::eKeyArrowLeft] = vk_Left;
		keys[EKeyCode::eKeyArrowRight] = vk_Right;

		//Numpad keys
		keys[EKeyCode::eKeyNumpadLock] = vk_NumLock;
		keys[EKeyCode::eKeyNumpadDivide] = vk_Divide;
		keys[EKeyCode::eKeyNumpadMultiply] = vk_Multiply;
		keys[EKeyCode::eKeyNumpadMinus] = vk_Subtract;
		keys[EKeyCode::eKeyNumpadAdd] = vk_Add;
		keys[EKeyCode::eKeyNumpadEnter] = unused;
		keys[EKeyCode::eKeyNumpadDecimal] = vk_Decimal;
		keys[EKeyCode::eKeyNumpad0] = vk_NumPad0;
		keys[EKeyCode::eKeyNumpad1] = vk_NumPad1;
		keys[EKeyCode::eKeyNumpad2] = vk_NumPad2;
		keys[EKeyCode::eKeyNumpad3] = vk_NumPad3;
		keys[EKeyCode::eKeyNumpad4] = vk_NumPad4;
		keys[EKeyCode::eKeyNumpad5] = vk_NumPad5;
		keys[EKeyCode::eKeyNumpad6] = vk_NumPad6;
		keys[EKeyCode::eKeyNumpad7] = vk_NumPad7;
		keys[EKeyCode::eKeyNumpad8] = vk_NumPad8;
		keys[EKeyCode::eKeyNumpad9] = vk_NumPad9;

		keys[EKeyCode::eMouseButtonLeft] = unused;
		keys[EKeyCode::eMouseButtonRight] = unused;
		keys[EKeyCode::eMouseButtonMiddle] = unused;
		keys[EKeyCode::eMouseXbutton1] = unused;
		keys[EKeyCode::eMouseXbutton2] = unused;

		/*
			Key names
		*/

		keyStrings[EKeyCode::eKeyEsc] = "ESCAPE";

		keyStrings[EKeyCode::eKeyA] = "A";
		keyStrings[EKeyCode::eKeyB] = "B";
		keyStrings[EKeyCode::eKeyC] = "C";
		keyStrings[EKeyCode::eKeyD] = "D";
		keyStrings[EKeyCode::eKeyE] = "E";
		keyStrings[EKeyCode::eKeyF] = "F";
		keyStrings[EKeyCode::eKeyG] = "G";
		keyStrings[EKeyCode::eKeyH] = "H";
		keyStrings[EKeyCode::eKeyI] = "I";
		keyStrings[EKeyCode::eKeyJ] = "J";
		keyStrings[EKeyCode::eKeyK] = "K";
		keyStrings[EKeyCode::eKeyL] = "L";
		keyStrings[EKeyCode::eKeyM] = "M";
		keyStrings[EKeyCode::eKeyN] = "N";
		keyStrings[EKeyCode::eKeyO] = "O";
		keyStrings[EKeyCode::eKeyP] = "P";
		keyStrings[EKeyCode::eKeyQ] = "Q";
		keyStrings[EKeyCode::eKeyR] = "R";
		keyStrings[EKeyCode::eKeyS] = "S";
		keyStrings[EKeyCode::eKeyT] = "T";
		keyStrings[EKeyCode::eKeyU] = "U";
		keyStrings[EKeyCode::eKeyV] = "V";
		keyStrings[EKeyCode::eKeyW] = "W";
		keyStrings[EKeyCode::eKeyX] = "X";
		keyStrings[EKeyCode::eKeyY] = "Y";
		keyStrings[EKeyCode::eKeyZ] = "Z";

		keyStrings[EKeyCode::eKeyF1] = "F1";
		keyStrings[EKeyCode::eKeyF2] = "F2";
		keyStrings[EKeyCode::eKeyF3] = "F3";
		keyStrings[EKeyCode::eKeyF4] = "F4";
		keyStrings[EKeyCode::eKeyF5] = "F5";
		keyStrings[EKeyCode::eKeyF6] = "F6";
		keyStrings[EKeyCode::eKeyF7] = "F7";
		keyStrings[EKeyCode::eKeyF8] = "F8";
		keyStrings[EKeyCode::eKeyF9] = "F9";
		keyStrings[EKeyCode::eKeyF10] = "F10";
		keyStrings[EKeyCode::eKeyF11] = "F11";
		keyStrings[EKeyCode::eKeyF12] = "F12";

		keyStrings[EKeyCode::eKey0] = "0";
		keyStrings[EKeyCode::eKey1] = "1";
		keyStrings[EKeyCode::eKey2] = "2";
		keyStrings[EKeyCode::eKey3] = "3";
		keyStrings[EKeyCode::eKey4] = "4";
		keyStrings[EKeyCode::eKey5] = "5";
		keyStrings[EKeyCode::eKey6] = "6";
		keyStrings[EKeyCode::eKey7] = "7";
		keyStrings[EKeyCode::eKey8] = "8";
		keyStrings[EKeyCode::eKey9] = "9";

		keyStrings[EKeyCode::eKeyMinus] = "-";
		keyStrings[EKeyCode::eKeyEquals] = "=";

		keyStrings[EKeyCode::eKeyGrave] = "`";
		keyStrings[EKeyCode::eKeyCapsLock] = "CAPS LOCK";
		keyStrings[EKeyCode::eKeyTab] = "TAB";
		keyStrings[EKeyCode::eKeyLWindows] = "Windows";
		keyStrings[EKeyCode::eKeyRWindows] = "Right Windows";
		keyStrings[EKeyCode::eKeyAltL] = "Alt";
		keyStrings[EKeyCode::eKeyAltR] = "Right Alt";

		keyStrings[EKeyCode::eKeySpace] = "SPACE";
		keyStrings[EKeyCode::eKeyBackSlash] = "\\";
		keyStrings[EKeyCode::eKeyForwardSlash] = "/";
		keyStrings[EKeyCode::eKeyComma] = ",";
		keyStrings[EKeyCode::eKeyFullStop] = ".";
		keyStrings[EKeyCode::eKeyBracketL] = "[";
		keyStrings[EKeyCode::eKeyBracketR] = "]";
		keyStrings[EKeyCode::eKeySemiColon] = ";";
		keyStrings[EKeyCode::eKeyApostrophe] = "'";
		keyStrings[EKeyCode::eKeyHash] = "#";
		keyStrings[EKeyCode::eKeyApps] = "APPLICATION";

		keyStrings[EKeyCode::eKeyBackspace] = "BACKSPACE";
		keyStrings[EKeyCode::eKeyEnter] = "ENTER";
		keyStrings[EKeyCode::eKeyCtrlL] = "Control";
		keyStrings[EKeyCode::eKeyCtrlR] = "Right Control";
		keyStrings[EKeyCode::eKeyShiftL] = "Shift";
		keyStrings[EKeyCode::eKeyShiftR] = "Right Shift";

		keyStrings[EKeyCode::eKeyPrintScreen] = "Print Screen";
		keyStrings[EKeyCode::eKeyScrollLock] = "SCROLL LOCK";
		keyStrings[EKeyCode::eKeyPause] = "Break";

		keyStrings[EKeyCode::eKeyInsert] = "INSERT";
		keyStrings[EKeyCode::eKeyHome] = "HOME";
		keyStrings[EKeyCode::eKeyPageUp] = "Page Up";
		keyStrings[EKeyCode::eKeyDelete] = "DELETE";
		keyStrings[EKeyCode::eKeyEnd] = "END";
		keyStrings[EKeyCode::eKeyPageDown] = "Page Down";

		//Arrow keys
		keyStrings[EKeyCode::eKeyArrowUp] = "UP";
		keyStrings[EKeyCode::eKeyArrowDown] = "DOWN";
		keyStrings[EKeyCode::eKeyArrowLeft] = "LEFT";
		keyStrings[EKeyCode::eKeyArrowRight] = "RIGHT";

		//Numpad keys
		keyStrings[EKeyCode::eKeyNumpadLock] = "NUM LOCK";
		keyStrings[EKeyCode::eKeyNumpadDivide] = "NUM Divide";
		keyStrings[EKeyCode::eKeyNumpadMultiply] = "NUM Multiply";
		keyStrings[EKeyCode::eKeyNumpadMinus] = "NUM Minus";
		keyStrings[EKeyCode::eKeyNumpadAdd] = "NUM Add";
		keyStrings[EKeyCode::eKeyNumpadEnter] = "NUM Enter";
		keyStrings[EKeyCode::eKeyNumpadDecimal] = "NUM Decimal";
		keyStrings[EKeyCode::eKeyNumpad0] = "NUM 0";
		keyStrings[EKeyCode::eKeyNumpad1] = "NUM 1";
		keyStrings[EKeyCode::eKeyNumpad2] = "NUM 2";
		keyStrings[EKeyCode::eKeyNumpad3] = "NUM 3";
		keyStrings[EKeyCode::eKeyNumpad4] = "NUM 4";
		keyStrings[EKeyCode::eKeyNumpad5] = "NUM 5";
		keyStrings[EKeyCode::eKeyNumpad6] = "NUM 6";
		keyStrings[EKeyCode::eKeyNumpad7] = "NUM 7";
		keyStrings[EKeyCode::eKeyNumpad8] = "NUM 8";
		keyStrings[EKeyCode::eKeyNumpad9] = "NUM 9";

		keyStrings[EKeyCode::eMouseButtonLeft] = "Mouse Left";
		keyStrings[EKeyCode::eMouseButtonRight] = "Mouse Right";
		keyStrings[EKeyCode::eMouseButtonMiddle] = "Mouse Middle";
		keyStrings[EKeyCode::eMouseXbutton1] = "Mouse X1";
		keyStrings[EKeyCode::eMouseXbutton2] = "Mouse X2";
	}

	typedef StaticString<32> Name;

	//Lookup tables
	array<uint32, EKeyCode::KeyEnumMax> keys;
	array<Name, EKeyCode::KeyEnumMax> keyStrings;

} s_keyTable;

///////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	namespace keys
	{
		const char* getKeyName(EKeyCode code)
		{
			if (code < EKeyCode::KeyEnumMax)
			{
				return s_keyTable.keyStrings[code].str();
			}

			return nullptr;
		}

		EKeyCode mapFromVirtualKey(uint32 code)
		{
			for (int i = 0; i < EKeyCode::KeyEnumMax; i++)
			{
				if (s_keyTable.keys[i] == code)
				{
					return (EKeyCode)i;
				}
			}

			return EKeyCode::eKeyUnknown;
		}

		uint32 mapToVirtualKey(EKeyCode code)
		{
			if (code < EKeyCode::KeyEnumMax)
			{
				return s_keyTable.keys[code];
			}

			return 0;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
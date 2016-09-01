/*
	Keycodes

	todo: fix NUMPAD minus and NUMPAD enter keys
*/

#include <tscore/strings.h>
#include <tscore/types.h>
#include "keycodes.h"
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

CKeyTable::CKeyTable()
{
	/*
		Key codes
	*/

	const uint32 unused = 0xffffffff;

	m_keys[EKeyCode::eKeyEsc] = vk_Escape;

	m_keys[EKeyCode::eKeyA] = vk_A;
	m_keys[EKeyCode::eKeyB] = vk_B;
	m_keys[EKeyCode::eKeyC] = vk_C;
	m_keys[EKeyCode::eKeyD] = vk_D;
	m_keys[EKeyCode::eKeyE] = vk_E;
	m_keys[EKeyCode::eKeyF] = vk_F;
	m_keys[EKeyCode::eKeyG] = vk_G;
	m_keys[EKeyCode::eKeyH] = vk_H;
	m_keys[EKeyCode::eKeyI] = vk_I;
	m_keys[EKeyCode::eKeyJ] = vk_J;
	m_keys[EKeyCode::eKeyK] = vk_K;
	m_keys[EKeyCode::eKeyL] = vk_L;
	m_keys[EKeyCode::eKeyM] = vk_M;
	m_keys[EKeyCode::eKeyN] = vk_N;
	m_keys[EKeyCode::eKeyO] = vk_O;
	m_keys[EKeyCode::eKeyP] = vk_P;
	m_keys[EKeyCode::eKeyQ] = vk_Q;
	m_keys[EKeyCode::eKeyR] = vk_R;
	m_keys[EKeyCode::eKeyS] = vk_S;
	m_keys[EKeyCode::eKeyT] = vk_T;
	m_keys[EKeyCode::eKeyU] = vk_U;
	m_keys[EKeyCode::eKeyV] = vk_V;
	m_keys[EKeyCode::eKeyW] = vk_W;
	m_keys[EKeyCode::eKeyZ] = vk_Z;
	m_keys[EKeyCode::eKeyY] = vk_Y;
	m_keys[EKeyCode::eKeyX] = vk_X;

	m_keys[EKeyCode::eKeyF1] = vk_F1;
	m_keys[EKeyCode::eKeyF2] = vk_F2;
	m_keys[EKeyCode::eKeyF3] = vk_F3;
	m_keys[EKeyCode::eKeyF4] = vk_F4;
	m_keys[EKeyCode::eKeyF5] = vk_F5;
	m_keys[EKeyCode::eKeyF6] = vk_F6;
	m_keys[EKeyCode::eKeyF7] = vk_F7;
	m_keys[EKeyCode::eKeyF8] = vk_F8;
	m_keys[EKeyCode::eKeyF9] = vk_F9;
	m_keys[EKeyCode::eKeyF10] = vk_F10;
	m_keys[EKeyCode::eKeyF11] = vk_F11;
	m_keys[EKeyCode::eKeyF12] = vk_F12;
	m_keys[EKeyCode::eKeyF13] = vk_F13;
	m_keys[EKeyCode::eKeyF14] = vk_F14;
	m_keys[EKeyCode::eKeyF15] = vk_F15;
	m_keys[EKeyCode::eKeyF16] = vk_F16;

	m_keys[EKeyCode::eKey0] = vk_0;
	m_keys[EKeyCode::eKey1] = vk_1;
	m_keys[EKeyCode::eKey2] = vk_2;
	m_keys[EKeyCode::eKey3] = vk_3;
	m_keys[EKeyCode::eKey4] = vk_4;
	m_keys[EKeyCode::eKey5] = vk_5;
	m_keys[EKeyCode::eKey6] = vk_6;
	m_keys[EKeyCode::eKey7] = vk_7;
	m_keys[EKeyCode::eKey8] = vk_8;
	m_keys[EKeyCode::eKey9] = vk_9;

	m_keys[EKeyCode::eKeyMinus] = vk_UnderScore;
	m_keys[EKeyCode::eKeyEquals] = vk_Equals;

	m_keys[EKeyCode::eKeyGrave] = vk_Grave;
	m_keys[EKeyCode::eKeyCapsLock] = vk_CapsLock;
	m_keys[EKeyCode::eKeyTab] = vk_Tab;
	m_keys[EKeyCode::eKeyLWindows] = vk_LWin;
	m_keys[EKeyCode::eKeyRWindows] = vk_RWin;
	m_keys[EKeyCode::eKeyAltL] = vk_LAlt;
	m_keys[EKeyCode::eKeyAltR] = vk_RAlt;

	m_keys[EKeyCode::eKeySpace] = vk_Space;
	m_keys[EKeyCode::eKeyBackSlash] = vk_BackSlash;
	m_keys[EKeyCode::eKeyForwardSlash] = vk_Slash;
	m_keys[EKeyCode::eKeyComma] = vk_Comma;
	m_keys[EKeyCode::eKeyFullStop] = vk_Period;
	m_keys[EKeyCode::eKeyBracketL] = vk_LeftBrace;
	m_keys[EKeyCode::eKeyBracketR] = vk_RightBrace;
	m_keys[EKeyCode::eKeySemiColon] = vk_SemiColon;
	m_keys[EKeyCode::eKeyApostrophe] = vk_Apostrophe;
	m_keys[EKeyCode::eKeyHash] = vk_Pound;
	m_keys[EKeyCode::eKeyApps] = vk_Apps;

	m_keys[EKeyCode::eKeyBackspace] = vk_BackSpace;
	m_keys[EKeyCode::eKeyEnter] = vk_Return;
	m_keys[EKeyCode::eKeyCtrlL] = vk_LControl;
	m_keys[EKeyCode::eKeyCtrlR] = vk_RControl;
	m_keys[EKeyCode::eKeyShiftL] = vk_LShift;
	m_keys[EKeyCode::eKeyShiftR] = vk_RShift;

	m_keys[EKeyCode::eKeyPrintScreen] = vk_PrintScreen;
	m_keys[EKeyCode::eKeyScrollLock] = vk_ScrollLock;
	m_keys[EKeyCode::eKeyPause] = vk_Pause;

	m_keys[EKeyCode::eKeyInsert] = vk_Insert;
	m_keys[EKeyCode::eKeyHome] = vk_Home;
	m_keys[EKeyCode::eKeyPageUp] = VK_PRIOR;
	m_keys[EKeyCode::eKeyDelete] = vk_Delete;
	m_keys[EKeyCode::eKeyEnd] = vk_End;
	m_keys[EKeyCode::eKeyPageDown] = VK_NEXT;

	//Arrow keys
	m_keys[EKeyCode::eKeyArrowUp] = vk_Up;
	m_keys[EKeyCode::eKeyArrorDown] = vk_Down;
	m_keys[EKeyCode::eKeyArrowLeft] = vk_Left;
	m_keys[EKeyCode::eKeyArrowRight] = vk_Right;

	//Numpad keys
	m_keys[EKeyCode::eKeyNumpadLock] = vk_NumLock;
	m_keys[EKeyCode::eKeyNumpadDivide] = vk_Divide;
	m_keys[EKeyCode::eKeyNumpadMultiply] = vk_Multiply;
	m_keys[EKeyCode::eKeyNumpadMinus] = vk_Subtract;
	m_keys[EKeyCode::eKeyNumpadAdd] = vk_Add;
	m_keys[EKeyCode::eKeyNumpadEnter] = unused;
	m_keys[EKeyCode::eKeyNumpadDecimal] = vk_Decimal;
	m_keys[EKeyCode::eKeyNumpad0] = vk_NumPad0;
	m_keys[EKeyCode::eKeyNumpad1] = vk_NumPad1;
	m_keys[EKeyCode::eKeyNumpad2] = vk_NumPad2;
	m_keys[EKeyCode::eKeyNumpad3] = vk_NumPad3;
	m_keys[EKeyCode::eKeyNumpad4] = vk_NumPad4;
	m_keys[EKeyCode::eKeyNumpad5] = vk_NumPad5;
	m_keys[EKeyCode::eKeyNumpad6] = vk_NumPad6;
	m_keys[EKeyCode::eKeyNumpad7] = vk_NumPad7;
	m_keys[EKeyCode::eKeyNumpad8] = vk_NumPad8;
	m_keys[EKeyCode::eKeyNumpad9] = vk_NumPad9;

	/*
		Key names
	*/

	m_keyStrings[EKeyCode::eKeyEsc] = "ESCAPE";

	m_keyStrings[EKeyCode::eKeyA] = "A";
	m_keyStrings[EKeyCode::eKeyB] = "B";
	m_keyStrings[EKeyCode::eKeyC] = "C";
	m_keyStrings[EKeyCode::eKeyD] = "D";
	m_keyStrings[EKeyCode::eKeyE] = "E";
	m_keyStrings[EKeyCode::eKeyF] = "F";
	m_keyStrings[EKeyCode::eKeyG] = "G";
	m_keyStrings[EKeyCode::eKeyH] = "H";
	m_keyStrings[EKeyCode::eKeyI] = "I";
	m_keyStrings[EKeyCode::eKeyJ] = "J";
	m_keyStrings[EKeyCode::eKeyK] = "K";
	m_keyStrings[EKeyCode::eKeyL] = "L";
	m_keyStrings[EKeyCode::eKeyM] = "M";
	m_keyStrings[EKeyCode::eKeyN] = "N";
	m_keyStrings[EKeyCode::eKeyO] = "O";
	m_keyStrings[EKeyCode::eKeyP] = "P";
	m_keyStrings[EKeyCode::eKeyQ] = "Q";
	m_keyStrings[EKeyCode::eKeyR] = "R";
	m_keyStrings[EKeyCode::eKeyS] = "S";
	m_keyStrings[EKeyCode::eKeyT] = "T";
	m_keyStrings[EKeyCode::eKeyU] = "U";
	m_keyStrings[EKeyCode::eKeyV] = "V";
	m_keyStrings[EKeyCode::eKeyW] = "W";
	m_keyStrings[EKeyCode::eKeyX] = "X";
	m_keyStrings[EKeyCode::eKeyY] = "Y";
	m_keyStrings[EKeyCode::eKeyZ] = "Z";

	m_keyStrings[EKeyCode::eKeyF1] = "F1";
	m_keyStrings[EKeyCode::eKeyF2] = "F2";
	m_keyStrings[EKeyCode::eKeyF3] = "F3";
	m_keyStrings[EKeyCode::eKeyF4] = "F4";
	m_keyStrings[EKeyCode::eKeyF5] = "F5";
	m_keyStrings[EKeyCode::eKeyF6] = "F6";
	m_keyStrings[EKeyCode::eKeyF7] = "F7";
	m_keyStrings[EKeyCode::eKeyF8] = "F8";
	m_keyStrings[EKeyCode::eKeyF9] = "F9";
	m_keyStrings[EKeyCode::eKeyF10] = "F10";
	m_keyStrings[EKeyCode::eKeyF11] = "F11";
	m_keyStrings[EKeyCode::eKeyF12] = "F12";

	m_keyStrings[EKeyCode::eKey0] = "0";
	m_keyStrings[EKeyCode::eKey1] = "1";
	m_keyStrings[EKeyCode::eKey2] = "2";
	m_keyStrings[EKeyCode::eKey3] = "3";
	m_keyStrings[EKeyCode::eKey4] = "4";
	m_keyStrings[EKeyCode::eKey5] = "5";
	m_keyStrings[EKeyCode::eKey6] = "6";
	m_keyStrings[EKeyCode::eKey7] = "7";
	m_keyStrings[EKeyCode::eKey8] = "8";
	m_keyStrings[EKeyCode::eKey9] = "9";

	m_keyStrings[EKeyCode::eKeyMinus] = "-";
	m_keyStrings[EKeyCode::eKeyEquals] = "=";

	m_keyStrings[EKeyCode::eKeyGrave] = "`";
	m_keyStrings[EKeyCode::eKeyCapsLock] = "CAPS LOCK";
	m_keyStrings[EKeyCode::eKeyTab] = "TAB";
	m_keyStrings[EKeyCode::eKeyLWindows] = "Windows";
	m_keyStrings[EKeyCode::eKeyRWindows] = "Right Windows";
	m_keyStrings[EKeyCode::eKeyAltL] = "Alt";
	m_keyStrings[EKeyCode::eKeyAltR] = "Right Alt";

	m_keyStrings[EKeyCode::eKeySpace] = "SPACE";
	m_keyStrings[EKeyCode::eKeyBackSlash] = "\\";
	m_keyStrings[EKeyCode::eKeyForwardSlash] = "/";
	m_keyStrings[EKeyCode::eKeyComma] = ",";
	m_keyStrings[EKeyCode::eKeyFullStop] = ".";
	m_keyStrings[EKeyCode::eKeyBracketL] = "[";
	m_keyStrings[EKeyCode::eKeyBracketR] = "]";
	m_keyStrings[EKeyCode::eKeySemiColon] = ";";
	m_keyStrings[EKeyCode::eKeyApostrophe] = "'";
	m_keyStrings[EKeyCode::eKeyHash] = "#";
	m_keyStrings[EKeyCode::eKeyApps] = "APPLICATION";

	m_keyStrings[EKeyCode::eKeyBackspace] = "BACKSPACE";
	m_keyStrings[EKeyCode::eKeyEnter] = "ENTER";
	m_keyStrings[EKeyCode::eKeyCtrlL] = "Control";
	m_keyStrings[EKeyCode::eKeyCtrlR] = "Right Control";
	m_keyStrings[EKeyCode::eKeyShiftL] = "Shift";
	m_keyStrings[EKeyCode::eKeyShiftR] = "Right Shift";

	m_keyStrings[EKeyCode::eKeyPrintScreen] = "Print Screen";
	m_keyStrings[EKeyCode::eKeyScrollLock] = "SCROLL LOCK";
	m_keyStrings[EKeyCode::eKeyPause] = "Break";

	m_keyStrings[EKeyCode::eKeyInsert] = "INSERT";
	m_keyStrings[EKeyCode::eKeyHome] = "HOME";
	m_keyStrings[EKeyCode::eKeyPageUp] = "Page Up";
	m_keyStrings[EKeyCode::eKeyDelete] = "DELETE";
	m_keyStrings[EKeyCode::eKeyEnd] = "END";
	m_keyStrings[EKeyCode::eKeyPageDown] = "Page Down";

	//Arrow keys
	m_keyStrings[EKeyCode::eKeyArrowUp] = "UP";
	m_keyStrings[EKeyCode::eKeyArrorDown] = "DOWN";
	m_keyStrings[EKeyCode::eKeyArrowLeft] = "LEFT";
	m_keyStrings[EKeyCode::eKeyArrowRight] = "RIGHT";

	//Numpad keys
	m_keyStrings[EKeyCode::eKeyNumpadLock] = "NUM LOCK";
	m_keyStrings[EKeyCode::eKeyNumpadDivide] = "NUM Divide";
	m_keyStrings[EKeyCode::eKeyNumpadMultiply] = "NUM Multiply";
	m_keyStrings[EKeyCode::eKeyNumpadMinus] = "NUM Minus";
	m_keyStrings[EKeyCode::eKeyNumpadAdd] = "NUM Add";
	m_keyStrings[EKeyCode::eKeyNumpadEnter] = "NUM Enter";
	m_keyStrings[EKeyCode::eKeyNumpadDecimal] = "NUM Decimal";
	m_keyStrings[EKeyCode::eKeyNumpad0] = "NUM 0";
	m_keyStrings[EKeyCode::eKeyNumpad1] = "NUM 1";
	m_keyStrings[EKeyCode::eKeyNumpad2] = "NUM 2";
	m_keyStrings[EKeyCode::eKeyNumpad3] = "NUM 3";
	m_keyStrings[EKeyCode::eKeyNumpad4] = "NUM 4";
	m_keyStrings[EKeyCode::eKeyNumpad5] = "NUM 5";
	m_keyStrings[EKeyCode::eKeyNumpad6] = "NUM 6";
	m_keyStrings[EKeyCode::eKeyNumpad7] = "NUM 7";
	m_keyStrings[EKeyCode::eKeyNumpad8] = "NUM 8";
	m_keyStrings[EKeyCode::eKeyNumpad9] = "NUM 9";
}

///////////////////////////////////////////////////////////////////////////////////////////

bool CKeyTable::getKeyName(EKeyCode code, KeyName& name)
{
	if (code > EKeyCode::KeyEnumMax)
		return false;

	name = m_keyStrings[code];

	return true;
}

EKeyCode CKeyTable::mapFromVirtualKey(uint32 code)
{
	for (int i = 0; i < EKeyCode::KeyEnumMax; i++)
	{
		if (m_keys[i] == code)
		{
			return (EKeyCode)i;
		}
	}

	return EKeyCode::eKeyUnknown;
}

uint32 CKeyTable::mapToVirtualKey(EKeyCode code)
{
	if (code > EKeyCode::KeyEnumMax)
		return 0;

	return m_keys[code];
}

///////////////////////////////////////////////////////////////////////////////////////////
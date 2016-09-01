/*
	Keycodes
*/

#pragma once

#include <tscore/strings.h>
#include <array>

namespace ts
{
	//Physical key codes
	enum EKeyCode
	{
		eKeyUnknown,

		eKeyEsc,

		eKeyA,
		eKeyB,
		eKeyC,
		eKeyD,
		eKeyE,
		eKeyF,
		eKeyG,
		eKeyH,
		eKeyI,
		eKeyJ,
		eKeyK,
		eKeyL,
		eKeyM,
		eKeyN,
		eKeyO,
		eKeyP,
		eKeyQ,
		eKeyR,
		eKeyS,
		eKeyT,
		eKeyU,
		eKeyV,
		eKeyW,
		eKeyX,
		eKeyY,
		eKeyZ,

		eKeyF1,
		eKeyF2,
		eKeyF3,
		eKeyF4,
		eKeyF5,
		eKeyF6,
		eKeyF7,
		eKeyF8,
		eKeyF9,
		eKeyF10,
		eKeyF11,
		eKeyF12,
		eKeyF13,
		eKeyF14,
		eKeyF15,
		eKeyF16,

		eKey1,
		eKey2,
		eKey3,
		eKey4,
		eKey5,
		eKey6,
		eKey7,
		eKey8,
		eKey9,
		eKey0,

		eKeyMinus,
		eKeyEquals,

		eKeyGrave,
		eKeyCapsLock,
		eKeyTab,
		eKeyLWindows,
		eKeyRWindows,
		eKeyAltL,
		eKeyAltR,

		eKeySpace,
		eKeyBackSlash,
		eKeyForwardSlash,
		eKeyComma,
		eKeyFullStop,
		eKeyBracketL,
		eKeyBracketR,
		eKeySemiColon,
		eKeyApostrophe,
		eKeyHash,
		eKeyApps,

		eKeyBackspace,
		eKeyEnter,
		eKeyCtrlL,
		eKeyCtrlR,
		eKeyShiftL,
		eKeyShiftR,

		eKeyPrintScreen,
		eKeyScrollLock,
		eKeyPause,

		eKeyInsert,
		eKeyHome,
		eKeyPageUp,
		eKeyDelete,
		eKeyEnd,
		eKeyPageDown,

		eKeyArrowUp,
		eKeyArrorDown,
		eKeyArrowLeft,
		eKeyArrowRight,

		eKeyNumpadLock,
		eKeyNumpadDivide,
		eKeyNumpadMultiply,
		eKeyNumpadMinus,
		eKeyNumpadAdd,
		eKeyNumpadEnter,
		eKeyNumpadDecimal,
		eKeyNumpad0,
		eKeyNumpad1,
		eKeyNumpad2,
		eKeyNumpad3,
		eKeyNumpad4,
		eKeyNumpad5,
		eKeyNumpad6,
		eKeyNumpad7,
		eKeyNumpad8,
		eKeyNumpad9,

		KeyEnumMax
	};
	
	//Key table class for converting between key types
	class CKeyTable
	{
	public:

		typedef StaticString<32> KeyName;

		CKeyTable();
		~CKeyTable() {}

		bool getKeyName(EKeyCode code, KeyName& name);
		EKeyCode mapFromVirtualKey(uint32 vk);
		uint32 mapToVirtualKey(EKeyCode code);

	private:

		std::array<uint32, EKeyCode::KeyEnumMax> m_keys;
		std::array<KeyName, EKeyCode::KeyEnumMax> m_keyStrings;

	};
}

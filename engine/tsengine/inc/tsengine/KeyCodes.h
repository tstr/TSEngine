/*
	Keycodes
*/

#pragma once

#include <tsengine/abi.h>
#include <tscore/strings.h>
#include <array>

namespace ts
{
	//Physical key/button codes
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
		eKeyArrowDown,
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

		eMouseButtonLeft,
		eMouseButtonRight,
		eMouseButtonMiddle,
		eMouseXbutton1,
		eMouseXbutton2,

		KeyEnumMax
	};
	
	namespace keys
	{
		TSENGINE_API const char* getKeyName(EKeyCode code);

		TSENGINE_API EKeyCode mapFromVirtualKey(uint32 vk);

		TSENGINE_API uint32 mapToVirtualKey(EKeyCode code);
	}
}

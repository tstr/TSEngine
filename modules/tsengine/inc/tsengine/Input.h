/*
	Input module header
*/

#pragma once

#include <tsengine/abi.h>
#include <tsengine/KeyCodes.h>
#include <tscore/types.h>
#include <list>

namespace ts
{
	struct PlatformEventArgs;
	class Window;

	enum EInputDevice
	{
		eInputDeviceMouse,
		eInputDeviceKeyboard,
		eInputDeviceOther
	};

	struct InputEvent
	{
		EInputDevice device;

		EKeyCode keyCode;
		bool isUp;
		bool isFirstTime;

		int16 deltaX;
		int16 deltaY;
		int16 deltaScroll;

		InputEvent() :
			device(EInputDevice::eInputDeviceOther),
			keyCode(eKeyUnknown),
			isUp(true), isFirstTime(true),
			deltaX(0), deltaY(0), deltaScroll(0)
		{}
	};

	/*
		Input System class:

		Handles management of input devices and events
	*/
	class InputSystem
	{
	public:

		enum EFlags : uint
		{
			eFlagDefault	   = 0,
			eFlagEnableLogging = 1
		};

		/*
			Event Listener interface
		*/
		class IListener
		{
		public:

			virtual void onKeyDown(EKeyCode code) {}
			virtual void onKeyUp(EKeyCode code) {}

			virtual void onChar(wchar character) {}

			virtual void onMouseMove(int deltaX, int deltaY) {}
			virtual void onMouseScroll(int delta) {}
		};

		//Constructor/Destructor
		TSENGINE_API InputSystem(Window* platformWindow, uint flags = EFlags::eFlagDefault);
		TSENGINE_API ~InputSystem();

		//Cursor methods
		void TSENGINE_API getCursorPos(uint& x, uint& y) const;
		void TSENGINE_API setCursorPos(uint x, uint y);
		void TSENGINE_API showCursor(bool show);
		bool TSENGINE_API cursorShown() const;

		//Input event listeners
		void TSENGINE_API addListener(IListener*);
		void TSENGINE_API removeListener(IListener*);

		//Window event handler
		void TSENGINE_API onEvent(const PlatformEventArgs& args);

		//Input system flags
		uint getFlags() const { return m_flags; }
		void setFlags(uint flags) { m_flags = flags; }

	private:

		//Platform window reference
		Window* m_window;

		//Event listeners
		std::list<IListener*> m_eventListeners;

		//Internal state
		uint m_flags;
		bool m_cursorShown;
		uint m_mouseX;
		uint m_mouseY;

		template<typename ... Args_t>
		void dispatch(void(IListener::*handler)(Args_t...), Args_t ... args)
		{
			for (auto l : m_eventListeners)
			{
				(l->*handler)(args...);
			}
		}

		InputEvent TSENGINE_API translate(const PlatformEventArgs& args);
	};
}

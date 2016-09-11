/*
	Input module header
*/

#pragma once

#include <tscore/types.h>
#include "inputdevice.h"

namespace ts
{
	class IInputEventListener
	{
	public:
		
		virtual int onMouse(int16 dx, int16 dy) { return 0; }
		//virtual int onMouseClick(const SInputMouseEvent&) { return 0; }
		virtual int onMouseDown(const SInputMouseEvent&) { return 0; }
		virtual int onMouseUp(const SInputMouseEvent&) { return 0; }
		//virtual int onMouseScroll(const SInputMouseEvent&) { return 0; }

		//virtual int onKeyPress(const SInputKeyEvent&) { return 0; }
		virtual int onKeyDown(EKeyCode code) { return 0; }
		virtual int onKeyUp(EKeyCode code) { return 0; }
	};

	class CInputModule
	{
	private:

		CWindow* m_window = nullptr;
		CInputDevice m_device;
		
		std::vector<IInputEventListener*> m_eventListeners;

		void inputLayerCallback(const SInputEvent& event);

		bool m_cursorShown = true;

		int m_mouseX = 0;
		int m_mouseY = 0;

	public:
		
		CInputModule(CWindow* window);
		~CInputModule();

		void showCursor(bool show);
		
		bool onWindowInputEvent(const SWindowEventArgs& args) { return m_device.onWindowInputEvent(args); }

		bool addEventListener(IInputEventListener* listener);
		bool removeEventListener(IInputEventListener* listener);
	};
}
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
		virtual int onMouseScroll(const SInputMouseEvent&) { return 0; }

		virtual int onChar(wchar c) { return 0; };

		//virtual int onKeyPress(const SInputKeyEvent&) { return 0; }
		virtual int onKeyDown(EKeyCode code) { return 0; }
		virtual int onKeyUp(EKeyCode code) { return 0; }
	};

	class CInputModule : public CWindow::IEventListener
	{
	private:

		CWindow* m_window = nullptr;
		CInputDevice m_device;
		
		std::vector<IInputEventListener*> m_eventListeners;

		TSENGINE_API void inputLayerCallback(const SInputEvent& event);

		bool m_cursorShown = true;

		int m_mouseX = 0;
		int m_mouseY = 0;

		TSENGINE_API int onWindowEvent(const SWindowEventArgs& args) override;

	public:
		
		TSENGINE_API CInputModule(CWindow* window);
		TSENGINE_API ~CInputModule();

		CWindow* getWindow() const { return m_window; }

		TSENGINE_API void showCursor(bool show);
		TSENGINE_API void getCursorPos(int16& x, int16& y);
		TSENGINE_API void setCursorPos(int16 x, int16 y);

		TSENGINE_API bool addEventListener(IInputEventListener* listener);
		TSENGINE_API bool removeEventListener(IInputEventListener* listener);
	};
}
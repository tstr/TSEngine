/*
	Debug menu header
*/

#pragma once

#include "ui.h"

#include <deque>

/////////////////////////////////////////////////////////////////////////////////////////////////

namespace ts
{
	class UIDebugMenu
	{
	private:
		
		Application* m_app = nullptr;

		std::deque<float> m_frametimes;
		std::deque<float> m_framerates;
		uint64 m_frameno = 0;
		double m_frametime = 0.0;

	public:
		
		UIDebugMenu(Application* app);
		~UIDebugMenu();

		void show(double dt);
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////
